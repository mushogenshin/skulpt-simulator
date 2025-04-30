#include "ArtGraph/ArtGraphSubsystem.h"
#include "ArtGraph/ArtGraph.h"

void UArtGraphSubsystem::RegisterGraph(UGraphElement *Graph)
{
	UE_LOG(LogEngine, Display, TEXT("Registering ArtGraph %s"), *Graph->GetName());

	if (!Graph)
		return;

	for (UGraphElement *Element : Graph->GetReferencedElements())
	{
		// BUG: this leaves phantom elements in the map
		ElementToGraphsMap.FindOrAdd(Element).Add(Graph);
	}

	// Log the current content of ElementToGraphsMap
	UE_LOG(LogEngine, Display, TEXT("Current ElementToGraphsMap content:"));
	for (const auto &Pair : ElementToGraphsMap)
	{
		FString ElementName = Pair.Key ? Pair.Key->GetName() : TEXT("Unknown");
		FString GraphNames;
		for (const UGraphElement *Graph : Pair.Value)
		{
			GraphNames += Graph ? Graph->GetName() + TEXT(", ") : TEXT("Unknown, ");
		}
		UE_LOG(LogEngine, Display, TEXT("Element: %s -> Graphs: %s"), *ElementName, *GraphNames);
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
				UE_LOG(LogEngine, Display, TEXT("Graph %s updated due to change in element %s"), *Graph->GetName(), *ChangedElement->GetName());
				Graph->UpdateAdjacencyList();
			}
		}
	}
}

void UArtGraphSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// Log a message to confirm the subsystem is initialized
	UE_LOG(LogEngine, Display, TEXT("ArtGraphSubsystem initialized successfully."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("GraphSubsystem initialized successfully."));
	}
}
