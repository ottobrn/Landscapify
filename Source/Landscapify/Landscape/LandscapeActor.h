// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "LandscapeActor.generated.h"

UENUM(BlueprintType)
enum ELandscapeSize
{
	E4x4 = 4			UMETA(DisplayName = "4x4"),
	E8x8 = 8			UMETA(DisplayName = "8x8"),
	E16x16 = 16			UMETA(DisplayName = "16x16"),
	E32x32 = 32			UMETA(DisplayName = "32x32"),
	E64x64 = 64			UMETA(DisplayName = "64x64"),
	E128x128 = 128		UMETA(DisplayName = "128x128"),
	E256x256 = 256		UMETA(DisplayName = "256x256"),
	E512x512 = 512		UMETA(DisplayName = "512x512"),
	E1024x1024 = 1024	UMETA(DisplayName = "1024x1024"),
	E2048x2048 = 2048	UMETA(DisplayName = "2048x2048"),
	E4096x4096 = 4096	UMETA(DisplayName = "4096x4096"),
	E8192x8192 = 8192	UMETA(DisplayName = "8192x8192"),

	None = 0
};

USTRUCT(BlueprintType)
struct FMeshSectionData
{
	GENERATED_BODY()

	FMeshSectionData()
	{}

public:
	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<int32> Triangles = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector> Vertices = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector> Normals = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector2D> UV = {};
};

UCLASS()
class LANDSCAPIFY_API ALandscapeActor : public AActor
{
	GENERATED_BODY()

public:
	ALandscapeActor();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape")
	void GenerateLandscape();

private:
	bool RegenerateLandscape();

	void GenerateHeightMap(TArray<FVector>& OutVertices);

	void GenerateProceduralMeshData(TArray<int32>& OutTriangles, TArray<FVector>& OutNormals);

	void DiamondSquareStep(FMeshSectionData& SectionData, const int32 CurrentStep);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TObjectPtr<UProceduralMeshComponent> LandscapeMeshComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", DisplayName = "LandscapeSize")
	TEnumAsByte<ELandscapeSize> LandscapeSizeEnum = ELandscapeSize::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	float Roughness = 0.9f;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<FMeshSectionData> LandscapeSections;

	UPROPERTY(EditAnywhere)
	bool bForceRegenerate_Debug = false;

	UPROPERTY()
	int32 LandscapeSize = 0;
};
