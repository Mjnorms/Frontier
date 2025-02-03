// Mjnorms -- 2024


#include "PlayerHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	AddCharacterOverlay();
}

void APlayerHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	// Crosshairs
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsLeft)		DrawCrosshair(HUDPackage.CrosshairsLeft,	ViewportCenter, FVector2D(-SpreadScaled,0.f),  HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsRight)		DrawCrosshair(HUDPackage.CrosshairsRight,	ViewportCenter, FVector2D( SpreadScaled,0.f),  HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsTop)		DrawCrosshair(HUDPackage.CrosshairsTop,		ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		if (HUDPackage.CrosshairsBottom)	DrawCrosshair(HUDPackage.CrosshairsBottom,	ViewportCenter, FVector2D(0.f, SpreadScaled),  HUDPackage.CrosshairColor);
	}
}

void APlayerHUD::DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, FVector2D Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPt(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);
	DrawTexture(
		Texture,
		TextureDrawPt.X,
		TextureDrawPt.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		Color
	);
}
