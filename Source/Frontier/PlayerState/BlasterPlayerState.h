// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FRONTIER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
	

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void AddToScore(float ScoreAmt);
	void AddToDeaths(int32 DeathsAmt);
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Deaths();

private:
	class APlayerCharacter* Character = nullptr;
	class AFrontierPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths = 0;
};
