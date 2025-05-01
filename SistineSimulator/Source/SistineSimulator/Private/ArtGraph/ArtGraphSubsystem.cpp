#include "ArtGraph/ArtGraphSubsystem.h"
#include "ArtGraph/ArtGraph.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void UArtGraphSubsystem::RegisterGraph(UGraphElement *Graph, bool bClearPreviousReferences)
{
	if (!Graph)
		return;

	// Optionally unregister the graph from all previously referenced elements
	if (bClearPreviousReferences)
	{
		UnregisterGraph(Graph);
	}

	UE_LOG(LogEngine, Display, TEXT("Registering ArtGraph %s"), *Graph->GetName());
	for (UGraphElement *Element : Graph->GetReferencedElements())
	{
		if (Element) // Ensure the element is valid
		{
			ElementToDependentGraphsMap.FindOrAdd(Element).Add(Graph);
		}
	}
}

void UArtGraphSubsystem::UnregisterGraph(UGraphElement *Graph)
{
	if (!Graph)
		return;

	UE_LOG(LogEngine, Display, TEXT("Unregistering ArtGraph %s"), *Graph->GetName());

	// Iterate through the entire map to find and remove the graph
	for (auto It = ElementToDependentGraphsMap.CreateIterator(); It; ++It)
	{
		UGraphElement *Element = It.Key();
		TSet<UGraphElement *> &Graphs = It.Value();

		if (Graphs.Contains(Graph))
		{
			Graphs.Remove(Graph);

			// If the set becomes empty after removal, remove the element entry from the map
			if (Graphs.Num() == 0)
			{
				It.RemoveCurrent();
			}
		}
	}
}

void UArtGraphSubsystem::NotifyElementChanged(UGraphElement *ChangedElement)
{
	if (!ChangedElement)
		return;

	if (TSet<UGraphElement *> *Graphs = ElementToDependentGraphsMap.Find(ChangedElement))
	{
		for (UGraphElement *Graph : *Graphs)
		{
			if (Graph)
			{
				// UE_LOG(LogEngine, Display, TEXT("ArtGraph %s updated due to change in element %s"), *Graph->GetName(), *ChangedElement->GetName());
				Graph->UpdateAdjacencyList();
			}
		}
	}
}

void UArtGraphSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// Log a message to confirm the subsystem is initialized
	UE_LOG(LogEngine, Display, TEXT("ArtGraphSubsystem initializing..."));

	// Scan for all UGraphElement assets and register them
	FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetData;
	// Use the correct class name here - UGraphElement
	AssetRegistryModule.Get().GetAssetsByClass(UGraphElement::StaticClass()->GetClassPathName(), AssetData);

	UE_LOG(LogEngine, Display, TEXT("Found %d UGraphElement assets."), AssetData.Num());

	for (const FAssetData &Data : AssetData)
	{
		UGraphElement *GraphElement = Cast<UGraphElement>(Data.GetAsset());
		if (GraphElement)
		{
			// Ensure the asset is fully loaded before accessing properties like ReferencedElements
			GraphElement->ConditionalPostLoad();
			GraphElement->UpdateAdjacencyList(); // Update the adjacency list for the graph element
			// Register without clearing previous references during initialization,
			// as the map is empty initially.
			RegisterGraph(GraphElement, false);
		}
		else
		{
			UE_LOG(LogEngine, Warning, TEXT("Failed to load or cast asset: %s"), *Data.GetObjectPathString());
		}
	}

	UE_LOG(LogEngine, Display, TEXT("ArtGraphSubsystem initialized successfully."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("ArtGraphSubsystem initialized successfully."));
	}
}
