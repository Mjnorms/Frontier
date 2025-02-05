// Mjnorms -- 2024


#include "BlasterGameMode.h"
#include "Frontier/PlayerCharacter.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Frontier/PlayerState/BlasterPlayerState.h"

void ABlasterGameMode::PlayerEliminated(APlayerCharacter* ElimdCharacter, AFrontierPlayerController* VictimController, AFrontierPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}

	if (ElimdCharacter)
	{
		ElimdCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimdCharacter, AController* ElimdController)
{
	if (ElimdCharacter)
	{
		ElimdCharacter->Reset();
		ElimdCharacter->Destroy();
	}
	if (ElimdController)
	{
		TArray<AActor*> StartActors;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), StartActors);
		int32 RandSelection = FMath::RandRange(0, StartActors.Num() - 1);
		RestartPlayerAtPlayerStart(ElimdController, StartActors[RandSelection]);
	}
}
