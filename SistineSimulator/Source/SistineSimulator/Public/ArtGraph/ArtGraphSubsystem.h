#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h" // Changed from GameInstanceSubsystem to EngineSubsystem
#include "GameplayTagContainer.h"
#include "ArtGraphSubsystem.generated.h"

class UGraphElement;
class UArtGraph;

/**
 * A subsystem to manage and notify graphs when their elements change.
 */
UCLASS()
class SISTINESIMULATOR_API UArtGraphSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Register a graph with the subsystem
	// @param Graph The graph element to register.
	// @param bClearPreviousReferences If true, unregister the graph from any elements it previously referenced before registering new ones.
	void RegisterGraph(UGraphElement *Graph, bool bClearPreviousReferences = true);

	// Unregister a graph from the subsystem
	void UnregisterGraph(UGraphElement *Graph);

	// Notify graphs that reference the given element to update their caches
	void NotifyElementChanged(UGraphElement *ChangedElement);

	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

private:
	// Map of elements to the graphs that reference them
	TMap<UGraphElement *, TSet<UGraphElement *>> ElementToGraphsMap;
};
