// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Item.h"
#include <string>
#include <fstream>
#include <ctime>
#include <algorithm>
using namespace std;
#include "LoggingFunctions.generated.h"


/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API ULoggingFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "StartNewSession"), Category = "StaniLogging")
		static bool StartNewSession();

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "SaveToFile"), Category = "StaniLogging")
		static bool SaveStringToFile(FString fileName, FString SaveText, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogPlayerHealth"), Category = "StaniLogging")
		static bool LogActorHealth(AActor* actor, AActor* instigator, int32 health, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogPlayerDeath"), Category = "StaniLogging")
		static bool LogPlayerDeath(AActor* player, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogItemPickup"), Category = "StaniLogging")
		static bool LogItemPickup(AActor* player, AItem* item, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogGuardNoticePlayer"), Category = "StaniLogging")
		static bool LogGuardNoticePlayer(AActor* guard, bool sawPlayer, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogSpawnedItem"), Category = "StaniLogging")
		static bool LogSpawnedItem(AItem* item, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogSpawnedFilingCabinets"), Category = "StaniLogging")
		static bool LogSpawnedFilingCabinets(AItem* cabinet, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogFilingCabinetsUsed"), Category = "StaniLogging")
		static bool LogFilingCabinetsUsed(AItem* cabinet, FString& Result);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "LogGuardDeath"), Category = "StaniLogging")
		static bool LogGuardDeath(AActor* guard, FString& Result);
	
private:

	inline static float GetElapsedTime(UWorld* contex) { return contex->GetTimeSeconds(); }
	inline static float GetElapsedTime(AActor* contexHolder) { return contexHolder->GetWorld()->GetTimeSeconds(); }

	static void SaveLog(string fileName, string data);

	static void CreateLoggingFile(string fileName, string labels);

	static const string RootDirectory;
	static const string versionFormat;
	static const string fileEnding;
	static string version;
	static string SessionDirectory;
	static bool newSession;
	static bool sessionInitialized;
	static int versionNumber;


	
};
