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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Components")
	class UStaticMeshComponent* StaticMesh1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Component")
	class UCapsuleComponent* Capsule1;

	UPROPERTY(VisibleAnywhere, Category = "Item Component")
	class USceneComponent* myRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Component")
	class UArrowComponent* Arrow1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Component")
	class UPawnNoiseEmitterComponent* NoiseEmitter1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties")
	bool Spawnable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Properties")
	FString Name;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ItemFunctions")
	void UseItem();
	virtual void UseItem_Implementation();

	AItem(const FObjectInitializer& ObjectInitializer);

private:
	TSubclassOf<class UActorComponent> ItemComponentBlueprint;
	
	
};
