// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseDoor.generated.h"

class ABaseRoomActor;

UCLASS()
class SLEEPERAGENTANISIMOV_API ABaseDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseDoor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	//The room on one side of the door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	ABaseRoomActor* roomA;

	//The room on the other side of the door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	ABaseRoomActor* roomB;

	//Locked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool NeedsKeycard;
	
};
