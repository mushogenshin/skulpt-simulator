#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ArtGraph.generated.h"

class UGraphElement;

/**
 * A struct representing an edge in a graph, connecting two graph elements.
 * This is used to represent the connections between different elements in the art graph.
 */
USTRUCT(BlueprintType)
struct SISTINESIMULATOR_API FGraphEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> ElementA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> ElementB;
};

/**
 * A data asset representing a graph element in the art graph.
 * This class contains a tag and a list of edges within the graph.
 */
UCLASS()
class SISTINESIMULATOR_API UGraphElement : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FGraphEdge> Edges;

	TArray<TPair<FGameplayTag, FGameplayTag>> GetEdgesAsTagPairs() const;
};
