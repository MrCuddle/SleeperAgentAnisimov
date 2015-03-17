// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseDoor.h"
#include "BaseRoomActor.generated.h"


USTRUCT(blueprintable)
struct FGuardStruct{

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	FVector2D spawnLocation;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	int32 patrolRouteIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	int32 startIndex;

	FGuardStruct(){
		patrolRouteIndex = -1;
		startIndex = 0;
		spawnLocation = FVector2D();
	}

};

USTRUCT(blueprintable)
struct FPatrolRouteStruct{

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	TArray<FVector2D> patrolPoints;


	FPatrolRouteStruct(){
		patrolPoints = TArray<FVector2D>();
	}

};

USTRUCT(blueprintable)
struct FStaticMeshStruct{

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	FVector2D location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	float rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	FString name;


	FStaticMeshStruct(){
		location = FVector2D();
		rotation = 0.0;
		name = "";
	}

};

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API ABaseRoomActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool NorthDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool EastDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool SouthDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool WestDoor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool NorthDoorPossible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool EastDoorPossible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool SouthDoorPossible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool WestDoorPossible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	bool Ceiling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	TArray<FStaticMeshStruct> StaticMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	TArray<FVector2D> ItemLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	TArray<FGuardStruct> Guards;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	TArray<FPatrolRouteStruct> PatrolRoutes;


	//The north door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	ABaseDoor* northDoorActor;

	//The east door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	ABaseDoor* eastDoorActor;

	UFUNCTION(BlueprintImplementableEvent, Category = "Level Generation")
	virtual void GenerateRoom();
	
	
};
