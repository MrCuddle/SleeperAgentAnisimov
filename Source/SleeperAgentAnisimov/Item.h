// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS()
class SLEEPERAGENTANISIMOV_API AItem : public AActor
{
	GENERATED_BODY()
public:
	
	UPROPERTY(VisibleAnywhere, Category = "Mesh Components")
	class UStaticMeshComponent* StaticMesh1;
	
	UPROPERTY(VisibleAnywhere, Category = "Sphere Component")
	class USphereComponent* Sphere1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties")
	bool Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties")
	FString Name;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	AItem(const FObjectInitializer& ObjectInitializer);
	
	
	
	
};
