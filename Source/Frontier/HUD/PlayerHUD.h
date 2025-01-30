// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
	FHUDPackage()
	{
		CrosshairsCenter = nullptr;
		CrosshairsLeft = nullptr;
		CrosshairsRight = nullptr;
		CrosshairsTop = nullptr;
		CrosshairsBottom = nullptr;
	}

	class UTexture2D* CrosshairsCenter;
	class UTexture2D* CrosshairsLeft;
	class UTexture2D* CrosshairsRight;
	class UTexture2D* CrosshairsTop;
	class UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
};

/**
 * 
 */
UCLASS()
class FRONTIER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, FVector2D Spread);
	
private:
	FHUDPackage HUDPackage;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
