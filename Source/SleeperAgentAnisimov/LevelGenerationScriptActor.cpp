// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "LevelGenerationScriptActor.h"
#include "BaseRoomActor.h"
#include "RoomLayout.h"
#include <cstdlib>
#include <iterator>
#include "DefaultValueHelper.h"
#include <assert.h>
#include <algorithm>

class Visitor : public IPlatformFile::FDirectoryVisitor{
public:
	std::vector<FString> files;
	Visitor(){}
	bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory){
		files.push_back(FString(FilenameOrDirectory));
		return true;
	}
};

ALevelGenerationScriptActor::ALevelGenerationScriptActor(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){
	srand(time(NULL));
	static ConstructorHelpers::FObjectFinder<UBlueprint> BlueprintFinder(TEXT("/Game/Blueprints/RoomLoader"));
	if (BlueprintFinder.Object){
		roomLoaderBlueprint = (UClass*)BlueprintFinder.Object->GeneratedClass;
	}

	LoadRoomLayouts();
}

void ALevelGenerationScriptActor::LoadRoomLayouts(){
	
	//Iterate through the RoomLayouts directory loading all files
	FString layoutsPath = FPaths::GameDir() + "Content/RoomLayouts/";
	Visitor* v = new Visitor();
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(layoutsPath.GetCharArray().GetData(), *v);
	for (int i = 0; i < v->files.size(); i++){
		LoadRoomLayout(v->files[i]);
	}

	//Sets must be sorted for set_intersection to work properly!
	sort(northRooms.begin(), northRooms.end());
	sort(eastRooms.begin(), eastRooms.end());
	sort(southRooms.begin(), southRooms.end());
	sort(westRooms.begin(), westRooms.end());
	
}

void ALevelGenerationScriptActor::LoadRoomLayout(FString path){
	TArray<FString> strings;
	FFileHelper::LoadANSITextFileToStrings(*path, NULL, strings);

	//Read the locations of doors
	RoomLayout *roomLayout = new RoomLayout();
	if (strings[0][0] == '1') { roomLayout->northDoor = true; northRooms.push_back(roomLayout); }
	else { roomLayout->northDoor = false; }
	if (strings[0][1] == '1') { roomLayout->eastDoor = true; eastRooms.push_back(roomLayout); }
	else { roomLayout->eastDoor = false; }
	if (strings[0][2] == '1') { roomLayout->southDoor = true; southRooms.push_back(roomLayout); }
	else { roomLayout->southDoor = false; }
	if (strings[0][3] == '1') { roomLayout->westDoor = true; westRooms.push_back(roomLayout); }
	else { roomLayout->westDoor = false; }

	//Read the locations of floor tiles
	TArray<FVector2D> floor;

	TArray<FString> floorStrings;
	strings[1].ParseIntoArrayWS(&floorStrings);
	
	for (int i = 0; i < floorStrings.Num(); i++){
		FString sX, sY;
		floorStrings[i].Split(FString(","), &sX, &sY);
		int x = 0, y = 0;
		FDefaultValueHelper::ParseInt(sX, x);
		FDefaultValueHelper::ParseInt(sY, y);
		floor.Add(FVector2D(x, y));
	}

	roomLayout->floorLocations = floor;

	//Read the locations of horizontal walls
	TArray<FVector2D> horizontalWalls;

	TArray<FString> hWallStrings;
	strings[2].ParseIntoArrayWS(&hWallStrings);

	for (int i = 0; i < hWallStrings.Num(); i++){
		FString sX, sY;
		hWallStrings[i].Split(FString(","), &sX, &sY);
		int x = 0, y = 0;
		FDefaultValueHelper::ParseInt(sX, x);
		FDefaultValueHelper::ParseInt(sY, y);
		horizontalWalls.Add(FVector2D(x, y));
	}

	roomLayout->hWallLocations = horizontalWalls;

	//Read the locations of vertical walls
	TArray<FVector2D> verticalWalls;

	TArray<FString> vWallStrings;
	strings[3].ParseIntoArrayWS(&vWallStrings);

	for (int i = 0; i < vWallStrings.Num(); i++){
		FString sX, sY;
		vWallStrings[i].Split(FString(","), &sX, &sY);
		int x = 0, y = 0;
		FDefaultValueHelper::ParseInt(sX, x);
		FDefaultValueHelper::ParseInt(sY, y);
		verticalWalls.Add(FVector2D(x, y));
	}

	roomLayout->vWallLocations = verticalWalls;
	//Read the locations of everything else
	//TODO
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
					std::vector<RoomLayout*> outputSet;
					if (j < 8 && layout[i][j + 1] == 1)
					{
						if (outputSet.empty())
						{
							outputSet = southRooms;
						}
					}
					if (i < 8 && layout[i + 1][j] == 1)
					{
						if (outputSet.empty())
						{
							outputSet = eastRooms;
						}
						else
						{
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), eastRooms.begin(), eastRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}
					if (i > 0 && layout[i - 1][j] == 1)
					{
						if (outputSet.empty())
						{
							outputSet = westRooms;
						}
						else
						{
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), westRooms.begin(), westRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}
					if (j > 0 && layout[i][j - 1] == 1)
					{
						if (outputSet.empty())
						{
							outputSet = northRooms;
						}
						else
						{
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), northRooms.begin(), northRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;

						}
					}

					RoomLayout* roomLayout = outputSet[rand() % outputSet.size()];

					//int index = rand() % outputSet.size();
					//room = (ABaseRoomActor*)world->SpawnActor<AActor>(outputSet[index], FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));

					ABaseRoomActor* room = (ABaseRoomActor*)world->SpawnActor<AActor>(roomLoaderBlueprint, FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));
					

					if (room){
						if (j < 8 && layout[i][j + 1] == 1)
							room->SouthDoor = true;
						if (i < 8 && layout[i + 1][j] == 1)
							room->EastDoor = true;
						if (i > 0 && layout[i - 1][j] == 1)
							room->WestDoor = true;
						if (j > 0 && layout[i][j - 1] == 1)
							room->NorthDoor = true;

						room->NorthDoorPossible = roomLayout->northDoor;
						room->EastDoorPossible = roomLayout->eastDoor;
						room->SouthDoorPossible = roomLayout->southDoor;
						room->WestDoorPossible = roomLayout->westDoor;

						room->FloorLocations = roomLayout->floorLocations;
						room->HWallLocations = roomLayout->hWallLocations;
						room->VWallLocations = roomLayout->vWallLocations;

						room->GenerateRoom();
					}
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
