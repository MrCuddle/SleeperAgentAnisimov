// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "Item.h"


AItem::AItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StaticMesh1 = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent >(this, TEXT("ItemMesh"));
	StaticMesh1->bVisible = true;
	RootComponent = StaticMesh1;

	Sphere1 = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere1"));
	Sphere1->InitSphereRadius(50.0f);
	Sphere1->AttachParent = RootComponent;

	Sphere1->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);        // set up a notification for when this component overlaps something

	
}

void AItem::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Picked up: " + Name);

		ACharacter* myCharacter;

		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			myCharacter = Cast<ACharacter>(*ActorItr);
		}

	}
}

