// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include <string>
#include <fstream>
#include <ctime>
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

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "SaveToFile"), Category = "LogginToolsForStani")
		static bool SaveToFile_SaveStringTextToFile(FString fileName, FString SaveText, FString& Result);
	
private:

	static const string outputDir;
	static const string versionFormat;
	static const string fileEnding;
	static string version;
	static bool newSession;
	static int versionNumber;


	
};
