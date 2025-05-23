#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Untangleable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UUntangleable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can have their position managed by the UntanglingLayout.
 */
class SISTINESIMULATOR_API IUntangleable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Returns the Gameplay Tags associated with this object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Untangleable")
	FGameplayTagContainer GetTags() const; // Changed from GetTag to GetTags
};
