// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/GraphUntangling.h"

// Sets default values
AGraphUntangling::AGraphUntangling()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGraphUntangling::BeginPlay()
{
	Super::BeginPlay();

	if (TargetGraph)
	{
		// TArray<TArray<FGameplayTag>> AdjacencyList = TargetGraph->GetAdjacencyList();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetGraph is null for $s"), *GetName());
	}
}

// Called every frame
void AGraphUntangling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
