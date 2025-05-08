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

// void AGraphUntangling::OnConstruction(const FTransform &Transform)
// {
// 	Super::OnConstruction(Transform);
// 	// Always call RefreshUntangleableActors on construction,
// 	// but add a slight delay to ensure actors are fully initialized
// 	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
// 													  { RefreshUntangleableActors(); });
// }

void AGraphUntangling::RefreshUntangleableActors()
{
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::RefreshUntangleableActors."));
	FindUntangleableActorsByTags();
	FormatDebugUntangleableObjects();
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

void AGraphUntangling::FindUntangleableActorsByTags()
{
	// If TargetGraph is not set, clear the list and the last constructed graph reference.
	if (!TargetedGraph)
	{
		UE_LOG(LogTemp, Log,
		       TEXT("AGraphUntangling::FindUntangleableActorsByTags: TargetGraph is null. Clearing UntangleableObjects."
		       ));
		UntangleableAdjacencyList.Empty();
		return;
	}

	// Explicitly update the adjacency list before using it
	TargetedGraph->UpdateAdjacencyList();

	UntangleableAdjacencyList.Empty();
	const TArray<TArray<FGameplayTag>>& AdjacencyList = TargetedGraph->GetAdjacencyList();

	if (AdjacencyList.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "AGraphUntangling::FindUntangleableActorsByTags: TargetGraph %s has an empty adjacency list. Skipping."
		       ), *TargetedGraph->GetName());
		return;
	}

	// Find all actors in the current level that implement the Untangleable interface
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUntangleable::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "AGraphUntangling::FindUntangleableActorsByTags: No actors implementing Untangleable interface found in the level."
		       ));
		return;
	}

	bool bConstructionSuccessful = true; // Track if construction completes without missing actors
	UntangleableAdjacencyList.Reserve(AdjacencyList.Num());

	// Iterate through the desired graph structure (Adjacency List)
	for (const TArray<FGameplayTag>& NodeConnections : AdjacencyList)
	{
		TArray<TScriptInterface<IUntangleable>> InnerArray;
		InnerArray.Reserve(NodeConnections.Num());

		// For each tag (node or neighbor) in the graph structure
		for (const FGameplayTag& RequiredPrimaryTag : NodeConnections)
		{
			if (!RequiredPrimaryTag.IsValid())
			{
				UE_LOG(LogTemp, Warning,
				       TEXT(
					       "AGraphUntangling::FindUntangleableActorsByTags: Encountered invalid tag in TargetGraph's adjacency list. Skipping."
				       ));
				InnerArray.Add(nullptr); // Add null to maintain array structure
				continue;
			}

			// Define the required tags for the actor: the primary tag from the graph + ALL secondary tags
			FGameplayTagContainer RequiredTags;
			RequiredTags.AddTag(RequiredPrimaryTag);
			RequiredTags.AppendTags(SecondaryTags); // Add all secondary tags

			bool bFoundMatchingActor = false;
			// Iterate through all found actors to find one that matches the required tags
			for (AActor* Actor : FoundActors)
			{
				if (!Actor)
				{
					UE_LOG(LogTemp, Warning, TEXT("Null actor found in FoundActors array"));
					continue;
				}
				if (Actor->GetClass()->ImplementsInterface(UUntangleable::StaticClass()))
				{
					FGameplayTagContainer ActorTags = IUntangleable::Execute_GetTags(Actor);

					// Check if the actor has ALL the required tags
					if (ActorTags.HasAll(RequiredTags))
					{
						// Found a matching actor
						TScriptInterface<IUntangleable> UntangleableActor;
						UntangleableActor.SetObject(Actor);
						// UntangleableActor.SetInterface(Cast<IUntangleable>(Actor)); // Safe cast

						InnerArray.Add(UntangleableActor);
						UE_LOG(LogTemp, Display,
						       TEXT("Matched Actor %s for Primary Tag %s (Required Secondary Tags: %s)"),
						       *Actor->GetName(), *RequiredPrimaryTag.ToString(), *SecondaryTags.ToString());
						bFoundMatchingActor = true;
						break; // Stop searching for actors for this specific RequiredPrimaryTag
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning,
					       TEXT("Actor %s was returned by GetAllActorsWithInterface but doesn't implement Untangleable"
					       ), *Actor->GetName());
				}
			} // End loop through FoundActors

			if (!bFoundMatchingActor)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT(
					       "AGraphUntangling::FindUntangleableActorsByTags: Could not find any actor matching required tags: Primary=%s, Secondary=%s"
				       ),
				       *RequiredPrimaryTag.ToString(), *SecondaryTags.ToString());
				bConstructionSuccessful = false; // Mark as potentially incomplete
				// Add a null entry to represent the missing actor in the structure
				InnerArray.Add(nullptr);
			}
		} // End loop through NodeConnections (tags)

		UntangleableAdjacencyList.Add(InnerArray);
	} // End loop through AdjacencyList

	if (bConstructionSuccessful)
	{
		UE_LOG(LogTemp, Log,
		       TEXT(
			       "AGraphUntangling::FindUntangleableActorsByTags: Successfully constructed UntangleableObjects with %d nodes for graph %s."
		       ), UntangleableAdjacencyList.Num(), *TargetedGraph->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "AGraphUntangling::FindUntangleableActorsByTags: Finished constructing UntangleableObjects for graph %s, but some actors were missing."
		       ), *TargetedGraph->GetName());
	}
}

void AGraphUntangling::FormatDebugUntangleableObjects()
{
	DebugAdjacencyList.Empty(); // Clear the debug string

	for (const TArray<TScriptInterface<IUntangleable>>& NodeConnections : UntangleableAdjacencyList)
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
		if (&NodeConnections != &UntangleableAdjacencyList.Last())
		{
			DebugAdjacencyList += TEXT("\n\n");
		}
	}
}

void AGraphUntangling::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AGraphUntangling, TargetedGraph) || PropertyName ==
			GET_MEMBER_NAME_CHECKED(AGraphUntangling, SecondaryTags))
		{
			UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::PostEditChangeProperty: Relevant property changed: %s"),
			       *PropertyName.ToString());
			RefreshUntangleableActors();
		}
	}
}
