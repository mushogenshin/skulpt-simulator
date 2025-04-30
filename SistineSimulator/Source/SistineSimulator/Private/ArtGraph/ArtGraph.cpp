// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/ArtGraph.h"
#include "ArtGraph/ArtGraphSubsystem.h"

// TArray<TPair<FGameplayTag, FGameplayTag>> UGraphElement::GetEdgesAsTagPairs() const
// {
// 	TArray<TPair<FGameplayTag, FGameplayTag>> TagPairs;

// 	for (const FGraphEdge &Edge : Edges)
// 	{
// 		if (Edge.ElementA && Edge.ElementB)
// 		{
// 			TagPairs.Add(TPair<FGameplayTag, FGameplayTag>(Edge.ElementA->Tag, Edge.ElementB->Tag));
// 		}
// 	}

// 	return TagPairs;
// }

TArray<TArray<FGameplayTag>> UGraphElement::GetAdjacencyList() const
{
	return CachedAdjacencyList;
}

void UGraphElement::UpdateAdjacencyList()
{
	TMap<FGameplayTag, TSet<FGameplayTag>> AdjacencyMap;

	for (const FGraphEdge &Edge : Edges)
	{
		if (Edge.ElementA && Edge.ElementB)
		{
			// Add connections in both directions
			AdjacencyMap.FindOrAdd(Edge.ElementA->Tag).Add(Edge.ElementB->Tag);
			AdjacencyMap.FindOrAdd(Edge.ElementB->Tag).Add(Edge.ElementA->Tag);
		}
	}

	// Convert the adjacency map to an array of arrays
	CachedAdjacencyList.Empty();
	for (const auto &Pair : AdjacencyMap)
	{
		TArray<FGameplayTag> NodeConnections;
		NodeConnections.Add(Pair.Key);				// Add the node itself
		NodeConnections.Append(Pair.Value.Array()); // Add all connected nodes
		CachedAdjacencyList.Add(NodeConnections);
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
		// Register this graph and all of its referenced graphs with the subsystem
		Subsystem->RegisterGraph(this);

		// Notify the subsystem
		Subsystem->NotifyElementChanged(this);
	}
}
