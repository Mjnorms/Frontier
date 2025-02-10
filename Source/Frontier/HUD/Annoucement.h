// Mjnorms -- 2024

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Annoucement.generated.h"

/**
 * 
 */
UCLASS()
class FRONTIER_API UAnnoucement : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InfoText = nullptr;
};
