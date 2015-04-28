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
#include <fstream>


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
			TArray<TSharedPtr<FJsonValue>> cameras = spawnGroups[i]->AsObject()->GetArrayField("cameras");
			for (int j = 0; j < cameras.Num(); ++j){
				FCameraStruct camera = FCameraStruct();
				camera.location = FVector2D(cameras[j]->AsObject()->GetNumberField("x"), cameras[j]->AsObject()->GetNumberField("y"));
				camera.rotation = cameras[j]->AsObject()->GetNumberField("rotation");
				roomLayout->cameras.Add(camera);

			}

			TArray<TSharedPtr<FJsonValue>> lights = spawnGroups[i]->AsObject()->GetArrayField("lights");
			for (int j = 0; j < lights.Num(); ++j){
				FLightStruct light = FLightStruct();
				light.location = FVector2D(lights[j]->AsObject()->GetNumberField("x"), lights[j]->AsObject()->GetNumberField("y"));
				light.radius = lights[j]->AsObject()->GetNumberField("radius");
				roomLayout->lights.Add(light);

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
					if (layout[i][j] == 1 || layout[i][j] == 5) //Normal room
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
					if (j + 1 < levelHeight && layout[i][j + 1] > 0)
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
					if (i + 1 < levelWidth && layout[i + 1][j] > 0)
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
					if (j + 1 >= levelHeight || layout[i][j + 1] <= 0)
					{
						std::vector<RoomLayout*> newOutput;
						std::set_difference(outputSet.begin(), outputSet.end(), southRooms.begin(), southRooms.end(), std::back_inserter(newOutput));
						if (newOutput.size() > 0)
							outputSet = newOutput;
					}
					if (i + 1 >= levelWidth || layout[i + 1][j] <= 0)
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

                    ABaseRoomActor* room = (ABaseRoomActor*)world->SpawnActor<AActor>(roomLoaderBlueprint, FVector(2020 * i, 2020 * j, 0), FRotator(0, 0, 0));
                    layoutRooms[i][j] = room;

					//Set player spawn if this is the start room
					if (layout[i][j] == 2){ 
						PlayerSpawn = FVector2D(2020 * i + 1000, 2020 * j + 1000);
                        StartRoom = layoutRooms[i][j];
					}


					if (room){
						if (j + 1 < levelHeight && layout[i][j + 1] > 0)
							room->SouthDoor = true;
						else
							room->SouthDoor = false;
						
						if (i  + 1 < levelWidth && layout[i + 1][j] > 0)
							room->EastDoor = true;
						else
							room->EastDoor = false;
						
						if (i > 0 && layout[i - 1][j] > 0)
							room->WestDoor = true;
						else
							room->WestDoor = false;
						
						if (j > 0 && layout[i][j - 1] > 0)
							room->NorthDoor = true;
						else
							room->NorthDoor = false;

						room->NorthDoorPossible = roomLayout->northDoor;
						room->EastDoorPossible = roomLayout->eastDoor;
						room->SouthDoorPossible = roomLayout->southDoor;
						room->WestDoorPossible = roomLayout->westDoor;

						//If this is the start room, don't start it with a ceiling
						if (layout[i][j] == 2){
							room->Ceiling = false;
						}
						else {
							room->Ceiling = true;
						}

                        if (layout[i][j] == 4)
                        {
                            for (size_t k = 0; k < indexOfObjectiveRooms.size(); ++k)
                            {
                                if (indexOfObjectiveRooms[k].first == i && indexOfObjectiveRooms[k].second == j)
                                    room->orderOfObjectives = k;
                            }
                        }
                        else
                            room->orderOfObjectives = -1;

						room->StaticMeshes = roomLayout->staticMeshes;
						room->ItemLocations = roomLayout->itemLocations;
						room->Guards = roomLayout->guards;
						room->PatrolRoutes = roomLayout->patrolRoutes;
						room->Cameras = roomLayout->cameras;
						room->Lights = roomLayout->lights;

						room->GenerateRoom();
					}
				}
			}
		}

		//Go through the spawned rooms and set up adjacency for their doors
		for (int i = 0; i < levelWidth; i++)
		{
			for (int j = 0; j < levelHeight; j++)
			{
				if (layoutRooms[i][j] != nullptr)
				{
					if (layoutRooms[i][j]->NorthDoor){
						if (layout[i][j - 1] == 5)
							layoutRooms[i][j]->northDoorActor->NeedsKeycard = true;
						layoutRooms[i][j]->northDoorActor->roomA = layoutRooms[i][j];
						layoutRooms[i][j]->northDoorActor->roomB = layoutRooms[i][j - 1];
					}
					if (layoutRooms[i][j]->EastDoor){
						if (layout[i][j] == 5)
							layoutRooms[i][j]->eastDoorActor->NeedsKeycard = true;
						layoutRooms[i][j]->eastDoorActor->roomA = layoutRooms[i][j];
						layoutRooms[i][j]->eastDoorActor->roomB = layoutRooms[i + 1][j];
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
	//5 == keycard door room


	//1. Split vertically (between 2 and levelHeight - 2):
	int vSplit = 2 + rand() % (levelHeight - 3);
	//1. Split horizontally (between 2 and levelWidth - 2):
	int hSplit = 2 + rand() % (levelWidth - 3);

	//door from northwest to northeast
	int door1 = 0 + rand() % (vSplit);
	//door from northeast to south
	int door2 = hSplit + 1 + rand() % (levelWidth - hSplit - 1);

	//Block the tiles between the north and south regions, except the door
	for (int i = 0; i < levelWidth; i++){
		if (i == door2)
			layout[i][vSplit] = 5;
		else
			layout[i][vSplit] = -1;
	}

	//Block the tiles between the northwest and northeast regions, except the door
	for (int i = 0; i < vSplit; i++){
		if (i == door1)
			layout[hSplit][i] = 5;
		else
			layout[hSplit][i] = -1;
	}


	PlanRegion(0, 0, hSplit, vSplit, true /*start*/, hSplit, door1, -1,-1);
	PlanRegion(hSplit, 0, levelWidth - 1, vSplit, false /*not start*/, hSplit, door1, door2, vSplit);
	PlanRegion(0, vSplit, levelWidth - 1, levelHeight - 1, false /*not start*/, door2, vSplit, -1, -1);


	FString path;
	path = FPaths::GameDir();
	path += "/roomlayout.txt";

	//output the layout to a file!
	//auto out = std::fstream("roomlayout.txt", std::ios_base::out);

	std::ofstream stream;
	stream.open(TCHAR_TO_UTF8(*path), std::ios_base::trunc); 
	
	for (int j = 0; j < levelHeight; j++){
		for (int i= 0; i < levelWidth; i++){
			stream << layout[i][j] << ",";
		}
		stream << std::endl;
	}

	stream.close();
	

}

void ALevelGenerationScriptActor::PlanRegion(int startX, int startY, int endX, int endY, bool startRegion, int conn1X, int conn1Y, int conn2X, int conn2Y){

	for (int i = 0; i < levelWidth; i++)
		for (int j = 0; j < levelHeight; j++)
			layoutSearchData[i][j].visited = false;

	int startPosX = 0;
	int startPosY = 0;
	int treasureRoomX = 0;
	int treasureRoomY = 0;
	int objRoomX = 0;
	int objRoomY = 0;


	//Pick start position if needed
	if (startRegion){
		while (true){
			startPosX = startX + rand() % (endX + 1 - startX);
			startPosY = startY + rand() % (endY + 1 - startY);
			if (layout[startPosX][startPosY] == 0){
				layout[startPosX][startPosY] = 2;
				break;
			}
		}
	}

	//Pick treasure room position
	while (true){
		treasureRoomX = startX + rand() % (endX + 1 - startX);
		treasureRoomY = startY + rand() % (endY + 1 - startY);
		if (layout[treasureRoomX][treasureRoomY] == 0){
			layout[treasureRoomX][treasureRoomY] = 3;
			break;
		}
	}

	//Pick objective room position
	while (true){
		objRoomX = startX + rand() % (endX + 1 - startX);
		objRoomY = startY + rand() % (endY + 1 - startY);
		if (layout[objRoomX][objRoomY] == 0){
			layout[objRoomX][objRoomY] = 4;
			break;
		}
	}
    indexOfObjectiveRooms.emplace_back(objRoomX, objRoomY);

	//Perform a breadth first traversal of the region starting at one of the "doors" ---- ORRRRR the objective room??
	std::vector<std::pair<int, int>> q;
	q.push_back(std::pair<int, int>(objRoomX, objRoomY));
	layoutSearchData[q.front().first][q.front().second].visited = true;
	while (!q.empty()){
		std::pair<int, int> current = q.front();
		q.erase(q.cbegin());
		if (current.first - 1 >= startX && layoutSearchData[current.first - 1][current.second].visited != true && layout[current.first - 1][current.second] != -1){
			q.push_back(std::pair<int, int>(current.first - 1, current.second));
			layoutSearchData[current.first - 1][current.second].visited = true;
			layoutSearchData[current.first - 1][current.second].parent = current;
		}
		if (current.first + 1 <= endX && layoutSearchData[current.first + 1][current.second].visited != true && layout[current.first + 1][current.second] != -1){
			q.push_back(std::pair<int, int>(current.first + 1, current.second));
			layoutSearchData[current.first + 1][current.second].visited = true;
			layoutSearchData[current.first + 1][current.second].parent = current;
		}
		if (current.second - 1 >= startY && layoutSearchData[current.first][current.second - 1].visited != true && layout[current.first][current.second - 1] != -1){
			q.push_back(std::pair<int, int>(current.first, current.second - 1));
			layoutSearchData[current.first][current.second - 1].visited = true;
			layoutSearchData[current.first][current.second - 1].parent = current;
		}
		if (current.second + 1 <= endY && layoutSearchData[current.first][current.second + 1].visited != true && layout[current.first][current.second + 1] != -1){
			q.push_back(std::pair<int, int>(current.first, current.second + 1));
			layoutSearchData[current.first][current.second + 1].visited = true;
			layoutSearchData[current.first][current.second + 1].parent = current;
		}
		if (rand() % 4 == 3){
			std::random_shuffle(q.begin(), q.end());
		}

	}


	//4. Place start
	if (startRegion){
		std::pair<int, int> current(startPosX, startPosY);
		while (true){
			if (current.first == objRoomX && current.second == objRoomY) break;
			current = layoutSearchData[current.first][current.second].parent;
			if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
		}
	}

	//5. Place treasure rooms
	std::pair<int, int> current(treasureRoomX, treasureRoomY);
	while (true){
		if (current.first == objRoomX && current.second == objRoomY) break;
		current = layoutSearchData[current.first][current.second].parent;
		if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
	}

	//Link up the first door
	if (conn1X != -1){
		current = std::pair<int, int>(conn1X, conn1Y);
		while (true){
			if (current.first == objRoomX && current.second == objRoomY) break;
			current = layoutSearchData[current.first][current.second].parent;
			if (layout[current.first][current.second] < 1) layout[current.first][current.second] = 1;
		}
	}

	//Link up the second door
	if (conn2X != -1){
		current = std::pair<int, int>(conn2X, conn2Y);
		while (true){
			if (current.first == objRoomX && current.second == objRoomY) break;
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
