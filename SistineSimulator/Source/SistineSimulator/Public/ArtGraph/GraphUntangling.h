// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArtGraph.h"
#include "Untangleable.h"
#include "GraphUntangling.generated.h"

UCLASS()
class SISTINESIMULATOR_API AGraphUntangling : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGraphUntangling();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	TObjectPtr<UGraphElement> TargetedGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph", meta = (ToolTip = "Additional tags to filter actors by. Actors must have the primary tag from the graph AND all of these secondary tags."))
	FGameplayTagContainer SecondaryTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph")
	float KConstant;

	// Array of arrays of objects that implement the Untangleable interface, structured like an adjacency list.
	// The first element of each inner array corresponds to a node, and the rest are its neighbors.
	TArray<TArray<TScriptInterface<IUntangleable>>> UntangleableAdjacencyList;

	// Function to manually refresh the UntangleableObjects list
	UFUNCTION(CallInEditor, Category = "ArtGraph", meta = (DisplayName = "Refresh Untangleable Actors"))
	void RefreshUntangleableActors();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// // Called when the actor is constructed or properties are changed in the editor
	// virtual void OnConstruction(const FTransform &Transform) override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

private:
	// Debug property to display the constructed UntangleableObjects list in the editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true", MultiLine = true))
	FString DebugAdjacencyList;

	// Stores the TargetGraph used in the last successful OnConstruction run
	UPROPERTY(Transient) // Prevent saving this state
	TObjectPtr<UGraphElement> LastConstructedTargetGraph;

	float KSquared;
	float Temperature;
	TArray<FVector> Movements;
	uint32 CurrentIter;
	uint32 MaxIter;

	// Helper function to find actors implementing Untangleable
	void FindUntangleableActorsByTags();

	// Helper function to format the DebugUntangleableObjects string
	void FormatDebugUntangleableObjects();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
