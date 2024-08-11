// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeActor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "LandscapifyStructLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"

ALandscapeActor::ALandscapeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	LandscapeSettings.LandscapeMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("LandscapeMeshComponent"));
	LandscapeSettings.LandscapeMeshComponent->SetupAttachment(RootComponent);
	LandscapeSettings.LandscapeMeshComponent->bUseComplexAsSimpleCollision = true;
	LandscapeSettings.LandscapeMeshComponent->bUseAsyncCooking = true;
}

void ALandscapeActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALandscapeActor::GenerateLandscape()
{
	const int32 SelectedSize = LandscapeSettings.LandscapeSizeEnum.GetIntValue();
	LandscapeSize = static_cast<int32>(FMath::Pow(2.f, static_cast<double>(SelectedSize))) + 1; // Weird casting ^^
	if (LandscapeSections.Num())
	{
		for (int32 Index = 0; Index < LandscapeSections.Num(); Index++)
		{
			if (!IsDirty(LandscapeSections[Index].Vertices.Num()))
			{
				UpdateLandscapeSection(LandscapeSections[Index]);
				return;
			}
		}
	}

	// TODO: In big sized map we need to divide it into a section. For now it works as one peace of mesh
	FMath::SRandInit(LandscapeSettings.Seed);
	for (int32 Index = 0; Index < 1; Index++)
	{
		FMeshSectionData LandscapeSection;
		LandscapeSection.Vertices.AddDefaulted(LandscapeSize * LandscapeSize);

		GenerateHeightMap(LandscapeSection.Vertices, LandscapeSection.UV);
		GenerateProceduralMeshData(LandscapeSection.Triangles, LandscapeSection.Normals);

		DiamondSquareStep(LandscapeSection, LandscapeSize - 1, LandscapeSettings.HeightMultiplier);

		LandscapeSection.Index = Index;
		LandscapeSections.Emplace(LandscapeSection);
		LandscapeSettings.LandscapeMeshComponent->CreateMeshSection_LinearColor(Index, LandscapeSection.Vertices, LandscapeSection.Triangles, LandscapeSection.Normals, LandscapeSection.UV, LandscapeSection.VertexColor, TArray<FProcMeshTangent>(), true);
	}
}

void ALandscapeActor::ResetLandscape()
{
	LandscapeSections.Reset();
}

void ALandscapeActor::GenerateHeightMap(TArray<FVector>& OutVertices, TArray<FVector2D>& OutUVs)
{
	OutUVs.SetNum(LandscapeSize * LandscapeSize);
	ParallelFor(LandscapeSize, [this, &OutVertices, &OutUVs](int32 YIndex)
	{
		for (int32 X = 0; X < LandscapeSize; X++)
		{
			int32 Index = X + YIndex * LandscapeSize;
			OutVertices[Index] = FVector(X * LandscapeSettings.VertexSize, YIndex * LandscapeSettings.VertexSize, OutVertices[Index].Z);

			float U = static_cast<float>(X) / static_cast<float>(LandscapeSize - 1);
			float V = static_cast<float>(YIndex) / static_cast<float>(LandscapeSize - 1);
			OutUVs[Index] = FVector2D(U, V);
		}
	});
	RandomizeInitPoint(OutVertices);
}

void ALandscapeActor::RandomizeInitPoint(TArray<FVector>& OutVertices)
{
	if (LandscapeSettings.bOverrideHeight)
	{
		OutVertices[0].Z = LandscapeSettings.TopLeftHeight;
		OutVertices[LandscapeSize - 1].Z = LandscapeSettings.TopRightHeight;
		OutVertices[(LandscapeSize - 1) * LandscapeSize].Z = LandscapeSettings.BottomLeftHeight;
		OutVertices[LandscapeSize * LandscapeSize - 1].Z = LandscapeSettings.BottomRightHeight;
	}
	else
	{
		FRandomStream RandomStream(LandscapeSettings.Seed);
		OutVertices[0].Z = RandomStream.FRandRange(-LandscapeSettings.MaxRandomInitHeight, LandscapeSettings.MaxRandomInitHeight);
		OutVertices[LandscapeSize - 1].Z = RandomStream.FRandRange(-LandscapeSettings.MaxRandomInitHeight, LandscapeSettings.MaxRandomInitHeight);
		OutVertices[(LandscapeSize - 1) * LandscapeSize].Z = RandomStream.FRandRange(-LandscapeSettings.MaxRandomInitHeight, LandscapeSettings.MaxRandomInitHeight);
		OutVertices[LandscapeSize * LandscapeSize - 1].Z = RandomStream.FRandRange(-LandscapeSettings.MaxRandomInitHeight, LandscapeSettings.MaxRandomInitHeight);
	}
}

void ALandscapeActor::GenerateProceduralMeshData(TArray<int32>& OutTriangles, TArray<FVector>& OutNormals)
{
	const int32 NumSquares = LandscapeSize - 1;
	OutTriangles.AddDefaulted(NumSquares * NumSquares * 6);

	ParallelFor(LandscapeSize - 1, [this, &OutTriangles](int32 YIndex)
	{
		for (int32 X = 0; X < LandscapeSize - 1; X++)
		{
			int32 TopLeft = (LandscapeSize * YIndex) + X;
			int32 TopRight = TopLeft + 1;
			int32 BottomLeft = TopLeft + LandscapeSize;
			int32 BottomRight = BottomLeft + 1;

			int32 TriangleIndex = 6 * (YIndex * (LandscapeSize - 1) + X);
			OutTriangles[TriangleIndex] = TopLeft;
			OutTriangles[TriangleIndex + 1] = BottomLeft;
			OutTriangles[TriangleIndex + 2] = TopRight;

			OutTriangles[TriangleIndex + 3] = TopRight;
			OutTriangles[TriangleIndex + 4] = BottomLeft;
			OutTriangles[TriangleIndex + 5] = BottomRight;
		}
	});
}

void ALandscapeActor::UpdateLandscapeSection(FMeshSectionData& InSection)
{
	RandomizeInitPoint(InSection.Vertices);
	DiamondSquareStep(InSection, LandscapeSize - 1, LandscapeSettings.HeightMultiplier);
	LandscapeSettings.LandscapeMeshComponent->UpdateMeshSection_LinearColor(InSection.Index, InSection.Vertices, InSection.Normals, InSection.UV, InSection.VertexColor, TArray<FProcMeshTangent>());
}

void ALandscapeActor::DiamondSquareStep(FMeshSectionData& SectionData, int32 CurrentStep, float Scale)
{
	if (CurrentStep <= 1)
	{
		GenerateTextureHeightMap(SectionData);
		return;
	}

	FRandomStream RandomStream(LandscapeSettings.Seed);
	const int32 HalfStep = CurrentStep / 2;

	for (int32 Y = HalfStep; Y < LandscapeSize; Y += CurrentStep)
	{
		for (int32 X = HalfStep; X < LandscapeSize; X += CurrentStep)
		{
			const int32 Index = X + Y * LandscapeSize;
			const int32 TopLeft = (X - HalfStep) + (Y - HalfStep) * LandscapeSize;
			const int32 TopRight = (X + HalfStep) + (Y - HalfStep) * LandscapeSize;
			const int32 BottomLeft = (X - HalfStep) + (Y + HalfStep) * LandscapeSize;
			const int32 BottomRight = (X + HalfStep) + (Y + HalfStep) * LandscapeSize;

			float AvgSquareHeight = (SectionData.Vertices[TopLeft].Z + SectionData.Vertices[TopRight].Z + SectionData.Vertices[BottomLeft].Z + SectionData.Vertices[BottomRight].Z) / 4.0f;
			SectionData.Vertices[Index].Z = AvgSquareHeight + (RandomStream.FRandRange(-Scale, Scale));
		}
	}

	for (int32 Y = 0; Y < LandscapeSize; Y += HalfStep)
	{
		for (int32 X = (Y + HalfStep) % CurrentStep; X < LandscapeSize; X += CurrentStep)
		{
			int32 Index = X + Y * LandscapeSize;

			int32 Left = (X - HalfStep < 0) ? X + HalfStep : X - HalfStep;
			int32 Right = (X + HalfStep >= LandscapeSize) ? X - HalfStep : X + HalfStep;
			int32 Top = (Y - HalfStep < 0) ? Y + HalfStep : Y - HalfStep;
			int32 Bottom = (Y + HalfStep >= LandscapeSize) ? Y - HalfStep : Y + HalfStep;

			Left = Left + Y * LandscapeSize;
			Right = Right + Y * LandscapeSize;
			Top = X + Top * LandscapeSize;
			Bottom = X + Bottom * LandscapeSize;

			float AvgHeight = (SectionData.Vertices[Left].Z + SectionData.Vertices[Right].Z + SectionData.Vertices[Top].Z + SectionData.Vertices[Bottom].Z) / 4.0f;
			SectionData.Vertices[Index].Z = AvgHeight + (RandomStream.FRandRange(-Scale, Scale));
		}
	}
	DiamondSquareStep(SectionData, CurrentStep / 2, Scale / 2.f);
}

bool ALandscapeActor::IsDirty(const int32 CurrentLandscapeSize) const
{
	return CurrentLandscapeSize != (LandscapeSize * LandscapeSize);
}

void ALandscapeActor::GenerateTextureHeightMap(const FMeshSectionData& SectionData)
{
	TArray<float> Heights;
	for (const FVector& Vertex : SectionData.Vertices)
	{
		Heights.Add(Vertex.Z);
	}

	if (Heights.Num())
	{
		Heights.Sort(TGreater<float>());

		const float& MaxHeight = Heights[0];
		const float& MinHeight = Heights[Heights.Num() - 1];

		const FString FullPackagePath = TEXT("/Game/Textures/T_HeightMap");
		const FString AssetName = FPackageName::ObjectPathToPackageName(FullPackagePath);
		
		UPackage* Package = CreatePackage(*FullPackagePath);
		if (!Package)
		{
			return;
		}
		
		UTextureFactory* TextureFactory = NewObject<UTextureFactory>();
		if (!TextureFactory)
		{
			return;
		}
		TextureFactory->AddToRoot();
		TextureFactory->SuppressImportOverwriteDialog();
		
		UTexture2D* HeightMapTexture = TextureFactory->CreateTexture2D(Package, *AssetName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
		TextureFactory->RemoveFromRoot();

		if (!HeightMapTexture)
		{
			return;
		}
		HeightMapTexture->SRGB = false;

		TArray<FColor> TextureData;
		TextureData.SetNum(LandscapeSize * LandscapeSize);

		for (int32 Y = 0; Y < LandscapeSize; Y++)
		{
			for (int32 X = 0; X < LandscapeSize; X++)
			{
				int32 Index = Y * LandscapeSize + X;
				if (Heights.IsValidIndex(Index))
				{
					float Height = Heights[Index];
					uint8 NormalizedHeight = static_cast<uint8>((Height - MinHeight) / (MaxHeight - MinHeight) * 255.0f);

					TextureData[Index] = FColor(NormalizedHeight, NormalizedHeight, NormalizedHeight, 255.f);
				}
			}
		}
		//FTexture2DMipMap& Mip = HeightMapTexture->GetPlatformData()->Mips[0];
		//void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		//FMemory::Memcpy(Data, TextureData.GetData(), TextureData.Num() * sizeof(FColor));
		//Mip.BulkData.Unlock();
		
		//HeightMapTexture->Source.Init(HeightMapTexture->GetSizeX(), HeightMapTexture->GetSizeY(), 1, 1, TSF_BGRA8, HeightMapTexture->Source.LockMip(0));
		HeightMapTexture->UpdateResource();
		SaveTexture(Package, HeightMapTexture);
	}
}

void ALandscapeActor::SaveTexture(UPackage* CreatedPackage, UTexture2D* CreatedTexture)
{
	CreatedTexture->UpdateResource();
	CreatedTexture->PostEditChange();
	
	FAssetRegistryModule::AssetCreated(CreatedTexture);
	if (CreatedPackage->MarkPackageDirty())
	{
		UEditorLoadingAndSavingUtils::SavePackages(TArray<UPackage*> { CreatedPackage }, false);
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(TArray<UObject*>{ CreatedTexture });
	}
}
