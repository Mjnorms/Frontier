// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern FRONTIER_API const FName Cooldown; // Match duration reached, display post match UI & start cooldown timer
}


/**
 * 
 */
UCLASS()
class FRONTIER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	void Tick(float DeltaTime) override;


	virtual void PlayerEliminated(class APlayerCharacter* ElimdCharacter, class AFrontierPlayerController* VictimController, AFrontierPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimdCharacter, AController* ElimdController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 15.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	float CountdownTime = 0.f;
};
