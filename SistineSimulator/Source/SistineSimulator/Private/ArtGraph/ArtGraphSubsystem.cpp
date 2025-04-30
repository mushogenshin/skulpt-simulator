#include "ArtGraph/ArtGraphSubsystem.h"
#include "ArtGraph/ArtGraph.h"

void UArtGraphSubsystem::RegisterGraph(UGraphElement *Graph)
{
	if (!Graph)
		return;

	for (UGraphElement *Element : Graph->GetReferencedElements())
	{
		ElementToGraphsMap.FindOrAdd(Element).Add(Graph);
	}
}

void UArtGraphSubsystem::UnregisterGraph(UGraphElement *Graph)
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

void UArtGraphSubsystem::NotifyElementChanged(UGraphElement *ChangedElement)
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

void UArtGraphSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// Log a message to confirm the subsystem is initialized
	UE_LOG(LogTemp, Warning, TEXT("GraphSubsystem initialized successfully."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("GraphSubsystem initialized successfully."));
	}
}
