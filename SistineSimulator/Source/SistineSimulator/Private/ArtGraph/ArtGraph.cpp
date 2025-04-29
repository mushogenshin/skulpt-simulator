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
