// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "Item.h"


AItem::AItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StaticMesh1 = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent >(this, TEXT("ItemMesh"));
	StaticMesh1->bVisible = true;

	Capsule1 = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("Capsule1"));
	Capsule1->InitCapsuleSize(50, 100);
	FRotator capsuleRotator = FRotator(90.0f, 0.0f, 0.0f);
	Capsule1->SetWorldRotation(capsuleRotator);
	
	myRootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));

	Arrow1 = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow1"));

	SetRootComponent(myRootComponent);

	StaticMesh1->AttachParent = RootComponent;
	Capsule1->AttachParent = RootComponent;
	/*RootComponent = Capsule1;

	
	StaticMesh1->AttachParent = RootComponent;*/

	Capsule1->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);        // set up a notification for when this component overlaps something

	
}

void AItem::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Name + " Collided with: " + OtherActor->GetName());
		
	}
}

