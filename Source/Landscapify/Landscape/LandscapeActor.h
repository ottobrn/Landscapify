// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapifyStructLibrary.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "LandscapeActor.generated.h"

UCLASS()
class LANDSCAPIFY_API ALandscapeActor : public AActor
{
	GENERATED_BODY()

public:
	ALandscapeActor();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Actor")
	void GenerateLandscape();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Actor")
	void ResetLandscape();

private:
	void GenerateHeightMap(TArray<FVector>& OutVertices, TArray<FVector2D>& OutUVs);

	void RandomizeInitPoint(TArray<FVector>& OutVertices);

	void GenerateProceduralMeshData(TArray<int32>& OutTriangles, TArray<FVector>& OutNormals);

	void UpdateLandscapeSection(FMeshSectionData& InSection);

	void DiamondSquareStep(FMeshSectionData& SectionData, int32 CurrentStep, float Scale);
	
	bool IsDirty(const int32 CurrentLandscapeSize) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLandscapeParameters LandscapeSettings;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<FMeshSectionData> LandscapeSections;

	UPROPERTY()
	int32 LandscapeSize = 0;
};
