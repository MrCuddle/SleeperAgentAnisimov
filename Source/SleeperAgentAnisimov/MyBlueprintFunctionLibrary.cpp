// Fill out your copyright notice in the Description page of Project Settings.

#include "SleeperAgentAnisimov.h"
#include "MyBlueprintFunctionLibrary.h"
#include <string>

bool UMyBlueprintFunctionLibrary::IsInShadows(ACharacterCPPIntermediate* actor, bool DrawDebugLines)
{
	if (!actor) return false;
	bool inShadow = true;
		
	for (TObjectIterator<ULightComponent> it; it; ++it)
	{
		if (!it->IsVisible()) continue;
		if (it->GetLightType() == LightType_Directional) continue;
		if (!it->AffectsPrimitive(actor->GetLightSensitiveComponent())) continue;
		FCollisionResponseParams resp;

		if (actor->GetWorld()->LineTraceTest(
			it->GetComponentLocation(),
			actor->GetLightSensitiveComponent()->GetComponentLocation(),
			ECC_Visibility,
			FCollisionQueryParams("Test", true, actor))) continue;

		inShadow = false;

		if (DrawDebugLines)
		{
			DrawDebugLine(actor->GetWorld(), it->GetComponentLocation(), actor->GetLightSensitiveComponent()->GetComponentLocation(), FColor::Yellow);
			continue;
		}
		break;

	}
	
	return inShadow;
}

