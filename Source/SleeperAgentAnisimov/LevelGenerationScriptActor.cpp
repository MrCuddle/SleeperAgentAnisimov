// Fill out your copyright notice in the Description page of Project Settings.


#include "SleeperAgentAnisimov.h"
#include "JsonObject.h"
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
	delete v;

	//Sets must be sorted for set_intersection to work properly!
	sort(northRooms.begin(), northRooms.end());
	sort(eastRooms.begin(), eastRooms.end());
	sort(southRooms.begin(), southRooms.end());
	sort(westRooms.begin(), westRooms.end());
	
}

void ALevelGenerationScriptActor::LoadRoomLayout(FString path){
	//TArray<FString> strings;
	//FFileHelper::LoadANSITextFileToStrings(*path, NULL, strings);

	RoomLayout *roomLayout = new RoomLayout();

	FString text = FString();
	FFileHelper::LoadFileToString(text, path.GetCharArray().GetData());

	//std::string s = TCHAR_TO_UTF8(text.GetCharArray().GetData());

	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(text);
	int sc = 0;

	FJsonSerializer serializer = FJsonSerializer();
	if (serializer.Deserialize(JsonReader, JsonParsed)){
		roomLayout->northDoor = JsonParsed->GetBoolField("northDoor");
		roomLayout->eastDoor = JsonParsed->GetBoolField("eastDoor");
		roomLayout->southDoor = JsonParsed->GetBoolField("southDoor");
		roomLayout->westDoor = JsonParsed->GetBoolField("westDoor");
		roomLayout->rarity = (int)JsonParsed->GetNumberField("roomRarity");
		roomLayout->rarity = (int)JsonParsed->GetNumberField("roomType");

		TArray<TSharedPtr<FJsonValue>> spawnGroups = JsonParsed->GetArrayField("spawnGroups");
		for (int i = 0; i < spawnGroups.Num(); ++i){
			TArray<TSharedPtr<FJsonValue>> guards = spawnGroups[i]->AsObject()->GetArrayField("guards");
			for (int j = 0; j < guards.Num(); ++i){
				FGuardStruct g = FGuardStruct();
				g.patrolRouteIndex = guards[j]->AsObject()->GetNumberField("patrolRouteIndex");
				g.startIndex = (int)guards[j]->AsObject()->GetNumberField("startIndex");
				g.spawnLocation = FVector2D(guards[j]->AsObject()->GetNumberField("x"), guards[j]->AsObject()->GetNumberField("y"));
				roomLayout->guards.Add(g);
			}
			TArray<TSharedPtr<FJsonValue>> items = spawnGroups[i]->AsObject()->GetArrayField("items");
			for (int j = 0; j < items.Num(); ++i){
				roomLayout->itemLocations.Add(FVector2D(items[j]->AsObject()->GetNumberField("x"), items[j]->AsObject()->GetNumberField("y")));
			}
			//TArray<TSharedPtr<FJsonValue>> meshes = spawnGroups[i]->AsObject()->GetArrayField("meshes");
			//TArray<TSharedPtr<FJsonValue>> patrolPoints = spawnGroups[i]->AsObject()->GetArrayField("patrolPoints");
		}
	}

	////Read the locations of doors
	//RoomLayout *roomLayout = new RoomLayout();
	//if (strings[0][0] == '1') { roomLayout->northDoor = true; northRooms.push_back(roomLayout); }
	//else { roomLayout->northDoor = false; }
	//if (strings[0][1] == '1') { roomLayout->eastDoor = true; eastRooms.push_back(roomLayout); }
	//else { roomLayout->eastDoor = false; }
	//if (strings[0][2] == '1') { roomLayout->southDoor = true; southRooms.push_back(roomLayout); }
	//else { roomLayout->southDoor = false; }
	//if (strings[0][3] == '1') { roomLayout->westDoor = true; westRooms.push_back(roomLayout); }
	//else { roomLayout->westDoor = false; }

	////Read the locations of floor tiles
	//TArray<FVector2D> floor;

	//TArray<FString> floorStrings;
	//strings[1].ParseIntoArrayWS(&floorStrings);
	//
	//for (int i = 0; i < floorStrings.Num(); i++){
	//	FString sX, sY;
	//	floorStrings[i].Split(FString(","), &sX, &sY);
	//	int x = 0, y = 0;
	//	FDefaultValueHelper::ParseInt(sX, x);
	//	FDefaultValueHelper::ParseInt(sY, y);
	//	floor.Add(FVector2D(x, y));
	//}

	//roomLayout->floorLocations = floor;

	////Read the locations of horizontal walls
	//TArray<FVector2D> horizontalWalls;

	//TArray<FString> hWallStrings;
	//strings[2].ParseIntoArrayWS(&hWallStrings);

	//for (int i = 0; i < hWallStrings.Num(); i++){
	//	FString sX, sY;
	//	hWallStrings[i].Split(FString(","), &sX, &sY);
	//	int x = 0, y = 0;
	//	FDefaultValueHelper::ParseInt(sX, x);
	//	FDefaultValueHelper::ParseInt(sY, y);
	//	horizontalWalls.Add(FVector2D(x, y));
	//}

	//roomLayout->hWallLocations = horizontalWalls;

	////Read the locations of vertical walls
	//TArray<FVector2D> verticalWalls;

	//TArray<FString> vWallStrings;
	//strings[3].ParseIntoArrayWS(&vWallStrings);

	//for (int i = 0; i < vWallStrings.Num(); i++){
	//	FString sX, sY;
	//	vWallStrings[i].Split(FString(","), &sX, &sY);
	//	int x = 0, y = 0;
	//	FDefaultValueHelper::ParseInt(sX, x);
	//	FDefaultValueHelper::ParseInt(sY, y);
	//	verticalWalls.Add(FVector2D(x, y));
	//}

	//roomLayout->vWallLocations = verticalWalls;

	////Items:
	//TArray<FVector2D> items;

	//TArray<FString> itemStrings;
	//strings[4].ParseIntoArrayWS(&itemStrings);

	//for (int i = 0; i < itemStrings.Num(); i++){
	//	FString sX, sY;
	//	itemStrings[i].Split(FString(","), &sX, &sY);
	//	int x = 0, y = 0;
	//	FDefaultValueHelper::ParseInt(sX, x);
	//	FDefaultValueHelper::ParseInt(sY, y);
	//	items.Add(FVector2D(x, y));
	//}

	//roomLayout->itemLocations = items;

	////Guards:
	//TArray<FVector2D> guards;

	//TArray<FString> guardStrings;
	//strings[5].ParseIntoArrayWS(&guardStrings);

	//for (int i = 0; i < guardStrings.Num(); i++){
	//	FString sX, sY;
	//	guardStrings[i].Split(FString(","), &sX, &sY);
	//	int x = 0, y = 0;
	//	FDefaultValueHelper::ParseInt(sX, x);
	//	FDefaultValueHelper::ParseInt(sY, y);
	//	guards.Add(FVector2D(x, y));
	//}

	//roomLayout->guardLocations = guards;

	////Read the locations of everything else
	////TODO
}

void ALevelGenerationScriptActor::GenerateLevel(){
	nRooms = 16;
	UWorld* const world = GetWorld();
	if (world){

		PlanLayout();


		//Old layout generation
		//while (nRooms > 0){
		//	rooms.push(std::pair<int, int>(4, 4));
		//	while (!rooms.empty()){
		//		ExploreLevel();
		//	}
		//}

		//Spawn the rooms
		for (int i = 0; i < levelWidth; i++)
		{
			for (int j = 0; j < levelHeight; j++)
			{
				if (layout[i][j] > 0)
				{
					std::vector<RoomLayout*> outputSet;
					if (j < 8 && layout[i][j + 1] > 0)
					{
						if (outputSet.empty())
						{
							outputSet = southRooms;
						}
					}
					if (i < 8 && layout[i + 1][j] > 0)
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
					if (i > 0 && layout[i - 1][j] > 0)
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
					if (j > 0 && layout[i][j - 1] > 0)
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

					if (i == 4 && j == 4){
						if (roomLayout->northDoor){
							PlayerSpawn = FVector2D(600, 200);
						}
						else if (roomLayout->eastDoor){
							PlayerSpawn = FVector2D(1000, 600);
						}
						else if (roomLayout->southDoor){
							PlayerSpawn = FVector2D(600, 1000);
						}
						else {
							PlayerSpawn = FVector2D(200, 600);
						}
					}

					//int index = rand() % outputSet.size();
					//room = (ABaseRoomActor*)world->SpawnActor<AActor>(outputSet[index], FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));

					ABaseRoomActor* room = (ABaseRoomActor*)world->SpawnActor<AActor>(roomLoaderBlueprint, FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));
					

					if (room){
						if (j < 8 && layout[i][j + 1] > 0)
							room->SouthDoor = true;
						if (i < 8 && layout[i + 1][j] > 0)
							room->EastDoor = true;
						if (i > 0 && layout[i - 1][j] > 0)
							room->WestDoor = true;
						if (j > 0 && layout[i][j - 1] > 0)
							room->NorthDoor = true;

						room->NorthDoorPossible = roomLayout->northDoor;
						room->EastDoorPossible = roomLayout->eastDoor;
						room->SouthDoorPossible = roomLayout->southDoor;
						room->WestDoorPossible = roomLayout->westDoor;

						room->FloorLocations = roomLayout->floorLocations;
						room->HWallLocations = roomLayout->hWallLocations;
						room->VWallLocations = roomLayout->vWallLocations;
						room->ItemLocations = roomLayout->itemLocations;
						//room->GuardLocations = roomLayout->guardLocations;

						room->GenerateRoom();
					}
				}
			}
		}
	}
}

void ALevelGenerationScriptActor::PlanLayout(){
	//1 == rooom
	//-1 == block
	//2 == start
	//3 == treasure room
	//4 == objective room

	//New layout generation
	//1. Split vertically (between 4 and levelHeight - 4):
	int vSplit = 4 +rand() % (levelHeight - 8);
	//choose start in top or bottom:
	bool startTop = false;
	if (rand() % 2 == 0){
		startTop = true;
	}

	//2. 50% chance to split horizontally (between 4 and levelWidth - 4);
	int hSplitTop = 0;
	int hSplitBottom = 0;
	bool splitTop = false;
	bool splitBottom = false;
	//top...
	if (true || rand() % 2 == 0){
		splitTop = true;
		hSplitTop = 4+rand() % (levelWidth - 8);
	}
	//bottom...
	if (true || rand() % 2 == 0){
		splitBottom = true;
		hSplitBottom = 4+rand() % (levelHeight - 8);
	}

	//3. Place connecting rooms
	int topConV = 0;
	int botConV = 0;
	int midConH = 0;

	//Connect the top two areas
	if (splitTop){
		topConV = rand() % (vSplit - 1);
		layout[hSplitTop - 1][topConV] = 1;
		layout[hSplitTop][topConV] = 1;
	}
	//Connect the bottom two areas 
	if (splitBottom){
		botConV = vSplit + 1 + rand() % (levelHeight - vSplit - 1);
		layout[hSplitBottom - 1][botConV] = 1;
		layout[hSplitBottom][botConV] = 1;
	}
	//Connect the bottom and the top areas
	if (startTop){
		midConH = std::max(hSplitTop, hSplitBottom) + 1 + rand() % (levelWidth - std::max(hSplitTop, hSplitBottom) - 1);
		layout[midConH][vSplit - 1] = 1;
		layout[midConH][vSplit] = 1;
	}
	else {
		midConH = rand() % (std::min(hSplitTop == 0 ? levelWidth : hSplitTop, hSplitBottom == 0 ? levelWidth : hSplitBottom) - 1);
		layout[midConH][vSplit - 1] = 1;
		layout[midConH][vSplit] = 1;
	}
	//Block the border tiles
	for (int i = 0; i < levelWidth; i++){
		if (layout[i][vSplit - 1] != 1) layout[i][vSplit - 1] = -1;
		if (layout[i][vSplit] != 1) layout[i][vSplit] = -1;
	}
	if (splitTop){
		for (int i = 0; i < vSplit; i++){
			if (layout[hSplitTop - 1][i] != 1)layout[hSplitTop - 1][i] = -1;
			if (layout[hSplitTop][i] != 1)layout[hSplitTop][i] = -1;
		}
	}
	if (splitBottom){
		for (int i = vSplit; i < levelHeight; i++){
			if (layout[hSplitBottom - 1][i] != 1)layout[hSplitBottom - 1][i] = -1;
			if (layout[hSplitBottom][i] != 1)layout[hSplitBottom][i] = -1;
		}
	}

	if (splitTop){
		PlanRegion(0, 0, hSplitTop - 2, vSplit - 2, startTop ? true : false, hSplitTop - 2, topConV, startTop ? -1 : midConH, startTop ? -1 : vSplit - 2);
		PlanRegion(hSplitTop + 1, 0, levelWidth - 1, vSplit - 2, false, hSplitTop + 1, topConV, startTop ? midConH : -1, startTop ? vSplit - 2 : -1);
	}
	else {
		PlanRegion(0, 0, levelWidth - 1, vSplit - 2, startTop ? true : false, midConH, vSplit-2, -1, -1);
	}

	if (splitBottom){
		PlanRegion(0, vSplit + 1, hSplitBottom - 2, levelHeight - 1, false, hSplitBottom - 2, botConV, startTop ? -1 : midConH, startTop ? -1 : vSplit + 1);
		PlanRegion(hSplitBottom + 1, vSplit + 1, levelWidth - 1, levelHeight - 1, startTop ? false : true, hSplitBottom + 1, botConV, startTop ? midConH : -1, startTop ? vSplit + 1 : -1);
	}
	else {
		PlanRegion(0, vSplit + 1, levelWidth - 1, levelHeight - 1, startTop ? false : true, midConH, vSplit + 1, -1, -1);
	}


}

void ALevelGenerationScriptActor::PlanRegion(int startX, int startY, int endX, int endY, bool startRegion, int con1X, int con1Y, int con2X = -1, int con2Y = -1){

	int startPosX = 0;
	int startPosY = 0;
	int treasureRoomX = 0;
	int treasureRoomY = 0;
	int objRoomX = 0;
	int objRoomY = 0;

	layout[con1X][con1Y] = 1;

	if (con2X != -1){
		layout[con2X][con2Y] = 1;
	}

	//Pick start position if needed
	if (startRegion){
		while (true){
			startPosX = startX + rand() % (endX - startX);
			startPosY = startY + rand() % (endY - startY);
			if (layout[startPosX][startPosY] == 0){
				layout[startPosX][startPosY] = 2;
				break;
			}
		}
	}

	//Pick treasure room position
	while (true){
		treasureRoomX = startX + rand() % (endX - startX);
		treasureRoomY = startY + rand() % (endY - startY);
		if (layout[treasureRoomX][treasureRoomY] == 0){
			layout[treasureRoomX][treasureRoomY] = 3;
			break;
		}
	}

	//Pick objective room position
	while (true){
		objRoomX = startX + rand() % (endX - startX);
		objRoomY = startY + rand() % (endY - startY);
		if (layout[objRoomX][objRoomY] == 0){
			layout[objRoomX][objRoomY] = 4;
			break;
		}
	}

	//Perform a breadth first traversal of the region starting at one of the "doors" ---- ORRRRR the objective room??
	std::queue<std::pair<int, int>> q;
	q.push(std::pair<int, int>(objRoomX, objRoomY));
	layoutSearchData[q.front().first][q.front().second].visited = true;
	while (!q.empty()){
		std::pair<int, int> current = q.front();
		q.pop();
		if (current.first - 1 >= startX && layoutSearchData[current.first - 1][current.second].visited != true && layout[current.first - 1][current.second] != -1){
			q.push(std::pair<int, int>(current.first - 1, current.second));
			layoutSearchData[current.first - 1][current.second].visited = true;
			layoutSearchData[current.first - 1][current.second].parent = current;
		}
		if (current.first + 1 <= endX && layoutSearchData[current.first + 1][current.second].visited != true && layout[current.first + 1][current.second] != -1){
			q.push(std::pair<int, int>(current.first + 1, current.second));
			layoutSearchData[current.first + 1][current.second].visited = true;
			layoutSearchData[current.first + 1][current.second].parent = current;
		}
		if (current.second - 1 >= startY && layoutSearchData[current.first][current.second - 1].visited != true && layout[current.first][current.second - 1] != -1){
			q.push(std::pair<int, int>(current.first, current.second - 1));
			layoutSearchData[current.first][current.second - 1].visited = true;
			layoutSearchData[current.first][current.second - 1].parent = current;
		}
		if (current.second + 1 <= endY && layoutSearchData[current.first][current.second + 1].visited != true && layout[current.first][current.second + 1] != -1){
			q.push(std::pair<int, int>(current.first, current.second + 1));
			layoutSearchData[current.first][current.second + 1].visited = true;
			layoutSearchData[current.first][current.second + 1].parent = current;
		}
	}






	//4. Place start
	if (startRegion){
		std::pair<int, int> current(startPosX, startPosY);
		while (current.first > -1){
			current = layoutSearchData[current.first][current.second].parent;
			if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
		}
	}

	//5. Place treasure rooms
	std::pair<int, int> current(treasureRoomX, treasureRoomY);
	while (current.first > -1){
		current = layoutSearchData[current.first][current.second].parent;
		if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
	}

	//6. Place objective rooms
	current = std::pair<int, int>(con1X, con1Y);
	while (current.first > -1){
		current = layoutSearchData[current.first][current.second].parent;
		if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
	}

	//Link up the second door
	if (con2X != -1){
		current = std::pair<int, int>(con2X, con2Y);
		while (current.first > -1){
			current = layoutSearchData[current.first][current.second].parent;
			if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
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
