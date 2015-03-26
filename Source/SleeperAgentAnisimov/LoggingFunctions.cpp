// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "LoggingFunctions.h"

const string ULoggingFunctions::outputDir = "LOGGING";
const string ULoggingFunctions::versionFormat = "v";
const string ULoggingFunctions::fileEnding = ".log";
bool ULoggingFunctions::newSession = true;
int ULoggingFunctions::versionNumber = 0;

bool ULoggingFunctions::SaveToFile_SaveStringTextToFile(FString fileName, FString SaveText, FString& Result){
	FString path;
	path = FPaths::GameDir();
	path += outputDir.c_str();

	//check if directory exists. If not: create it
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*path)){
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*path);

		if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*path)){
			return false;
		}
	}
	//Append filename to the path
	path += "/" + fileName + "." + versionFormat.c_str();
	
	//Create temporary string to search with
	FString temp = path + to_string(versionNumber).c_str() + fileEnding.c_str();
	
	//Find version
	if (newSession)
	{
		while (FPlatformFileManager::Get().GetPlatformFile().FileExists(*temp))
		{
			versionNumber++;
			temp = path + to_string(versionNumber).c_str() + fileEnding.c_str();
		}
	}
	
	//Save data to the file	
	ofstream stream;
	stream.open(TCHAR_TO_UTF8(*temp), ios_base::app);	
	if (newSession)
	{
		newSession = false;
		time_t now = time(0);
		char* dt = ctime(&now);
		stream << dt << '\n';
	}
	string data(TCHAR_TO_UTF8(*SaveText));
	stream << data;
	stream.close();
	
	Result = path + " : " + SaveText;
	return true;
}



