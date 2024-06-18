// Fill out your copyright notice in the Description page of Project Settings.


#include "Obstacles.h"

// Sets default values
AObstacles::AObstacles()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
		
	ScoreCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("ScoreCollider"));
	ScoreCollider->SetupAttachment(StaticMeshComponent);
	ScoreCollider->SetVisibility(true);

}

// Called when the game starts or when spawned
void AObstacles::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AObstacles::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

