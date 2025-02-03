// Mjnorms -- 2024


#include "BlasterGameMode.h"
#include "Frontier/PlayerCharacter.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"

void ABlasterGameMode::PlayerEliminated(APlayerCharacter* ElimdCharacter, AFrontierPlayerController* VictimController, AFrontierPlayerController* AttackerController)
{
	if (ElimdCharacter)
	{
		ElimdCharacter->Elim();
	}
}
