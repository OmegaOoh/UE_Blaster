// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapons/WeaponTypes.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnoucementCountdown(float CountDownTime);
	void SetHUDWeaponName(EWeaponType WeaponName);
	void SetHUDGrenade(int32 Grenade);
	virtual void Tick(float DeltaSeconds) override;

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override; //Sync with ServerClock ASAP
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCoolDown();

	float SingleTripTime;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker,APlayerState* Victim);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	void CheckPing(float DeltaSeconds);
	virtual void SetupInputComponent() override;

	/*
	 * Sync Server and Client Time
	 */

	//Request the Current ServerTime, passing in ClientTime when the Request was Sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports The Currents Server time to the client in response to ServerRequestServerTime
	UFUNCTION(Server, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; //Differents between Client and ServerTime

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateofMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();

	void ShowReturntoMenu();

	UFUNCTION(Client,Reliable)
	void ClientElimAnnoucement(APlayerState* Attacker, APlayerState* Victim);

private:
	/*
	 * Return to Main Menu
	 */
	UPROPERTY(EditAnywhere,Category = "HUD")
	TSubclassOf<class UUserWidget > ReturnToMenuWidget;
	UPROPERTY()
	class UReturnToMenu* ReturnToMenu;
	bool bReturnToMenuOpen = false;


	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterGamemode* BlasterGamemode;

	float LevelStartingTime = 0.f;
	float WarmupTime = 0.f;
	float MatchTime = 0.f;
	float CoolDownTime = 0.f;
	uint32 CountDownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	float HUDHealth = 100.f;
	float HUDMaxHealth = 100.f;
	bool bInitializeShield = false;
	float HUDShield = 0.f;
	float HUDMaxShield = 100.f;
	bool bInitializeScore = false;
	float HUDScore;
	bool bInitializeDefeats = false;
	int32 HUDDefeats;
	bool bInitializeGrenades = false;
	int32 HUDGrenades;
	bool bInitializeCarriedAmmo = false;
	int32 HUDCarriedAmmo;
	bool bInitializeWeaponAmmo = false;
	int32 HUDWeaponAmmo;
	bool bInitializeWeaponName = false;
	EWeaponType HUDWeaponName;

	float HighPingRunningTime = 0.f;
	float HighPingAnimRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPickFrequency = 15.f ;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingTreshold = 100.f;


};


