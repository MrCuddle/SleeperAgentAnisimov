// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LevelScriptActor.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <queue>
#include <utility>
#include <vector>
#include "LevelGenerationScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API ALevelGenerationScriptActor : public ALevelScriptActor
{


	TSubclassOf<class AActor> RoomBlueprint;
	TSubclassOf<class AActor> RoomBlueprint2;
	ALevelGenerationScriptActor(const class FObjectInitializer& ObjectInitializer);

	GENERATED_BODY()
	int layout[9][9];
	int nRooms;
	std::queue<std::pair<int, int>> rooms;

public:
	/** Generate a level*/
	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	void GenerateLevel();

private:
	std::vector<TSubclassOf<class AActor>> northRooms;
	std::vector<TSubclassOf<class AActor>> eastRooms;
	std::vector<TSubclassOf<class AActor>> southRooms;
	std::vector<TSubclassOf<class AActor>> westRooms;

	void ExploreLevel();
	int GetAdjacentRooms(int i, int j);
	int GetDistanceFromStart(int i, int j);
		
};
