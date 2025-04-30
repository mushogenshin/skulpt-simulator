// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/GraphUntangling.h"
#include "Kismet/GameplayStatics.h"
#include "ArtGraph/Untangleable.h"

// Sets default values
AGraphUntangling::AGraphUntangling()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AGraphUntangling::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);
	MakeAdjacencyListFromTags();
}

// Called when the game starts or when spawned
void AGraphUntangling::BeginPlay()
{
	Super::BeginPlay();

	// Optionally re-run construction logic if needed at runtime,
	// though OnConstruction handles editor-time setup.
	// OnConstruction(GetActorTransform());
}

// Called every frame
void AGraphUntangling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGraphUntangling::MakeAdjacencyListFromTags()
{
	// If TargetGraph is not set, clear the list and the last constructed graph reference.
	if (!TargetGraph)
	{
		if (LastConstructedTargetGraph != nullptr) // Only clear if it was previously set
		{
			UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: TargetGraph is null. Clearing UntangleableObjects."));
			AdjacencyList.Empty();
			DebugAdjacencyList.Empty(); // Clear debug string
			LastConstructedTargetGraph = nullptr;
		}
		return;
	}

	// If the current TargetGraph and SecondaryTags are the same as the ones used last time, skip reconstruction.
	// Note: This simple check might not catch all cases if SecondaryTags changes but TargetGraph doesn't.
	// A more robust check would involve comparing SecondaryTags as well.
	if (TargetGraph == LastConstructedTargetGraph)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: TargetGraph (%s) or SecondaryTags changed, or first run. Reconstructing UntangleableObjects."), *TargetGraph->GetName());

	AdjacencyList.Empty();
	DebugAdjacencyList.Empty();			  // Clear debug string initially
	LastConstructedTargetGraph = nullptr; // Clear last graph in case construction fails below

	// Ensure the graph's adjacency list is up-to-date
	TargetGraph->UpdateAdjacencyList();
	const TArray<TArray<FGameplayTag>> &AdjacencyList = TargetGraph->GetAdjacencyList();

	if (AdjacencyList.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: TargetGraph %s has an empty adjacency list."), *TargetGraph->GetName());
		return; // Keep LastConstructedTargetGraph null
	}

	// Find all actors in the current level that implement the Untangleable interface
	TArray<AActor *> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUntangleable::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: No actors implementing Untangleable interface found in the level."));
		return; // Keep LastConstructedTargetGraph null
	}
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: Found %d actors implementing Untangleable interface."), FoundActors.Num());

	bool bConstructionSuccessful = true; // Track if construction completes without missing actors
	AdjacencyList.Reserve(AdjacencyList.Num());

	// Iterate through the desired graph structure (Adjacency List)
	for (const TArray<FGameplayTag> &NodeConnections : AdjacencyList)
	{
		TArray<TScriptInterface<IUntangleable>> InnerArray;
		InnerArray.Reserve(NodeConnections.Num());

		// For each tag (node or neighbor) in the graph structure
		for (const FGameplayTag &RequiredPrimaryTag : NodeConnections)
		{
			if (!RequiredPrimaryTag.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: Encountered invalid tag in TargetGraph's adjacency list. Skipping."));
				continue;
			}

			// Define the required tags for the actor: the primary tag from the graph + ALL secondary tags
			FGameplayTagContainer RequiredTags;
			RequiredTags.AddTag(RequiredPrimaryTag);
			RequiredTags.AppendTags(SecondaryTags); // Add all secondary tags

			bool bFoundMatchingActor = false;
			// Iterate through all found actors to find one that matches the required tags
			for (AActor *Actor : FoundActors)
			{
				if (!Actor || !Actor->GetClass()->ImplementsInterface(UUntangleable::StaticClass()))
				{
					continue; // Skip invalid actors or those not implementing the interface
				}

				FGameplayTagContainer ActorTags = IUntangleable::Execute_GetTags(Actor);

				// Check if the actor has ALL the required tags
				if (ActorTags.HasAll(RequiredTags))
				{
					// Found a matching actor
					TScriptInterface<IUntangleable> UntangleableActor;
					UntangleableActor.SetObject(Actor);
					UntangleableActor.SetInterface(Cast<IUntangleable>(Actor)); // Safe cast

					InnerArray.Add(UntangleableActor);
					UE_LOG(LogTemp, Verbose, TEXT("Matched Actor %s for Primary Tag %s (Required Secondary Tags: %s)"), *Actor->GetName(), *RequiredPrimaryTag.ToString(), *SecondaryTags.ToString());
					bFoundMatchingActor = true;
					break; // Stop searching for actors for this specific RequiredPrimaryTag
				}
			} // End loop through FoundActors

			if (!bFoundMatchingActor)
			{
				UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: Could not find any actor matching required tags: Primary=%s, Secondary=%s"),
					   *RequiredPrimaryTag.ToString(), *SecondaryTags.ToString());
				bConstructionSuccessful = false; // Mark as potentially incomplete
				// Add a null entry to represent the missing actor in the structure
				InnerArray.Add(nullptr);
			}
		} // End loop through NodeConnections (tags)

		AdjacencyList.Add(InnerArray);
	}

	// Always format the debug string, even if construction was incomplete
	FormatDebugAdjacenctyList();

	if (bConstructionSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: Successfully constructed UntangleableObjects with %d nodes for graph %s."), AdjacencyList.Num(), *TargetGraph->GetName());
		// Update the last constructed graph reference only on successful completion
		LastConstructedTargetGraph = TargetGraph;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindUntangleableActorsWithTag: Finished constructing UntangleableObjects for graph %s, but some actors were missing."), *TargetGraph->GetName());
		// Keep LastConstructedTargetGraph null to allow reconstruction attempt on next run
	}
}

void AGraphUntangling::FormatDebugAdjacenctyList()
{
	DebugAdjacencyList.Empty(); // Clear the debug string

	for (const TArray<TScriptInterface<IUntangleable>> &NodeConnections : AdjacencyList)
	{
		FString KeyName = TEXT("INVALID_NODE"); // Default if node is missing/invalid
		if (NodeConnections.Num() > 0)
		{
			if (NodeConnections[0].GetObject())
			{
				// Use the object's name directly
				KeyName = NodeConnections[0].GetObject()->GetName();
			}
			else
			{
				KeyName = TEXT("NULL_NODE"); // Node entry exists but is null
			}

			DebugAdjacencyList += FString::Printf(TEXT("%s -> ["), *KeyName);

			// Start from index 1 to get neighbors
			for (int32 i = 1; i < NodeConnections.Num(); ++i)
			{
				if (NodeConnections[i].GetObject())
				{
					// Use the neighbor object's name directly
					FString NeighborName = NodeConnections[i].GetObject()->GetName();
					DebugAdjacencyList += NeighborName;
				}
				else
				{
					DebugAdjacencyList += TEXT("NULL"); // Handle null neighbor entries
				}

				if (i < NodeConnections.Num() - 1)
				{
					DebugAdjacencyList += TEXT(", ");
				}
			}
			DebugAdjacencyList += TEXT("]");
		}
		else
		{
			// Handle case where the inner array itself is empty (shouldn't happen with current logic but good practice)
			DebugAdjacencyList += TEXT("EMPTY_NODE_ARRAY");
		}

		// Add newline for readability, check if it's not the last element
		if (&NodeConnections != &AdjacencyList.Last())
		{
			DebugAdjacencyList += TEXT("\n\n");
		}
	}
}
