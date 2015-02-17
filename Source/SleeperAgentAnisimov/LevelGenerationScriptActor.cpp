// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "LevelGenerationScriptActor.h"
#include "BaseRoomActor.h"
#include <cstdlib>

ALevelGenerationScriptActor::ALevelGenerationScriptActor(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){
	srand(time(NULL));
	static ConstructorHelpers::FObjectFinder<UBlueprint> ItemBlueprint(TEXT("/Game/Blueprints/RoomPrototype1"));
	if (ItemBlueprint.Object){
		RoomBlueprint = (UClass*)ItemBlueprint.Object->GeneratedClass;
	}
}

void ALevelGenerationScriptActor::GenerateLevel(){
	nRooms = 16;
	UWorld* const world = GetWorld();
	if (world){
		while (nRooms > 0){
			rooms.push(std::pair<int, int>(4, 4));
			while (!rooms.empty()){
				ExploreLevel();
			}
		}

		//Spawn the rooms
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (layout[i][j] == 1)
				{
					ABaseRoomActor* room = (ABaseRoomActor*)world->SpawnActor<AActor>(RoomBlueprint, FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));
			
					if (j < 8 && layout[i][j + 1] == 1)
						room->SouthDoor = true;
					if(i < 8 && layout[i + 1][j] == 1)
						room->EastDoor = true;
					if (i > 0 && layout[i - 1][j] == 1)
						room->WestDoor = true;
					if (j > 0 && layout[i][j - 1] == 1)
						room->NorthDoor = true;
					room->SetDoors();
				}
			}
		}
	}
}

void ALevelGenerationScriptActor::ExploreLevel(){
	std::pair<int, int> current = rooms.front();
	rooms.pop();
	int i = current.first;
	int j = current.second;


	if (nRooms == 0) return;
	//Put a room at i,j if there isn't already one, and decrement the number of rooms remaining if necessary
	if (layout[i][j] == 0){
		nRooms--;
		layout[i][j] = 1;
		if (nRooms == 0)
			return;
	}

	//Chance to visit the remaining rooms...
	float chance;
	bool chosen = false;

	//North
	if (j > 0){
		chance = 0.5f;
		chance /= (float)GetAdjacentRooms(i, j - 1) * 2;
		chance += GetDistanceFromStart(i, j - 1) * 0.1f;
		if ((float)rand() / (float)RAND_MAX < chance){
			rooms.push(std::pair<int, int>(i, j - 1));
		}
	}
	
	//East
	if (i < 8){
		chance = 0.5f;
		chance /= (float)GetAdjacentRooms(i + 1, j) * 2;
		chance += GetDistanceFromStart(i + 1, j) * 0.1f;
		if ((float)rand() / (float)RAND_MAX < chance){
			rooms.push(std::pair<int, int>(i + 1, j));
		}
	}

	//South
	if (j < 8){
		chance = 0.5f;
		chance /= (float)GetAdjacentRooms(i, j + 1) * 2;
		chance += GetDistanceFromStart(i, j + 1) * 0.1f;
		if ((float)rand() / (float)RAND_MAX < chance){
			rooms.push(std::pair<int, int>(i, j + 1));
		}
	}

	//West
	if (i > 0){
		chance = 0.5f;
		chance /= (float)GetAdjacentRooms(i - 1, j) * 2;
		chance += GetDistanceFromStart(i - 1, j) * 0.1f;
		if ((float)rand() / (float)RAND_MAX < chance){
			rooms.push(std::pair<int, int>(i - 1, j));
		}
	}

}

int ALevelGenerationScriptActor::GetAdjacentRooms(int i, int j){
	int adj = 0;
	if (i > 0 && layout[i - 1][j] > 0) adj++;
	if (i < 8 && layout[i + 1][j] > 0) adj++;
	if (j > 0 && layout[i][j - 1] > 0) adj++;
	if (j < 8 && layout[i][j + 1] > 0) adj++;
	return adj;
}

int ALevelGenerationScriptActor::GetDistanceFromStart(int i, int j){
	return std::abs(i - 4) + std::abs(j - 4);
}


//void ALevelGenerationScriptActor::GenerateLevel()
//{
//	int nrOfRooms = 10;
//	UWorld* const World = GetWorld();
//	if (World){
//		//World->SpawnActor<AActor>(RoomBlueprint, FVector(0, 0, 0), FRotator(0, 0, 0));
//		layout[4][4] = 1;
//		std::queue<std::pair<int, int>> rooms;
//		rooms.push(std::pair<int, int >(4, 4));
//		while (nrOfRooms > 0)
//		{
//			if (rooms.empty())
//				break;
//			std::pair<int, int> current = rooms.front();
//			rooms.pop();
//
//			for (int i = 0; i < 4; i++)
//			{
//				if (nrOfRooms == 0)
//					break;
//				int placeRoom = rand() % 2;
//				if (placeRoom == 1)
//				{
//					switch (i)
//					{
//					case 0:
//						if (current.second > 0 && layout[current.first][current.second - 1] == 0)
//						{
//							layout[current.first][current.second - 1] = 1;
//							rooms.push(std::pair<int, int>(current.first, current.second - 1));
//							nrOfRooms--;
//						}
//						break;
//					case 1:
//						if (current.first < 8 && layout[current.first + 1][current.second] == 0)
//						{
//							layout[current.first + 1][current.second] = 1;
//							rooms.push(std::pair<int, int>(current.first + 1, current.second));
//							nrOfRooms--;
//						}
//						break;
//					case 2:
//						if (current.second < 8 && layout[current.first][current.second + 1] == 0)
//						{
//							layout[current.first][current.second + 1] = 1;
//							rooms.push(std::pair<int, int>(current.first, current.second + 1));
//							nrOfRooms--;
//						}
//						break;
//					case 3:
//						if (current.first > 0 && layout[current.first - 1][current.second] == 0)
//						{
//							layout[current.first - 1][current.second] = 1;
//							rooms.push(std::pair<int, int>(current.first - 1, current.second));
//							nrOfRooms--;
//						}
//						break;
//					}
//				}
//			}
//		}
//
//		for (int i = 0; i < 9; i++)
//		{
//			for (int j = 0; j < 9; j++)
//			{
//				if (layout[i][j] == 1)
//				{
//					ABaseRoomActor* room = (ABaseRoomActor*)World->SpawnActor<AActor>(RoomBlueprint, FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));
//
//					if (j < 8 && layout[i][j + 1] == 1)
//						room->SouthDoor = true;
//					if(i < 8 && layout[i + 1][j] == 1)
//						room->EastDoor = true;
//					if (i > 0 && layout[i - 1][j] == 1)
//						room->WestDoor = true;
//					if (j > 0 && layout[i][j - 1] == 1)
//						room->NorthDoor = true;
//					room->SetDoors();
//				}
//			}
//		}
//	}
//}

