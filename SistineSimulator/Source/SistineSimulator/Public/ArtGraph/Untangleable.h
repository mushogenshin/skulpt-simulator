#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h" // Include for FGameplayTag
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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Untangleable")
	FVector GetLocation() const;

	/** Sets the world location of the object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Untangleable")
	void SetLocation(const FVector& NewLocation);

	/** Returns the Gameplay Tag associated with this object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Untangleable")
	FGameplayTag GetTag() const;

	/** Returns the name of the object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Untangleable")
	FString GetName() const;
};

