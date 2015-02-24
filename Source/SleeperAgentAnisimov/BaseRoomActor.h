// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseRoomActor.generated.h"

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API ABaseRoomActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoorLocations)
	bool NorthDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoorLocations)
	bool EastDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoorLocations)
	bool SouthDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DoorLocations)
	bool WestDoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloorLocations)
	TArray<FVector2D> FloorLocations;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Level Generation")
	virtual void GenerateRoom();
	
	
};
