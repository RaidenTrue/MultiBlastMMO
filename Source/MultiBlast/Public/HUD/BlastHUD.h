// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlastHUD.generated.h"

class UTexture2D;
class UCharacterOverlay;
class UAnnouncment;
class UUserWidget;
class APlayerController;
class UElimAnnouncement;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	UTexture2D* CrosshairsCenter;

	UTexture2D* CrosshairsLeft;

	UTexture2D* CrosshairsRight;

	UTexture2D* CrosshairsTop;

	UTexture2D* CrosshairsBottom;

	float CrosshairSpread;

	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class MULTIBLAST_API ABlastHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	void AddAnnouncement();

	void AddElimAnnouncement(FString Attacker, FString Victim);

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncment* Announcement;

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	
};
