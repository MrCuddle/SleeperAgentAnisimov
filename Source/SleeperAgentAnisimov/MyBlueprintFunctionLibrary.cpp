// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "MyBlueprintFunctionLibrary.h"
#include <string>

FText UMyBlueprintFunctionLibrary::STATICCPPTEST()
{
	FString asdas = "HHH";
	asdas.Append("asdawda");
	std::string asd(TCHAR_TO_UTF8(*asdas));
	
	
	return FText::FromString("HEJ SIMON");
}

void UMyBlueprintFunctionLibrary::STATICCPPVOID()
{

}

