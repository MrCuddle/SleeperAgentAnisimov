// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PatrolNode.generated.h"

UCLASS()
class SLEEPERAGENTANISIMOV_API APatrolNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolNode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    APatrolNode* nextNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    APatrolNode* prevNode;
};
