// Fill out your copyright notice in the Description page of Project Settings.

#include "ArtGraph/UntanglingLayout.h"

// Sets default values
AUntanglingLayout::AUntanglingLayout()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUntanglingLayout::BeginPlay()
{
	Super::BeginPlay();

	if (TargetGraph)
	{
		TArray<TArray<FGameplayTag>> AdjacencyList = TargetGraph->GetAdjacencyList();
		for (const TArray<FGameplayTag>& NodeConnections : AdjacencyList)
		{
			if (NodeConnections.Num() > 0)
			{
				FString LogMessage = FString::Printf(TEXT("Node: %s -> "), *NodeConnections[0].ToString());
				for (int32 i = 1; i < NodeConnections.Num(); ++i)
				{
					LogMessage += NodeConnections[i].ToString() + TEXT(", ");
				}
				UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
				// E.g.
				// LogTemp: Node: Sistine.Sibyl.Delphic -> Sistine.Prophet.Isaah, Sistine.Prophet.Zacheria, 
				// LogTemp: Node: Sistine.Prophet.Isaah -> Sistine.Sibyl.Delphic, 
				// LogTemp: Node: Sistine.Prophet.Zacheria -> Sistine.Sibyl.Delphic, 
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetGraph is null in AUntanglingLayout::BeginPlay"));
	}
}

// Called every frame
void AUntanglingLayout::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

