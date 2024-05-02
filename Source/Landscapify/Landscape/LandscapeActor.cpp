// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeActor.h"

ALandscapeActor::ALandscapeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	LandscapeMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("LandscapeMeshComponent"));
	LandscapeMeshComponent->SetupAttachment(RootComponent);
	LandscapeMeshComponent->bUseComplexAsSimpleCollision = false;
	LandscapeMeshComponent->bUseAsyncCooking = true;
}

void ALandscapeActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALandscapeActor::GenerateLandscape()
{
	if (bForceRegenerate_Debug)
	{
		LandscapeSections.Reset();
	}

	LandscapeSize = static_cast<int32>(LandscapeSizeEnum) + 1;
	if (LandscapeSections.Num())
	{
		if (RegenerateLandscape())
		{
			return;
		}
	}
	// TODO: In big sized map we need to divide it into a section. For now it works as one peace of mesh
	for (int32 Index = 0; Index < 1; Index++)
	{
		FMeshSectionData LandscapeSection;
		GenerateHeightMap(LandscapeSection.Vertices);

		DiamondSquareStep(LandscapeSection, LandscapeSize * LandscapeSize);
		
		GenerateProceduralMeshData(LandscapeSection.Triangles, LandscapeSection.Normals);
	
		LandscapeSections.Emplace(LandscapeSection);
		LandscapeMeshComponent->CreateMeshSection(Index, LandscapeSection.Vertices, LandscapeSection.Triangles, LandscapeSection.Normals, LandscapeSection.UV, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	}
}

bool ALandscapeActor::RegenerateLandscape()
{
	const int32 CurrentLandscapeSize = LandscapeSections[0].Vertices.Num() / LandscapeSize;
	if (CurrentLandscapeSize == LandscapeSize)
	{
		for (int32 Index = 0; Index < LandscapeSections.Num(); Index++)
		{
			// TODO: Update vertices if its has the same size
		}
	}
	return false;
}

void ALandscapeActor::GenerateHeightMap(TArray<FVector>& OutVertices)
{
	for (int32 Y = 0; Y < LandscapeSize; Y++)
	{
		for (int32 X = 0; X < LandscapeSize; X++)
		{
			OutVertices.Add(FVector(X, Y, 0.f));
		}
	}
}

void ALandscapeActor::GenerateProceduralMeshData(TArray<int32>& OutTriangles, TArray<FVector>& OutNormals)
{
	/*
	*  4 x 4 Example (TL - TopLeft; TR - TopRight)
	*  [ 0   1  2  4]
	*  [ TL TR  7  8]
	*  [ BL BR 11 12]
	*  [ 13 14 15 16]
	*/

	for (int32 Y = 0; Y < LandscapeSize - 1; Y++)
	{
		for (int32 X = 0; X < LandscapeSize - 1; X++)
		{
			int32 TopLeft = (LandscapeSize * Y) + X;
			int32 TopRight = TopLeft + 1;
			int32 BottomLeft = TopLeft + LandscapeSize;
			int32 BottomRight = BottomLeft + 1;

			OutTriangles.Add(TopLeft);
			OutTriangles.Add(BottomLeft);
			OutTriangles.Add(TopRight);

			OutTriangles.Add(TopRight);
			OutTriangles.Add(BottomLeft);
			OutTriangles.Add(BottomRight);
		}
	}
}

void ALandscapeActor::DiamondSquareStep(FMeshSectionData& SectionData, const int32 CurrentStep)
{
	
}