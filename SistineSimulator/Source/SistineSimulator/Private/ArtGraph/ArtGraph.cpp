// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/ArtGraph.h"

TArray<TPair<FGameplayTag, FGameplayTag>> UGraphElement::GetEdgesAsTagPairs() const
{
	TArray<TPair<FGameplayTag, FGameplayTag>> TagPairs;

	for (const FGraphEdge &Edge : Edges)
	{
		if (Edge.ElementA && Edge.ElementB)
		{
			TagPairs.Add(TPair<FGameplayTag, FGameplayTag>(Edge.ElementA->Tag, Edge.ElementB->Tag));
		}
	}

	return TagPairs;
}

TArray<TArray<FGameplayTag>> UGraphElement::GetAdjacencyList() const
{
	TMap<FGameplayTag, TSet<FGameplayTag>> AdjacencyMap;

	for (const FGraphEdge& Edge : Edges)
	{
		if (Edge.ElementA && Edge.ElementB)
		{
			// Add connections in both directions
			AdjacencyMap.FindOrAdd(Edge.ElementA->Tag).Add(Edge.ElementB->Tag);
			AdjacencyMap.FindOrAdd(Edge.ElementB->Tag).Add(Edge.ElementA->Tag);
		}
	}

	// Convert the adjacency map to an array of arrays
	TArray<TArray<FGameplayTag>> AdjacencyList;
	for (const auto& Pair : AdjacencyMap)
	{
		TArray<FGameplayTag> NodeConnections;
		NodeConnections.Add(Pair.Key); // Add the node itself
		NodeConnections.Append(Pair.Value.Array()); // Add all connected nodes
		AdjacencyList.Add(NodeConnections);
	}

	return AdjacencyList;
}
