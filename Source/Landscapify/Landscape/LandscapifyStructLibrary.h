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

UENUM(BlueprintType)
enum EAlgorithm : uint8
{
	DiamondSquare,
	PerlinNoise
};


USTRUCT(BlueprintType)
struct FMeshSectionData
{
	GENERATED_BODY()

	FMeshSectionData()
	{}

public:
	// Now only the array with vertices is filled,
	// the rest was removed because I decided to generate a texture and then use it as a height map in Landscape.
	UPROPERTY(BlueprintReadOnly, Category = "Landscape")
	TArray<FVector> Vertices = {};
};

USTRUCT(BlueprintType)
struct FLandscapeParameters
{
	GENERATED_BODY()

	// This enum allows the user to select a landscape size from the available options in ELandscapeSize.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", DisplayName = "LandscapeSize")
	TEnumAsByte<ELandscapeSize> LandscapeSizeEnum = ELandscapeSize::None;

	// This enum allows the user to choose the algorithm used for landscape generation,
	// defaulting to the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", DisplayName = "LandscapeSize")
	TEnumAsByte<EAlgorithm> GeneratorAlgorithm = EAlgorithm::DiamondSquare;

	// Seed for random number generation, used in landscape algorithms.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape")
	int64 Seed = 1234567890;

	// Overrides the randomly generated corner heights when using the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape|DiamondSquare", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::DiamondSquare", EditConditionHides))
	bool bOverrideHeight = false;

	// Maximum random initial height for the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "!bOverrideHeight && GeneratorAlgorithm == EAlgorithm::DiamondSquare"))
	int32 MaxRandomInitHeight = 100.f;

	// Height of the top-left corner when overriding heights in the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float TopLeftHeight = 1000.f;

	// Height of the top-right corner when overriding heights in the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float TopRightHeight = 1000.f;

	// Height of the bottom-left corner when overriding heights in the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float BottomLeftHeight = 1000.f;

	// Height of the bottom-right corner when overriding heights in the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "bOverrideHeight", EditConditionHides))
	float BottomRightHeight = 1000.f;

	// Multiplier for the height values in the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::DiamondSquare", EditConditionHides))
	float HeightMultiplier = 50.f;

	// Size of each vertex in the landscape mesh for the Diamond-Square algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::DiamondSquare", EditConditionHides))
	float VertexSize = 10.f;

	// Scale of the noise pattern used in the Perlin noise algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::PerlinNoise", EditConditionHides))
	float NoiseScale = 0.1f;

	// Number of octaves used in the Perlin noise algorithm to add detail.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::PerlinNoise", EditConditionHides))
	int32 Octaves = 4;

	// Persistence controls the amplitude decrease in each octave of the Perlin noise algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::PerlinNoise", EditConditionHides))
	float Persistence = 0.5f;

	// Lacunarity controls the frequency increase in each octave of the Perlin noise algorithm.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::PerlinNoise", EditConditionHides))
	float Lacunarity = 2.0f;

	// Maximum height that the Perlin noise algorithm can generate.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landscape", meta = (EditCondition = "GeneratorAlgorithm == EAlgorithm::PerlinNoise", EditConditionHides))
	float MaxHeight = 500.f;
};


