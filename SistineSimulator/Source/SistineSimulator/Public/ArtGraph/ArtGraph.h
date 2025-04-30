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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Self")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FGraphEdge> Edges;

	// Get the cached adjacency list
	TArray<TArray<FGameplayTag>> GetAdjacencyList() const;

	// Helper function to update the cached adjacency list
	void UpdateAdjacencyList();

	// Get all elements referenced by this graph
	TArray<UGraphElement *> GetReferencedElements() const;

protected:
	// Override PostEditChangeProperty to update the adjacency list when properties change
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

private:
	// Cache for the adjacency list
	TArray<TArray<FGameplayTag>> CachedAdjacencyList;

	// Debug property to display the cached adjacency list in the editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true", MultiLine = true))
	FString DebugAdjacencyList;
};
