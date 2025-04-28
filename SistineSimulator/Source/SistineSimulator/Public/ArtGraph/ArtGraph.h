// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArtGraph.generated.h"

class UGraphElement;

USTRUCT(BlueprintType)
struct SISTINESIMULATOR_API FNodeTransform
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	FVector Position;
};

USTRUCT(BlueprintType)
struct SISTINESIMULATOR_API FGraphEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> NodeA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> NodeB;
};

/**
 * 
 */
UCLASS()
class SISTINESIMULATOR_API UGraphElement : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Position;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
	TArray<FGraphEdge> AdjacencyList;
};
