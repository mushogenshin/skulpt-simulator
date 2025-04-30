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
	TMap<FGameplayTag, TSet<FGameplayTag>> AdjacencyMap;

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
	DebugAdjacencyList.Empty(); // Clear the debug string
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

		// Update the debug string
		DebugAdjacencyList += FString::Printf(TEXT("%s -> ["), *Key.ToString());
		for (int32 i = 0; i < Neighbors.Num(); ++i)
		{
			DebugAdjacencyList += Neighbors[i].ToString();
			if (i < Neighbors.Num() - 1)
			{
				DebugAdjacencyList += TEXT(", ");
			}
			DebugAdjacencyList += TEXT("]\n");
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
