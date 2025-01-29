// Mjnorms -- 2024


#include "PlayerHUD.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	// Crosshairs
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		if (HUDPackage.CrosshairsCenter)	DrawCrosshair(HUDPackage.CrosshairsCenter,	ViewportCenter);
		if (HUDPackage.CrosshairsLeft)		DrawCrosshair(HUDPackage.CrosshairsLeft,	ViewportCenter);
		if (HUDPackage.CrosshairsRight)		DrawCrosshair(HUDPackage.CrosshairsRight,	ViewportCenter);
		if (HUDPackage.CrosshairsBottom)	DrawCrosshair(HUDPackage.CrosshairsBottom,	ViewportCenter);
		if (HUDPackage.CrosshairsTop)		DrawCrosshair(HUDPackage.CrosshairsTop,		ViewportCenter);
	}
}

void APlayerHUD::DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPt(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f)
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
		FLinearColor::White
	);
}
