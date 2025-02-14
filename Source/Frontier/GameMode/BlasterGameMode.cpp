// Mjnorms -- 2024


#include "BlasterGameMode.h"
#include "Frontier/PlayerCharacter.h"
#include "Frontier/PlayerController/FrontierPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Frontier/PlayerState/BlasterPlayerState.h"


namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true; // Hold GameMode in waiting for StartMatch()
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - (GetWorld()->GetTimeSeconds() + LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - (GetWorld()->GetTimeSeconds() + LevelStartingTime);
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFrontierPlayerController* PlayerController = Cast<AFrontierPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->OnMatchStateSet(MatchState);
		}

	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

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


void ABlasterGameMode::SMS(FName NewState)
{
	if (HasAuthority()) // Ensure only the server can change match states
	{
		SetMatchState(NewState);
		UE_LOG(LogTemp, Log, TEXT("Match state changed to: %s"), *NewState.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Only the server can change match state."));
	}
}
