// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
		UFUNCTION(BlueprintPure, meta = (FriendlyName = "Hejsan!", CompactNodeTitle = "Test", Keywords = "asdawd"), Category = Game)
		static FText Test();
	
	
	
};
