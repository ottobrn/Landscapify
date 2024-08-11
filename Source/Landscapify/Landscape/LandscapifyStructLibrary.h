#pragma once
#include "ProceduralMeshComponent.h"
#include "LandscapifyStructLibrary.generated.h"

UENUM(BlueprintType)
enum ELandscapeSize : uint8
{
	E4x4 = 2			UMETA(DisplayName = "4x4"),
	E8x8 = 3			UMETA(DisplayName = "8x8"),
	E16x16 = 4			UMETA(DisplayName = "16x16"),
	E32x32 = 5			UMETA(DisplayName = "32x32"),
	E64x64 = 6			UMETA(DisplayName = "64x64"),
	E128x128 = 7		UMETA(DisplayName = "128x128"),
	E256x256 = 8		UMETA(DisplayName = "256x256"),
	E512x512 = 9		UMETA(DisplayName = "512x512"),
	E1024x1024 = 10	    UMETA(DisplayName = "1024x1024"),
	E2048x2048 = 11		UMETA(DisplayName = "2048x2048"),
	E4096x4096 = 12		UMETA(DisplayName = "4096x4096"),
	E8192x8192 = 13		UMETA(DisplayName = "8192x8192"),

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
	int32 Index = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<int32> Triangles = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector> Vertices = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector> Normals = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector2D> UV = {};

	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FLinearColor> VertexColor = {};
};

USTRUCT(BlueprintType)
struct FLandscapeParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	TObjectPtr<UProceduralMeshComponent> LandscapeMeshComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", DisplayName = "LandscapeSize")
	TEnumAsByte<ELandscapeSize> LandscapeSizeEnum = ELandscapeSize::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	bool bOverrideHeight = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "!bOverrideHeight"))
	int32 MaxRandomInitHeight = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float TopLeftHeight = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float TopRightHeight = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float BottomLeftHeight = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float BottomRightHeight = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	int32 Seed = 1234567890;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	float HeightMultiplier = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	float VertexSize = 10.f;
};


