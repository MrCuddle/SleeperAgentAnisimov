// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "PatrolNode.h"


// Sets default values
APatrolNode::APatrolNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APatrolNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APatrolNode::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

