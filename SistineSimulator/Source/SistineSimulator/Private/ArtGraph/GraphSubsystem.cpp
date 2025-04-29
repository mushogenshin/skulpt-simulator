#include "ArtGraph/GraphSubsystem.h"
#include "ArtGraph/ArtGraph.h"

void UGraphSubsystem::RegisterGraph(UGraphElement *Graph)
{
	if (!Graph)
		return;

	for (UGraphElement *Element : Graph->GetReferencedElements())
	{
		ElementToGraphsMap.FindOrAdd(Element).Add(Graph);
	}
}

void UGraphSubsystem::UnregisterGraph(UGraphElement *Graph)
{
	if (!Graph)
		return;

	for (UGraphElement *Element : Graph->GetReferencedElements())
	{
		if (TSet<UGraphElement *> *Graphs = ElementToGraphsMap.Find(Element))
		{
			Graphs->Remove(Graph);
			if (Graphs->Num() == 0)
			{
				ElementToGraphsMap.Remove(Element);
			}
		}
	}
}

void UGraphSubsystem::NotifyElementChanged(UGraphElement *ChangedElement)
{
	if (!ChangedElement)
		return;

	if (TSet<UGraphElement *> *Graphs = ElementToGraphsMap.Find(ChangedElement))
	{
		for (UGraphElement *Graph : *Graphs)
		{
			if (Graph)
			{
				Graph->UpdateAdjacencyList();
			}
		}
	}
}
