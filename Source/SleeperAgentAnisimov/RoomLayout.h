// Fill out your copyright notice in the Description page of Project Settings.

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
	TArray<FVector2D> floorLocations;

};
