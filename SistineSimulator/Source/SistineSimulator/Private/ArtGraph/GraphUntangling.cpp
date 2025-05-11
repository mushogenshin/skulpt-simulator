// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/GraphUntangling.h"
#include "Kismet/GameplayStatics.h"
#include "ArtGraph/Untangleable.h"
#include "DrawDebugHelpers.h"

namespace
{
	constexpr TCHAR GStatic_Mesh_Asset_Path[] = TEXT(
		"/Engine/Functions/Engine_MaterialFunctions02/ExampleContent/PivotPainter2/SimplePivotPainterExample.SimplePivotPainterExample");
}

AGraphUntangling::AGraphUntangling()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create StaticMeshComponent and set as root
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(PreviewMesh);

	// Set PreviewMesh to be hidden in game
	PreviewMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(GStatic_Mesh_Asset_Path);
	if (MeshAsset.Succeeded())
	{
		PreviewMesh->SetStaticMesh(MeshAsset.Object);
		PreviewMesh->SetWorldScale3D(FVector(3.0f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find static mesh asset at %s"), GStatic_Mesh_Asset_Path);
	}
}

// Called when the game starts or when spawned
void AGraphUntangling::BeginPlay()
{
	Super::BeginPlay();

	// Parameters
	RefreshUntangleableActors();
	InitializeGraphParameters();
}

void AGraphUntangling::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	DrawAdjacencyLines();
}

void AGraphUntangling::InitializeGraphParameters()
{
	KConstant = KConstantUser > 0.f ? KConstantUser : 15.f;
	KSquared = KConstant * KConstant;
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::InitializeGraphParameters: KConstant: %f, KSquared: %f"), KConstant, KSquared);

	NumNodes = ActorAdjacencyList.Num();
	Temperature = 10.f * FMath::Sqrt(static_cast<float>(NumNodes));
	Positions.SetNum(NumNodes);
	Movements.SetNumZeroed(NumNodes);

	UE_LOG(LogTemp, Log,
	       TEXT("AGraphUntangling::InitializeGraphParameters: NumNodes=%d, Temperature=%.2f, PositionsSize=%d, MovementsSize=%d"),
	       NumNodes, Temperature, Positions.Num(), Movements.Num());
}

void AGraphUntangling::RefreshUntangleableActors()
{
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::RefreshUntangleableActors."));
	FindImplementorsWithTags();
	CastToUntangleableActors();
	UpdateActorToIndexMap();
	FormatDebugUntangleableObjects();
}

// Find actors that implement Untangleable and match the required tags.
//
//   - A primary tag is defined on each node of the TargetGraph.
//  - Secondary tags are defined on the AGraphUntangling actor.
//
// The actor must have the primary tag from the graph and all secondary tags to
// be considered a match.
void AGraphUntangling::FindImplementorsWithTags()
{
	// If TargetGraph is not set, clear the list and the last constructed graph reference.
	if (!TargetedGraph)
	{
		UE_LOG(LogTemp, Log,
		       TEXT(
			       "AGraphUntangling::FindImplementorsWithTags: TargetGraph is null. Clearing UntangleableAdjacencyList."
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
			       "AGraphUntangling::FindImplementorsWithTags: TargetGraph %s has an empty adjacency list. Skipping."),
		       *TargetedGraph->GetName());
		return;
	}

	// Find all actors in the current level that implement the Untangleable interface
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUntangleable::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "AGraphUntangling::FindImplementorsWithTags: No actors implementing Untangleable interface found in the level."
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
					       "AGraphUntangling::FindImplementorsWithTags: Encountered invalid tag in TargetGraph's adjacency list. Skipping."
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
					UE_LOG(LogTemp, Warning, TEXT("AGraphUntangling::FindImplementorsWithTags: Null actor found in FoundActors array"));
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
						UE_LOG(LogTemp, Log,
						       TEXT(
							       "AGraphUntangling::FindImplementorsWithTags: Matched Actor %s for Primary Tag %s (Required Secondary Tags: %s)"
						       ),
						       *Actor->GetName(), *RequiredPrimaryTag.ToString(), *SecondaryTags.ToString());
						bFoundMatchingActor = true;
						break; // Stop searching for actors for this specific RequiredPrimaryTag
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning,
					       TEXT("AGraphUntangling::FindImplementorsWithTags: Actor %s was returned by GetAllActorsWithInterface but doesn't implement Untangleable"
					       ), *Actor->GetName());
				}
			} // End loop through FoundActors

			if (!bFoundMatchingActor)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT(
					       "AGraphUntangling::FindImplementorsWithTags: Could not find any actor matching required tags: Primary=%s, Secondary=%s"
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
			       "AGraphUntangling::FindImplementorsWithTags: Successfully constructed UntangleableAdjacencyList with %d nodes for graph %s."
		       ),
		       UntangleableAdjacencyList.Num(), *TargetedGraph->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "AGraphUntangling::FindImplementorsWithTags: Finished constructing UntangleableAdjacencyList for graph %s, but some actors were missing."
		       ),
		       *TargetedGraph->GetName());
	}
}

void AGraphUntangling::CastToUntangleableActors()
{
	// Clear the existing ActorAdjacencyList
	ActorAdjacencyList.Empty();

	// Populate ActorAdjacencyList based on UntangleableAdjacencyList
	for (const TArray<TScriptInterface<IUntangleable>>& UntangleableInnerList : UntangleableAdjacencyList)
	{
		TArray<AActor*> ActorInnerList;
		for (const TScriptInterface<IUntangleable>& UntangleableItem : UntangleableInnerList)
		{
			if (AActor* ActorItem = Cast<AActor>(UntangleableItem.GetObject()))
			{
				ActorInnerList.Add(ActorItem);
			}
			else
			{
				// Warn if the cast fails, though an IUntangleable should ideally be an AActor or derived from it
				UE_LOG(LogTemp, Warning, TEXT("RefreshUntangleableActors: UntangleableItem %s is not an AActor."),
				       *UntangleableItem.GetObject()->GetName());
			}
		}
		ActorAdjacencyList.Add(ActorInnerList);
	}
}

void AGraphUntangling::FormatDebugUntangleableObjects()
{
	DebugAdjacencyList.Empty(); // Clear the debug string

	for (const TArray<AActor*>& NodeConnections : ActorAdjacencyList)
	{
		FString KeyName = TEXT("INVALID_NODE"); // Default if node is missing/invalid
		if (NodeConnections.Num() > 0)
		{
			if (NodeConnections[0])
			{
				// Use the actor's name directly
				KeyName = NodeConnections[0]->GetName();
			}
			else
			{
				KeyName = TEXT("NULL_NODE"); // Node entry exists but is null
			}

			DebugAdjacencyList += FString::Printf(TEXT("%s -> ["), *KeyName);

			// Start from index 1 to get neighbors
			for (int32 i = 1; i < NodeConnections.Num(); ++i)
			{
				if (NodeConnections[i])
				{
					// Use the neighbor actor's name directly
					FString NeighborName = NodeConnections[i]->GetName();
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
			// Handle case where the inner array itself is empty
			DebugAdjacencyList += TEXT("EMPTY_NODE_ARRAY");
		}

		// Add newline for readability, check if it's not the last element
		if (&NodeConnections != &ActorAdjacencyList.Last())
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

void AGraphUntangling::UpdateActorToIndexMap()
{
	ActorToIndexMap.Empty();
	for (int32 i = 0; i < ActorAdjacencyList.Num(); ++i)
	{
		if (ActorAdjacencyList[i].Num() > 0 && ActorAdjacencyList[i][0])
		{
			ActorToIndexMap.Add(ActorAdjacencyList[i][0], i);
		}
	}
}

void AGraphUntangling::DoStep()
{
	// Gather current positions
	for (int32 i = 0; i < NumNodes; ++i)
	{
		if (const AActor* NodeActor = (ActorAdjacencyList[i].Num() > 0) ? ActorAdjacencyList[i][0] : nullptr)
		{
			Positions[i] = NodeActor->GetActorLocation();
		}
		else
		{
			Positions[i] = FVector::ZeroVector;
		}
	}

	// Repulsion between all pairs
	for (int32 v = 0; v < NumNodes; ++v)
	{
		for (int32 u = v + 1; u < NumNodes; ++u)
		{
			// No need to repulse self
			if (v == u)
				continue;

			FVector Delta = Positions[v] - Positions[u];
			const float Dist = Delta.Size();

			if (Dist < KINDA_SMALL_NUMBER)
				continue;

			// Not worth computing if the distance is too large
			if (Dist > 1000.f)
				continue;

			const float Repulsion = KSquared / Dist;
			FVector Dir = Delta / Dist;
			Movements[v] += Dir * Repulsion;
			Movements[u] -= Dir * Repulsion;

			// Debug print for repulsion
			const AActor* ActorV = (ActorAdjacencyList[v].Num() > 0) ? ActorAdjacencyList[v][0] : nullptr;
			const AActor* ActorU = (ActorAdjacencyList[u].Num() > 0) ? ActorAdjacencyList[u][0] : nullptr;
			if (ActorV && ActorU)
			{
				FString Msg = FString::Printf(TEXT("Repulsion: %s <-> %s | Dist: %.2f | Rep: %.2f"),
				                              *ActorV->GetName(), *ActorU->GetName(), Dist, Repulsion);
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Msg);
			}
		}
	}

	// Attraction along edges
	for (int32 v = 0; v < NumNodes; ++v)
	{
		const TArray<AActor*>& NodeList = ActorAdjacencyList[v];
		AActor* NodeActor = (NodeList.Num() > 0) ? NodeList[0] : nullptr;
		if (!NodeActor)
			continue;

		for (int32 n = 1; n < NodeList.Num(); ++n)
		{
			const AActor* NeighborActor = NodeList[n];
			if (!NeighborActor)
				continue;

			// Optimized: Use ActorToIndexMap instead of linear search
			int32 NeighborIdx = INDEX_NONE;
			if (const int32* FoundIdx = ActorToIndexMap.Find(NeighborActor))
			{
				NeighborIdx = *FoundIdx;
			}
			if (NeighborIdx == INDEX_NONE || NeighborIdx == v)
				continue;
			if (NeighborIdx > v)
				continue; // Only process each edge once

			FVector Delta = Positions[v] - Positions[NeighborIdx];
			const float Dist = Delta.Size();
			if (Dist < KINDA_SMALL_NUMBER)
				continue;

			const float Attraction = (Dist * Dist) / KConstant;
			FVector Dir = Delta / Dist;
			Movements[v] -= Dir * Attraction;
			Movements[NeighborIdx] += Dir * Attraction;

			// Debug print for attraction
			const AActor* ActorV = NodeActor;
			const AActor* ActorN = NeighborActor;
			if (ActorV && ActorN)
			{
				FString Msg = FString::Printf(TEXT("Attraction: %s <-> %s | Dist: %.2f | Attr: %.2f"),
				                              *ActorV->GetName(), *ActorN->GetName(), Dist, Attraction);
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, Msg);
			}
		}
	}

	// Cap movement by temperature and apply
	for (int32 v = 0; v < NumNodes; ++v)
	{
		const float MoveNorm = Movements[v].Size();
		// < 1.0: No need to cap movements for those that are already moving very little
		if (MoveNorm < 1.f)
			continue;
		const float CappedNorm = FMath::Min(MoveNorm, Temperature);
		FVector CappedMove = Movements[v] / MoveNorm * CappedNorm;

		if (AActor* NodeActor = (ActorAdjacencyList[v].Num() > 0) ? ActorAdjacencyList[v][0] : nullptr)
		{
			NodeActor->SetActorLocation(NodeActor->GetActorLocation() + CappedMove);

			// Debug print for movement
			FString Msg = FString::Printf(TEXT("Move: %s | Δ: (%.2f, %.2f, %.2f) | Norm: %.2f"),
			                              *NodeActor->GetName(), CappedMove.X, CappedMove.Y, CappedMove.Z, CappedNorm);
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, Msg);
		}
	}

	// Cool down fast until we reach 1.5, then stay at low temperature
	if (Temperature > 1.5f)
	{
		Temperature *= 0.85f;
	}
	else
	{
		Temperature = 1.5f;
	}

	// // Log the current temperature
	// UE_LOG(LogTemp, Log, TEXT("Current Temperature: %.2f"), Temperature);
}

void AGraphUntangling::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoStep();
}

void AGraphUntangling::DrawAdjacencyLines()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	const float LineThickness = 2.0f;
	const float LineDuration = 5.0f;
	const FColor LineColor = FColor::Yellow;

	for (const TArray<AActor*>& NodeConnections : ActorAdjacencyList)
	{
		if (NodeConnections.Num() < 2 || !NodeConnections[0])
			continue;

		const FVector Start = NodeConnections[0]->GetActorLocation();

		for (int32 i = 1; i < NodeConnections.Num(); ++i)
		{
			AActor* Neighbor = NodeConnections[i];
			if (!Neighbor)
				continue;

			const FVector End = Neighbor->GetActorLocation();
			DrawDebugLine(World, Start, End, LineColor, false, LineDuration, 0, LineThickness);
		}
	}
}
