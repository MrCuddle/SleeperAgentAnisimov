// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LevelScriptActor.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <queue>
#include <utility>
#include <vector>
#include "RoomLayout.h"
#include "LevelGenerationScriptActor.generated.h"


struct SearchData{
	bool visited;
	std::pair<int, int> parent;
	SearchData(){
		visited = false;
		parent = std::pair<int, int>(-1, -1);
	}
	
};

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API ALevelGenerationScriptActor : public ALevelScriptActor
{
	static const int levelWidth = 10;
	static const int levelHeight = 10;

	TSubclassOf<class AActor> roomLoaderBlueprint;
	ALevelGenerationScriptActor(const class FObjectInitializer& ObjectInitializer);

	GENERATED_BODY()
	int layout[levelWidth][levelHeight];
	SearchData layoutSearchData[levelWidth][levelHeight];
	int nRooms;
	std::queue<std::pair<int, int>> rooms;

public:
	/** Generate a level*/
	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void GenerateLevel();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
	FVector2D PlayerSpawn;

private:
	std::vector<RoomLayout*> northRooms;
	std::vector<RoomLayout*> eastRooms;
	std::vector<RoomLayout*> southRooms;
	std::vector<RoomLayout*> westRooms;

	void LoadRoomLayouts();
	void LoadRoomLayout(FString path);
	void ExploreLevel();
	void PlanLayout();
	void PlanRegion(int startX, int startY, int endX, int endY, bool startRegion, int con1X, int con1Y, int con2X, int con2Y);
	int GetAdjacentRooms(int i, int j);
	int GetDistanceFromStart(int i, int j);
		
};
