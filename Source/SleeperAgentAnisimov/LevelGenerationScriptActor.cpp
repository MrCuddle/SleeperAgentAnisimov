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
	sort(normalRooms.begin(), normalRooms.end());
	sort(treasureRooms.begin(), treasureRooms.end());
	sort(objectiveRooms.begin(), objectiveRooms.end());
	sort(startRooms.begin(), startRooms.end());
	
}

void ALevelGenerationScriptActor::LoadRoomLayout(FString path){

	RoomLayout *roomLayout = new RoomLayout();

	FString text = FString();
	FFileHelper::LoadFileToString(text, path.GetCharArray().GetData());

	//std::string s = TCHAR_TO_UTF8(text.GetCharArray().GetData());

	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(text);
	int sc = 0;

	FJsonSerializer serializer = FJsonSerializer();
	if (serializer.Deserialize(JsonReader, JsonParsed)){
		//Doors
		roomLayout->northDoor = JsonParsed->GetBoolField("northDoor");
		roomLayout->eastDoor = JsonParsed->GetBoolField("eastDoor");
		roomLayout->southDoor = JsonParsed->GetBoolField("southDoor");
		roomLayout->westDoor = JsonParsed->GetBoolField("westDoor");

		//Rarity
		roomLayout->rarity = (int)JsonParsed->GetNumberField("roomRarity");

		//Type
		roomLayout->type = (int)JsonParsed->GetNumberField("roomType");

		//Patrol routes
		TArray<TSharedPtr<FJsonValue>> patrolRoutes = JsonParsed->GetArrayField("patrolRoutes");
		for (int i = 0; i < patrolRoutes.Num(); ++i){
			FPatrolRouteStruct pr = FPatrolRouteStruct();

			TArray<TSharedPtr<FJsonValue>> patrolPoints = patrolRoutes[i]->AsObject()->GetArrayField("patrolPoints");
			for (int j = 0; j < patrolPoints.Num(); ++j){
				pr.patrolPoints.Add(FVector2D(patrolPoints[j]->AsObject()->GetNumberField("x"), patrolPoints[j]->AsObject()->GetNumberField("y")));
			}
			roomLayout->patrolRoutes.Add(pr);
		}

		//Guards, items and meshes
		TArray<TSharedPtr<FJsonValue>> spawnGroups = JsonParsed->GetArrayField("spawnGroups");
		for (int i = 0; i < spawnGroups.Num(); ++i){
			TArray<TSharedPtr<FJsonValue>> guards = spawnGroups[i]->AsObject()->GetArrayField("guards");
			for (int j = 0; j < guards.Num(); ++j){
				FGuardStruct g = FGuardStruct();
				g.patrolRouteIndex = guards[j]->AsObject()->GetNumberField("patrolRouteIndex");
				g.startIndex = (int)guards[j]->AsObject()->GetNumberField("startIndex");
				g.spawnLocation = FVector2D(guards[j]->AsObject()->GetNumberField("x"), guards[j]->AsObject()->GetNumberField("y"));
				roomLayout->guards.Add(g);
			}
			TArray<TSharedPtr<FJsonValue>> items = spawnGroups[i]->AsObject()->GetArrayField("items");
			for (int j = 0; j < items.Num(); ++j){
				roomLayout->itemLocations.Add(FVector2D(items[j]->AsObject()->GetNumberField("x"), items[j]->AsObject()->GetNumberField("y")));
			}
			TArray<TSharedPtr<FJsonValue>> meshes = spawnGroups[i]->AsObject()->GetArrayField("meshes");
			for (int j = 0; j < meshes.Num(); ++j){
				FStaticMeshStruct mesh = FStaticMeshStruct();
				mesh.name = meshes[j]->AsObject()->GetStringField("staticMesh");
				mesh.location = FVector2D(meshes[j]->AsObject()->GetNumberField("x"), meshes[j]->AsObject()->GetNumberField("y"));
				mesh.rotation = meshes[j]->AsObject()->GetNumberField("rotation");
				roomLayout->staticMeshes.Add(mesh);

			}
		}

	}

	//Set up the room sets
	if (roomLayout->northDoor) northRooms.push_back(roomLayout);
	if (roomLayout->eastDoor) eastRooms.push_back(roomLayout);
	if (roomLayout->southDoor) southRooms.push_back(roomLayout);
	if (roomLayout->westDoor) westRooms.push_back(roomLayout);
	switch (roomLayout->type) {
	case 0:
		normalRooms.push_back(roomLayout);
		break;
	case 1:
		treasureRooms.push_back(roomLayout);
		break;
	case 2:
		objectiveRooms.push_back(roomLayout);
		break;
	case 3:
		startRooms.push_back(roomLayout);
		break;
	}

}

void ALevelGenerationScriptActor::GenerateLevel(){
	nRooms = 16;
	UWorld* const world = GetWorld();
	if (world){

		PlanLayout();

		//Spawn the rooms according to planned layout
		for (int i = 0; i < levelWidth; i++)
		{
			for (int j = 0; j < levelHeight; j++)
			{
				if (layout[i][j] > 0)
				{
					std::vector<RoomLayout*> outputSet;

					//Pick potential rooms based on room TYPE
					if (layout[i][j] == 1) //Normal room
					{
						if (outputSet.empty())
						{
							outputSet = normalRooms;
						}
						else {
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), normalRooms.begin(), normalRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}
					else if (layout[i][j] == 2) //Start room
					{
						if (outputSet.empty())
						{
							outputSet = startRooms;
						}
						else {
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), startRooms.begin(), startRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}
					else if (layout[i][j] == 3) //Treasure room
					{
						if (outputSet.empty())
						{
							outputSet = treasureRooms;
						}
						else {
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), treasureRooms.begin(), treasureRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}
					else if (layout[i][j] == 4) //Objective room
					{
						if (outputSet.empty())
						{
							outputSet = objectiveRooms;
						}
						else {
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), objectiveRooms.begin(), objectiveRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
						}
					}


					//Pick potential rooms based on available doors
					if (j < 8 && layout[i][j + 1] > 0)
					{
						if (outputSet.empty())
						{
							outputSet = southRooms;
						}
						else {
							std::vector<RoomLayout*> newOutput;
							std::set_intersection(outputSet.begin(), outputSet.end(), southRooms.begin(), southRooms.end(), std::back_inserter(newOutput));
							outputSet = newOutput;
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

					//Try to pick rooms with fewer doors, if possible
					if (j >=8 || layout[i][j + 1] <= 0)
					{
						std::vector<RoomLayout*> newOutput;
						std::set_difference(outputSet.begin(), outputSet.end(), southRooms.begin(), southRooms.end(), std::back_inserter(newOutput));
						if (newOutput.size() > 0)
							outputSet = newOutput;
					}
					if (i >= 8 || layout[i + 1][j] <= 0)
					{
						std::vector<RoomLayout*> newOutput;
						std::set_difference(outputSet.begin(), outputSet.end(), eastRooms.begin(), eastRooms.end(), std::back_inserter(newOutput));
						if (newOutput.size() > 0)
							outputSet = newOutput;
					}
					if (i <= 0 || layout[i - 1][j] <= 0)
					{
						std::vector<RoomLayout*> newOutput;
						std::set_difference(outputSet.begin(), outputSet.end(), westRooms.begin(), westRooms.end(), std::back_inserter(newOutput));
						if (newOutput.size() > 0)
							outputSet = newOutput;
					}
					if (j <= 0 || layout[i][j - 1] <= 0)
					{
						std::vector<RoomLayout*> newOutput;
						std::set_difference(outputSet.begin(), outputSet.end(), northRooms.begin(), northRooms.end(), std::back_inserter(newOutput));
						if (newOutput.size() > 0)
							outputSet = newOutput;
					}

					RoomLayout* roomLayout = outputSet[rand() % outputSet.size()];

					//Set player spawn if this is the start room
					if (layout[i][j] == 2){ 
						PlayerSpawn = FVector2D(1200 * (i - 4) + 600, 1200 * (j - 4) + 600);
					}


					ABaseRoomActor* room = (ABaseRoomActor*)world->SpawnActor<AActor>(roomLoaderBlueprint, FVector(1200 * (i - 4), 1200 * (j - 4), 0), FRotator(0, 0, 0));
					

					if (room){
						if (j < levelHeight && layout[i][j + 1] > 0)
							room->SouthDoor = true;
						if (i < levelWidth && layout[i + 1][j] > 0)
							room->EastDoor = true;
						if (i > 0 && layout[i - 1][j] > 0)
							room->WestDoor = true;
						if (j > 0 && layout[i][j - 1] > 0)
							room->NorthDoor = true;

						room->NorthDoorPossible = roomLayout->northDoor;
						room->EastDoorPossible = roomLayout->eastDoor;
						room->SouthDoorPossible = roomLayout->southDoor;
						room->WestDoorPossible = roomLayout->westDoor;

						room->StaticMeshes = roomLayout->staticMeshes;
						
						room->ItemLocations = roomLayout->itemLocations;
						room->Guards = roomLayout->guards;
						room->PatrolRoutes = roomLayout->patrolRoutes;

						room->GenerateRoom();
					}
				}
			}
		}
	}
}

void ALevelGenerationScriptActor::PlanLayout(){
	//1 == room
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
