// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "CharacterCPPIntermediate.h"


// Sets default values
ACharacterCPPIntermediate::ACharacterCPPIntermediate()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharacterCPPIntermediate::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterCPPIntermediate::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void ACharacterCPPIntermediate::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

UPrimitiveComponent* ACharacterCPPIntermediate::GetLightSensitiveComponent_Implementation()
{
	return GetCapsuleComponent();
}

