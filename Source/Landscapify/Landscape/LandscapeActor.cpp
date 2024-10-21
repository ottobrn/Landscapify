// Fill out your copyright notice in the Description page of Project Settings.

#include "LandscapeActor.h"
#include "LandscapifyStructLibrary.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

ALandscapeActor::ALandscapeActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALandscapeActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALandscapeActor::GenerateLandscape()
{
	const int32 SelectedSize = LandscapeSettings.LandscapeSizeEnum.GetIntValue();
	LandscapeSize = static_cast<int32>(FMath::Pow(2.f, static_cast<double>(SelectedSize))) + 1;
	
	FMath::SRandInit(LandscapeSettings.Seed);
	
	FMeshSectionData LandscapeSection;
	LandscapeSection.Vertices.AddDefaulted(LandscapeSize * LandscapeSize);

	GenerateHeightMap(LandscapeSection.Vertices);

	// Select what kind of algo will be processed
	if (LandscapeSettings.GeneratorAlgorithm == EAlgorithm::DiamondSquare)
	{
		DiamondSquareStep(LandscapeSection, LandscapeSize - 1, LandscapeSettings.HeightMultiplier);
	}
	else
	{
		PerlinNoise(LandscapeSection);
	}
}

void ALandscapeActor::GenerateHeightMap(TArray<FVector>& OutVertices)
{
	ParallelFor(LandscapeSize, [this, &OutVertices](int32 YIndex)
	{
		for (int32 X = 0; X < LandscapeSize; X++)
		{
			int32 Index = X + YIndex * LandscapeSize;
			OutVertices[Index] = FVector(X * LandscapeSettings.VertexSize, YIndex * LandscapeSettings.VertexSize, OutVertices[Index].Z);
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

void ALandscapeActor::UpdateLandscapeSection(FMeshSectionData& InSection)
{
	RandomizeInitPoint(InSection.Vertices);
	DiamondSquareStep(InSection, LandscapeSize - 1, LandscapeSettings.HeightMultiplier);
}

void ALandscapeActor::DiamondSquareStep(FMeshSectionData& SectionData, int32 CurrentStep, float Scale)
{
	if (CurrentStep <= 1)
	{
		GenerateHeightMapImage(SectionData);
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

void ALandscapeActor::PerlinNoise(FMeshSectionData& SectionData)
{
	ParallelFor(SectionData.Vertices.Num(), [this, &SectionData](int32 Index)
	{
		FVector& Vertex = SectionData.Vertices[Index];
		float NoiseValue = GetPerlinNoiseValue(Vertex.X, Vertex.Y);
		Vertex.Z = (NoiseValue + 1) / 2.0f * LandscapeSettings.MaxHeight;

		// TODO: [REMOVE] Island test
		// float DistanceFromCenter = FVector2D(Vertex.X - LandscapeSize / 2.0f, Vertex.Y - LandscapeSize / 2.0f).Size();
		// Vertex.Z = (DistanceFromCenter < LandscapeSize / 2) ? HeightValue : -100.f;
	});
	GenerateHeightMapImage(SectionData);
}

float ALandscapeActor::GetPerlinNoiseValue(float X, float Y)
{
	FRandomStream RandomStream(LandscapeSettings.Seed);
	float Amplitude = 1.0f;
	float Frequency = 1.0f;
	float NoiseHeight = 0.0f;

	float OffsetX = RandomStream.FRandRange(-10000.0f, 10000.0f);
	float OffsetY = RandomStream.FRandRange(-10000.0f, 10000.0f);
	
	for (int i = 0; i < LandscapeSettings.Octaves; i++)
	{
		float SampleX = X * Frequency * LandscapeSettings.NoiseScale;
		float SampleY = Y * Frequency * LandscapeSettings.NoiseScale;

		float PerlinValue = FMath::PerlinNoise2D(FVector2D(SampleX + OffsetX , SampleY + OffsetY));
		
		NoiseHeight += PerlinValue * Amplitude;

		Amplitude *= LandscapeSettings.Persistence;
		Frequency *= LandscapeSettings.Lacunarity;
	}
	return NoiseHeight;
}

void ALandscapeActor::GenerateHeightMapImage(const FMeshSectionData& SectionData)
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
		
		TArray<uint16> GrayscaleData;
		GrayscaleData.AddUninitialized(LandscapeSize * LandscapeSize);
		for (int32 Y = 0; Y < LandscapeSize; Y++)
		{
			for (int32 X = 0; X < LandscapeSize; X++)
			{
				int32 Index = Y * LandscapeSize + X;
				
				if (SectionData.Vertices.IsValidIndex(Index))
				{
					float Height = SectionData.Vertices[Index].Z;
					GrayscaleData[Index] = static_cast<uint16>((Height - MinHeight) / (MaxHeight - MinHeight) * 65535.0f);
				}
			}
		}
		CreateAndSaveHeightMapTexture(GrayscaleData);
	}
}

void ALandscapeActor::CreateAndSaveHeightMapTexture(const TArray<uint16>& TextureData)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!ImageWrapper.IsValid())
	{
		return;
	}
	// Texture will be created in ProjectDir/Content/Textures/
	TArray<FString> FileNames;
	const FString& FullPath = FPaths::ProjectContentDir() / TEXT("Textures/");
	const FString& SearchPattern = FString::Printf(TEXT("*.%s"), *FString("png"));
	
	IFileManager::Get().FindFiles(FileNames, *FullPath, *SearchPattern);

	ImageWrapper->SetRaw(TextureData.GetData(), TextureData.Num() * sizeof(uint16), LandscapeSize, LandscapeSize, ERGBFormat::Gray, 16);
	FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(), *(FPaths::ProjectContentDir() / FString::Printf(TEXT("Textures/T_HeightMap_%d.png"), FileNames.Num())));
}