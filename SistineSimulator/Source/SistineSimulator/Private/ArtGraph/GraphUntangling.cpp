// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/GraphUntangling.h"
#include "Kismet/GameplayStatics.h"
#include "ArtGraph/Untangleable.h"

namespace
{
	constexpr TCHAR STATIC_MESH_ASSET_PATH[] = TEXT(
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(STATIC_MESH_ASSET_PATH);
	if (MeshAsset.Succeeded())
	{
		PreviewMesh->SetStaticMesh(MeshAsset.Object);
		PreviewMesh->SetWorldScale3D(FVector(3.0f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find static mesh asset at %s"), STATIC_MESH_ASSET_PATH);
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

// void AGraphUntangling::OnConstruction(const FTransform &Transform)// void AGraphUntangling::OnConstruction(const FTransform &Transform)
// {
// 	Super::OnConstruction(Transform);
// 	// Always call RefreshUntangleableActors on construction,
// 	// but add a slight delay to ensure actors are fully initialized
// 	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
// 													  { RefreshUntangleableActors(); });
// }

void AGraphUntangling::InitializeGraphParameters()
{
	KConstant = KConstantUser > 0.f ? KConstantUser : 15.f;
	KSquared = KConstant * KConstant;
	UE_LOG(LogTemp, Log, TEXT("KConstant: %f, KSquared: %f"), KConstant, KSquared);

	NumNodes = ActorAdjacencyList.Num();
	Temperature = 10.f * FMath::Sqrt(static_cast<float>(NumNodes));
	Positions.SetNum(NumNodes);
	Movements.SetNumZeroed(NumNodes);

	UE_LOG(LogTemp, Log, TEXT("Graph Parameters Initialized: NumNodes=%d, Temperature=%.2f, PositionsSize=%d, MovementsSize=%d"),
		   NumNodes, Temperature, Positions.Num(), Movements.Num());
}

void AGraphUntangling::RefreshUntangleableActors()
{
	UE_LOG(LogTemp, Log, TEXT("AGraphUntangling::RefreshUntangleableActors."));
	FindUntangleableActorsByTags();

	// Clear the existing ActorAdjacencyList
	ActorAdjacencyList.Empty();

	// Populate ActorAdjacencyList based on UntangleableAdjacencyList
	for (const TArray<TScriptInterface<IUntangleable>> &UntangleableInnerList : UntangleableAdjacencyList)
	{
		TArray<AActor *> ActorInnerList;
		for (const TScriptInterface<IUntangleable> &UntangleableItem : UntangleableInnerList)
		{
			if (UntangleableItem.GetObject())
			{
				AActor *ActorItem = Cast<AActor>(UntangleableItem.GetObject());
				if (ActorItem)
				{
					ActorInnerList.Add(ActorItem);
				}
				else
				{
					// Warn if the cast fails, though an IUntangleable should ideally be an AActor or derived from it
					UE_LOG(LogTemp, Warning, TEXT("RefreshUntangleableActors: UntangleableItem %s is not an AActor."), *UntangleableItem.GetObject()->GetName());
				}
			}
		}
		ActorAdjacencyList.Add(ActorInnerList);
	}

	FormatDebugUntangleableObjects();
}

void AGraphUntangling::FindUntangleableActorsByTags()
{
	// If TargetGraph is not set, clear the list and the last constructed graph reference.
	if (!TargetedGraph)
	{
		UE_LOG(LogTemp, Log,
			   TEXT("AGraphUntangling::FindUntangleableActorsByTags: TargetGraph is null. Clearing UntangleableObjects."));
		UntangleableAdjacencyList.Empty();
		return;
	}

	// Explicitly update the adjacency list before using it
	TargetedGraph->UpdateAdjacencyList();

	UntangleableAdjacencyList.Empty();
	const TArray<TArray<FGameplayTag>> &AdjacencyList = TargetedGraph->GetAdjacencyList();

	if (AdjacencyList.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			   TEXT(
				   "AGraphUntangling::FindUntangleableActorsByTags: TargetGraph %s has an empty adjacency list. Skipping."),
			   *TargetedGraph->GetName());
		return;
	}

	// Find all actors in the current level that implement the Untangleable interface
	TArray<AActor *> FoundActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UUntangleable::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			   TEXT(
				   "AGraphUntangling::FindUntangleableActorsByTags: No actors implementing Untangleable interface found in the level."));
		return;
	}

	bool bConstructionSuccessful = true; // Track if construction completes without missing actors
	UntangleableAdjacencyList.Reserve(AdjacencyList.Num());

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
				UE_LOG(LogTemp, Warning,
					   TEXT(
						   "AGraphUntangling::FindUntangleableActorsByTags: Encountered invalid tag in TargetGraph's adjacency list. Skipping."));
				InnerArray.Add(nullptr); // Add null to maintain array structure
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
						   TEXT("Actor %s was returned by GetAllActorsWithInterface but doesn't implement Untangleable"), *Actor->GetName());
				}
			} // End loop through FoundActors

			if (!bFoundMatchingActor)
			{
				UE_LOG(LogTemp, Warning,
					   TEXT(
						   "AGraphUntangling::FindUntangleableActorsByTags: Could not find any actor matching required tags: Primary=%s, Secondary=%s"),
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
				   "AGraphUntangling::FindUntangleableActorsByTags: Successfully constructed UntangleableObjects with %d nodes for graph %s."),
			   UntangleableAdjacencyList.Num(), *TargetedGraph->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			   TEXT(
				   "AGraphUntangling::FindUntangleableActorsByTags: Finished constructing UntangleableObjects for graph %s, but some actors were missing."),
			   *TargetedGraph->GetName());
	}
}

void AGraphUntangling::FormatDebugUntangleableObjects()
{
	DebugAdjacencyList.Empty(); // Clear the debug string

	for (const TArray<AActor *> &NodeConnections : ActorAdjacencyList)
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

void AGraphUntangling::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
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

void AGraphUntangling::DoStep()
{
	// Gather current positions
	for (int32 i = 0; i < NumNodes; ++i)
	{
		AActor *NodeActor = (ActorAdjacencyList[i].Num() > 0) ? ActorAdjacencyList[i][0] : nullptr;
		if (NodeActor)
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
			FVector Delta = Positions[v] - Positions[u];
			float Dist = Delta.Size();
			if (Dist < KINDA_SMALL_NUMBER)
				continue;
			if (Dist > 1000.f)
				continue;

			float Repulsion = KSquared / Dist;
			FVector Dir = Delta / Dist;
			Movements[v] += Dir * Repulsion;
			Movements[u] -= Dir * Repulsion;

			// Debug print for repulsion
			AActor *ActorV = (ActorAdjacencyList[v].Num() > 0) ? ActorAdjacencyList[v][0] : nullptr;
			AActor *ActorU = (ActorAdjacencyList[u].Num() > 0) ? ActorAdjacencyList[u][0] : nullptr;
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
		const TArray<AActor *> &NodeList = ActorAdjacencyList[v];
		AActor *NodeActor = (NodeList.Num() > 0) ? NodeList[0] : nullptr;
		if (!NodeActor)
			continue;

		for (int32 n = 1; n < NodeList.Num(); ++n)
		{
			AActor *NeighborActor = NodeList[n];
			if (!NeighborActor)
				continue;

			// Find index of neighbor in ActorAdjacencyList
			int32 NeighborIdx = INDEX_NONE;
			for (int32 idx = 0; idx < NumNodes; ++idx)
			{
				if (ActorAdjacencyList[idx].Num() > 0 && ActorAdjacencyList[idx][0] == NeighborActor)
				{
					NeighborIdx = idx;
					break;
				}
			}
			if (NeighborIdx == INDEX_NONE || NeighborIdx == v)
				continue;
			if (NeighborIdx > v)
				continue; // Only process each edge once

			FVector Delta = Positions[v] - Positions[NeighborIdx];
			float Dist = Delta.Size();
			if (Dist < KINDA_SMALL_NUMBER)
				continue;

			float Attraction = (Dist * Dist) / KConstant;
			FVector Dir = Delta / Dist;
			Movements[v] -= Dir * Attraction;
			Movements[NeighborIdx] += Dir * Attraction;

			// Debug print for attraction
			AActor *ActorV = NodeActor;
			AActor *ActorN = NeighborActor;
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
		float MoveNorm = Movements[v].Size();
		if (MoveNorm < 1.f)
			continue;
		float CappedNorm = FMath::Min(MoveNorm, Temperature);
		FVector CappedMove = Movements[v] / MoveNorm * CappedNorm;

		AActor *NodeActor = (ActorAdjacencyList[v].Num() > 0) ? ActorAdjacencyList[v][0] : nullptr;
		if (NodeActor)
		{
			NodeActor->SetActorLocation(NodeActor->GetActorLocation() + CappedMove);

			// Debug print for movement
			FString Msg = FString::Printf(TEXT("Move: %s | Δ: (%.2f, %.2f, %.2f) | Norm: %.2f"),
										  *NodeActor->GetName(), CappedMove.X, CappedMove.Y, CappedMove.Z, CappedNorm);
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, Msg);
		}
	}

	// // Cool down
	// if (Temperature > 1.5f)
	// {
	// 	Temperature *= 0.85f;
	// }
	// else
	// {
	// 	Temperature = 1.5f;
	// }
}

void AGraphUntangling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoStep();
}
