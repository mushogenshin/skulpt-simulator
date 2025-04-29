#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ArtGraph.generated.h"

class UGraphElement;

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
 * 
 */
UCLASS()
class SISTINESIMULATOR_API UGraphElement : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag Tag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FGraphEdge> AdjacencyList;
};

