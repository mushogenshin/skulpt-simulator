// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/ArtGraph.h"
#include "ArtGraph/ArtGraphSubsystem.h"

TArray<TArray<FGameplayTag>> UGraphElement::GetAdjacencyList() const
{
	return CachedAdjacencyList;
}

void UGraphElement::UpdateAdjacencyList()
{
	UE_LOG(LogTemp, Display, TEXT("Updating adjacency list for graph %s"), *GetName());
	CalculateAdjacencyList();
	FormatDebugAdjacencyList();
}

void UGraphElement::CalculateAdjacencyList()
{
	TMap<FGameplayTag, TSet<FGameplayTag>> AdjacencyMap;

	// // Add self to the map to ensure it appears even if it has no edges
	// if (Tag.IsValid())
	// {
	// 	AdjacencyMap.FindOrAdd(Tag);
	// }

	for (const FGraphEdge &Edge : Edges)
	{
		if (Edge.ElementA && Edge.ElementA->Tag.IsValid() && Edge.ElementB && Edge.ElementB->Tag.IsValid())
		{
			// Add connections in both directions
			AdjacencyMap.FindOrAdd(Edge.ElementA->Tag).Add(Edge.ElementB->Tag);
			AdjacencyMap.FindOrAdd(Edge.ElementB->Tag).Add(Edge.ElementA->Tag);
		}
	}

	// Convert the adjacency map to an array of arrays
	CachedAdjacencyList.Empty();
	TArray<FGameplayTag> MapKeys;
	AdjacencyMap.GetKeys(MapKeys);
	MapKeys.Sort([](const FGameplayTag &A, const FGameplayTag &B) { // Sort for consistent debug output
		return A.ToString() < B.ToString();
	});

	for (const FGameplayTag &Key : MapKeys)
	{
		TArray<FGameplayTag> NodeConnections;
		NodeConnections.Add(Key); // Add the node itself
		TArray<FGameplayTag> Neighbors = AdjacencyMap[Key].Array();
		Neighbors.Sort([](const FGameplayTag &A, const FGameplayTag &B) { // Sort neighbors for consistent debug output
			return A.ToString() < B.ToString();
		});
		NodeConnections.Append(Neighbors); // Add all connected nodes
		CachedAdjacencyList.Add(NodeConnections);
	}
}

void UGraphElement::FormatDebugAdjacencyList()
{
	DebugAdjacencyList.Empty(); // Clear the debug string

	for (const TArray<FGameplayTag> &NodeConnections : CachedAdjacencyList)
	{
		if (NodeConnections.Num() > 0)
		{
			const FGameplayTag &Key = NodeConnections[0];
			// The first element is the node itself
			DebugAdjacencyList += FString::Printf(TEXT("%s -> ["), *Key.ToString());

			// Start from index 1 to get neighbors
			for (int32 i = 1; i < NodeConnections.Num(); ++i)
			{
				DebugAdjacencyList += NodeConnections[i].ToString();
				if (i < NodeConnections.Num() - 1)
				{
					DebugAdjacencyList += TEXT(", ");
				}
			}
			DebugAdjacencyList += TEXT("]");
			if (&NodeConnections != &CachedAdjacencyList.Last())
			{
				DebugAdjacencyList += TEXT("\n\n");
			}
		}
	}
}

TArray<UGraphElement *> UGraphElement::GetReferencedElements() const
{
	TArray<UGraphElement *> ReferencedElements;

	for (const FGraphEdge &Edge : Edges)
	{
		if (Edge.ElementA)
		{
			ReferencedElements.AddUnique(Edge.ElementA);
		}
		if (Edge.ElementB)
		{
			ReferencedElements.AddUnique(Edge.ElementB);
		}
	}

	return ReferencedElements;
}

void UGraphElement::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Update the cached adjacency list
	UpdateAdjacencyList();

	if (UArtGraphSubsystem *Subsystem = GEngine->GetEngineSubsystem<UArtGraphSubsystem>())
	{
		// Register this graph and all of its referenced graphs with the subsystem.
		// Pass true to clear previous references since the graph's references might have changed.
		Subsystem->RegisterGraph(this, true);

		// Notify the subsystem that this element might have changed,
		// affecting graphs that reference it.
		Subsystem->NotifyElementChanged(this);
	}
}
