#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
	/** Returns the current world location of the object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ArtGraph")
	FVector GetLocation() const;

	/** Sets the world location of the object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ArtGraph")
	void SetLocation(const FVector &NewLocation);
};
