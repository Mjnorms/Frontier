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
		CrosshairSpread = 0.f;
		CrosshairColor = FLinearColor::White;
	}

	class UTexture2D* CrosshairsCenter;
	class UTexture2D* CrosshairsLeft;
	class UTexture2D* CrosshairsRight;
	class UTexture2D* CrosshairsTop;
	class UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class FRONTIER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	class UCharacterOverlay* CharacterOverlay = nullptr;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	void AddCharacterOverlay();

	class UAnnoucement* AnnouncementOverlay = nullptr;
	UPROPERTY(EditAnywhere, Category = "Annoucement")
	TSubclassOf<class UUserWidget> AnnoucementClass;
	void AddAnnoucement();

protected:
	virtual void BeginPlay() override;
private:
	FHUDPackage HUDPackage;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float CrosshairSpreadMax = 16.f;


public:


	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
private:
	void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, FVector2D Spread, FLinearColor Color);
	
};
