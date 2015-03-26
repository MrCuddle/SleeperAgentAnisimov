// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "CharacterCPPIntermediate.generated.h"

UCLASS()
class SLEEPERAGENTANISIMOV_API ACharacterCPPIntermediate : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterCPPIntermediate();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "StaniSneak")
	UPrimitiveComponent* GetLightSensitiveComponent();
	virtual UPrimitiveComponent* GetLightSensitiveComponent_Implementation();
	
};
