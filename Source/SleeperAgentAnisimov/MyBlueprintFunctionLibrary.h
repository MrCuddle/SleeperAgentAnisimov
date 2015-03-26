// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SleeperAgentAnisimov/CharacterCPPIntermediate.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	//Returns true of the actors light-sensing component is visible to any light in the world
	//If DrawDebugLines is true every light will be checked, otherwise the first hit return true
	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", FriendlyName = "IsInShadows"), Category = "StaniSneak")
		static bool IsInShadows(ACharacterCPPIntermediate* actor, bool DrawDebugLines);
	
	
	
};
