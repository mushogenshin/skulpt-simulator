// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArtGraph.h"
#include "Untangleable.h" // Include the new interface header
#include "GraphUntangling.generated.h"

UCLASS()
class SISTINESIMULATOR_API AGraphUntangling : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGraphUntangling();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> TargetGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	float KConstant;

	// Array of objects that implement the Untangleable interface and are part of the layout
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArtGraph")
	TArray<TScriptInterface<IUntangleable>> UntangleableObjects;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	float KSquared;
	float Temperature;
	TArray<FVector> Movements;
	uint32 CurrentIter;
	uint32 MaxIter;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

