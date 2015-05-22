// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "LoggingFunctions.h"

const string ULoggingFunctions::RootDirectory = "LOGGING";
const string ULoggingFunctions::versionFormat = "v";
const string ULoggingFunctions::fileEnding = ".log";
string ULoggingFunctions::SessionDirectory = string();
bool ULoggingFunctions::newSession = true;
bool ULoggingFunctions::sessionInitialized = false;
int ULoggingFunctions::versionNumber = 0;

void ULoggingFunctions::CreateLoggingFile(string fileName, string labels)
{
	time_t now = time(0);
	char* date = ctime(&now);	
	ofstream stream;
	stream.open(fileName + fileEnding, ios_base::app);
	stream << date << '\n';
	stream << labels << '\n';
	stream.close();
}

bool ULoggingFunctions::StartNewSession()
{
	time_t now = time(0);
	char* date = ctime(&now);
	string dateFormatted = date;
	dateFormatted.erase(remove_if(dateFormatted.begin(), dateFormatted.end(), isspace), dateFormatted.end());
	dateFormatted.erase(remove(dateFormatted.begin(), dateFormatted.end(), ':'), dateFormatted.end());

	FString p = FPaths::GameDir();
	p += RootDirectory.c_str();
	string path = string(TCHAR_TO_UTF8(*p));
	path += "/" + dateFormatted;
	FString pathTest = path.c_str();

	if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*pathTest)){
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: Path: '" + pathTest + "' already exists. Logging session NOT started");
		return false;
	}
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*pathTest);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*pathTest)){
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: Could not create path: '" + pathTest + "'. Logging session NOT started");
		return false;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "LOGGING: Created path: '" + pathTest + "', logging session started");
	
	//Create files
	CreateLoggingFile(path + "/Health", "Time\tInstigator\tLocation");
	CreateLoggingFile(path + "/Deaths", "Time\tLocation");
	CreateLoggingFile(path + "/ItemPickups", "Time\tItem\tLocation");
	CreateLoggingFile(path + "/LevelLayout", "Layout");

	sessionInitialized = true;
	SessionDirectory = path;
	return true;
}

bool ULoggingFunctions::SaveStringToFile(FString fileName, FString SaveText, FString& Result){
	FString path;
	path = FPaths::GameDir();
	path += RootDirectory.c_str();
	if (!sessionInitialized)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: No Session Started before logging. No data will be logged");
		Result = "FAIL";
		return false;
	}

	if (newSession)
	{
		time_t now = time(0);
		char* date = ctime(&now);
		path += "/";
		path += date;
	}

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
	stream << data << '\n';
	stream.close();
	
	Result = path + " : " + SaveText;
	return true;
}

void ULoggingFunctions::SaveLog(string fileName, string data)
{
	ofstream stream;
	stream.open(SessionDirectory + '/' + fileName + fileEnding, ios_base::app);
	stream << data << '\n';
	stream.close();
}

bool ULoggingFunctions::LogActorHealth(AActor* actor, AActor* instigator, int32 health, FString& Result)
{	
	if (actor == NULL)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: LogActorHealth FAILED");
		return false;
	}
	string data = to_string(GetElapsedTime(actor));
	data += '\t' + to_string(health) + '\t';
	data += (instigator != NULL) ? TCHAR_TO_UTF8(*instigator->GetName()) : "NONE";
	data += '\t';
	FVector location = actor->GetActorLocation();
	data += TCHAR_TO_UTF8(*location.ToString());
	SaveLog("Health", data);
	Result = data.c_str();
	return true;
}

bool ULoggingFunctions::LogPlayerDeath(AActor* player, FString& Result)
{	
	if (player == NULL)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: LogPlayerDeath FAILED");
		return false;
	}
	string data = to_string(GetElapsedTime(player));
	data += '\t';
	FVector location = player->GetActorLocation();
	data += TCHAR_TO_UTF8(*location.ToString());
	SaveLog("Death", data);
	Result = data.c_str();
	return true;
}

bool ULoggingFunctions::LogItemPickup(AActor* player, AItem* item, FString& Result)
{
	if (player == NULL)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "LOGGING: LogItemPickup FAILED");
		return false;
	}
	
	string data = to_string(GetElapsedTime(player));
	data += '\t';
	data += (item != NULL) ? TCHAR_TO_UTF8(*item->GetName()) : "NONE";
	data += '\t';
	FVector location = player->GetActorLocation();
	data += TCHAR_TO_UTF8(*location.ToString());
	SaveLog("ItemPickups", data);
	Result = data.c_str();
	return true;
}

bool ULoggingFunctions::LogLevelLayout(vector<vector<string>>& rooms){

	string data = "";
	for (int j = 0; j < rooms.size(); j++){
		for (int i = 0; i < rooms[j].size(); i++){
			data += rooms[j][i] + "\t\t\t\t\t\t";
		}
		data += '\n';
	}

	data += "\n\n";

	for (int j = 0; j < rooms.size(); j++){
		for (int i = 0; i < rooms[j].size(); i++){
			if (rooms[j][i] == "none")
				data += '0';
			else
				data += '#';
		}
		data += '\n';
	}

	SaveLog("LevelLayout", data);

	return true;

}
