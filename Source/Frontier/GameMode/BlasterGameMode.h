// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FRONTIER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(class APlayerCharacter* ElimdCharacter, class AFrontierPlayerController* VictimController, AFrontierPlayerController* AttackerController);
};
