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
	void RegisterGraph(UGraphElement *Graph);

	// Unregister a graph from the subsystem
	void UnregisterGraph(UGraphElement *Graph);

	// Notify graphs that reference the given element to update their caches
	void NotifyElementChanged(UGraphElement *ChangedElement);

	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

private:
	// Map of elements to the graphs that reference them
	TMap<UGraphElement *, TSet<UGraphElement *>> ElementToGraphsMap;
};
