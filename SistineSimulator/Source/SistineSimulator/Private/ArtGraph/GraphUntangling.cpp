// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/GraphUntangling.h"
#include "Kismet/GameplayStatics.h" // Include for UGameplayStatics
#include "ArtGraph/Untangleable.h"	// Include the interface header

// Sets default values
AGraphUntangling::AGraphUntangling()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGraphUntangling::BeginPlay()
{
	Super::BeginPlay();

	// Optionally re-run construction logic if needed at runtime,
	// though OnConstruction handles editor-time setup.
	// OnConstruction(GetActorTransform());
}

void AGraphUntangling::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	// If TargetGraph is not set, clear the list and the last constructed graph reference.
	if (!TargetGraph)
	{
		if (LastConstructedTargetGraph != nullptr) // Only clear if it was previously set
		{
			UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::OnConstruction: TargetGraph is null. Clearing UntangleableObjects."));
			UntangleableObjects.Empty();
			LastConstructedTargetGraph = nullptr;
		}
		return;
	}

	// If the current TargetGraph is the same as the one used last time, skip reconstruction.
	if (TargetGraph == LastConstructedTargetGraph)
	{
		// UE_LOG(LogTemp, Verbose, TEXT("AGraphUntangling::OnConstruction: TargetGraph (%s) has not changed. Skipping reconstruction."), *TargetGraph->GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::OnConstruction: TargetGraph changed to %s or first run. Reconstructing UntangleableObjects."), *TargetGraph->GetName());

	UntangleableObjects.Empty();
	LastConstructedTargetGraph = nullptr; // Clear last graph in case construction fails below

	// Ensure the graph's adjacency list is up-to-date (important if graph was modified)
	TargetGraph->UpdateAdjacencyList();
	const TArray<TArray<FGameplayTag>> &AdjacencyList = TargetGraph->GetAdjacencyList();

	if (AdjacencyList.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::OnConstruction: TargetGraph %s has an empty adjacency list."), *TargetGraph->GetName());
		return; // Keep LastConstructedTargetGraph null
	}

	// Find all actors in the current level that implement the Untangleable interface
	TArray<AActor *> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUntangleable::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::OnConstruction: No actors implementing Untangleable interface found in the level."));
		return; // Keep LastConstructedTargetGraph null
	}
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::OnConstruction: Found %d actors implementing Untangleable interface."), FoundActors.Num());

	// Create a map for quick lookup of actors by their tag
	TMap<FGameplayTag, TScriptInterface<IUntangleable>> TagToActorMap;
	for (AActor *Actor : FoundActors)
	{
		TScriptInterface<IUntangleable> UntangleableActor(Actor);
		if (UntangleableActor)
		{
			// Use the interface function directly
			FGameplayTag ActorTag = IUntangleable::Execute_GetTag(Actor);
			if (ActorTag.IsValid())
			{
				TagToActorMap.Add(ActorTag, UntangleableActor);
				UE_LOG(LogTemp, Verbose, TEXT("Found Untangleable Actor: %s with Tag: %s"), *Actor->GetName(), *ActorTag.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Untangleable Actor %s has an invalid tag."), *Actor->GetName());
			}
		}
	}

	// Construct the UntangleableObjects array based on the graph's adjacency list
	bool bConstructionSuccessful = true; // Track if construction completes without missing actors
	UntangleableObjects.Reserve(AdjacencyList.Num());
	for (const TArray<FGameplayTag> &NodeConnections : AdjacencyList)
	{
		TArray<TScriptInterface<IUntangleable>> InnerArray;
		InnerArray.Reserve(NodeConnections.Num());

		for (const FGameplayTag &Tag : NodeConnections)
		{
			if (TScriptInterface<IUntangleable> *FoundActor = TagToActorMap.Find(Tag))
			{
				InnerArray.Add(*FoundActor);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::OnConstruction: Could not find an actor with tag %s required by TargetGraph %s."), *Tag.ToString(), *TargetGraph->GetName());
				bConstructionSuccessful = false; // Mark as potentially incomplete
				// Optionally add a null entry or skip, depending on desired behavior
				// InnerArray.Add(nullptr);
			}
		}
		UntangleableObjects.Add(InnerArray);
	}

	if (bConstructionSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::OnConstruction: Successfully constructed UntangleableObjects with %d nodes for graph %s."), UntangleableObjects.Num(), *TargetGraph->GetName());
		// Update the last constructed graph reference only on successful completion
		LastConstructedTargetGraph = TargetGraph;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::OnConstruction: Finished constructing UntangleableObjects for graph %s, but some actors were missing."), *TargetGraph->GetName());
		// Keep LastConstructedTargetGraph null to allow reconstruction attempt on next run
	}
}

// Called every frame
void AGraphUntangling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

