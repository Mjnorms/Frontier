// Mjnorms -- 2024


#include "BlasterGameMode.h"
#include "Frontier/PlayerCharacter.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ABlasterGameMode::PlayerEliminated(APlayerCharacter* ElimdCharacter, AFrontierPlayerController* VictimController, AFrontierPlayerController* AttackerController)
{
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
