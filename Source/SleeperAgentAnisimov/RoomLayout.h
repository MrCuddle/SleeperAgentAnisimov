// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseRoomActor.h"


#pragma once

/**
 * 
 */
class SLEEPERAGENTANISIMOV_API RoomLayout
{
public:
	RoomLayout();
	~RoomLayout();

	bool northDoor;
	bool eastDoor;
	bool southDoor;
	bool westDoor;
	int rarity;
	int type;
	TArray<FVector2D> floorLocations;
	TArray<FVector2D> hWallLocations;
	TArray<FVector2D> vWallLocations;
	TArray<FVector2D> itemLocations;
	TArray<FGuardStruct> guards;
	TArray<FPatrolRouteStruct> patrolRoutes;
	int testtest;

};
