// Fill out your copyright notice in the Description page of Project Settings.

#include "VertexComponentDev.h"

// Sets default values for this component's properties
UVertexComponentDev::UVertexComponentDev()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UVertexComponentDev::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, TEXT("FD VertexComponentDev BeginPlay"));
	}

	// ...
}

// Called every frame
void UVertexComponentDev::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
