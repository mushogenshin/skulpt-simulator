// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph",
		meta = (ToolTip =
			"Additional tags to filter actors by. Actors must have the primary tag from the graph AND all of these secondary tags to match."
		))
	FGameplayTagContainer SecondaryTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArtGraph",
		meta = (DisplayName = "Fruchterman-Reingold K Constant", ToolTip =
			"This constant determines the distance between nodes to stabilize towards."
		))
	float KConstantUser;

	// Array of arrays of objects that implement the Untangleable interface, structured like an adjacency list.
	// The first element of each inner array corresponds to a node, and the rest are its neighbors.
	TArray<TArray<TScriptInterface<IUntangleable>>> UntangleableAdjacencyList;

	// Array of arrays of AActor pointers, mirroring UntangleableAdjacencyList.
	// The first element of each inner array corresponds to a node actor, and the rest are its neighbor actors.
	TArray<TArray<AActor*>> ActorAdjacencyList;

	// Function to manually refresh the UntangleableObjects list
	UFUNCTION(CallInEditor, Category = "ArtGraph", meta = (DisplayName = "Refresh Untangleable Actors"))
	void RefreshUntangleableActors();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the actor is constructed or properties are changed in the editor
	virtual void OnConstruction(const FTransform& Transform) override;

	// virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PreviewMesh;

	// Map actors to their indices in ActorAdjacencyList for fast lookup
	UPROPERTY()
	TMap<AActor*, int32> ActorToIndexMap;

	// Debug property to display the constructed UntangleableObjects list in the editor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ArtGraph",
		meta = (AllowPrivateAccess = "true", MultiLine = true))
	FString DebugAdjacencyList;

	float KConstant;
	float KSquared;
	float Temperature; // maximum allowable movement, used for cooling mechanism
	uint32 NumNodes;
	TArray<FVector> Positions;
	TArray<FVector> Movements;
	uint32 CurrentIter;
	uint32 MaxIter;

	// Helper function to find actors implementing Untangleable
	void FindImplementorsWithTags();

	// Helper function to store the found actors in UntangleableAdjacencyList
	void CastToUntangleableActors();

	// Helper to update ActorToIndexMap
	void UpdateActorToIndexMap();

	// Helper function to initialize graph parameters
	void InitializeGraphParameters();

	// Helper function to format the DebugUntangleableObjects string
	void FormatDebugUntangleableObjects();

	// Helper to draw lines between nodes and their neighbors
	void DrawAdjacencyLines(float LineThickness = 2.0f, bool PersistentLines = false, float LineDuration = 5.0f,
	                        FColor LineColor = FColor::Yellow);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// A single Fruchterman-Reingold step for current ActorAdjacencyList
	void DoStep();
};
