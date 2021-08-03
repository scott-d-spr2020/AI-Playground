// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIDemoCharacter.h"
#include "AIFriendController.generated.h"

/**
 * 
 */
UCLASS()
class AIDEMO_API AAIFriendController : public AAIController
{
	GENERATED_BODY()
public:
	AAIFriendController();
	virtual void Tick(float DeltaTime);
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float GoalRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float GoalThreshold; //within GoalRadius +/- GoalThreshold, only orbit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float OrbitThreshold; //within GoalRadius +/- OrbitThreshold, orbit and move

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float HeightRelativeToPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float HeightThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float AccelerationTowardsHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float DecelerationTowardsHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float OrbitTangentialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float AccelerationTowardsRing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float DecelerationTowardsRing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float OrbitalTangentialAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Friend")
		float OrbitalTangentialDeceleration;
private:
	AAIDemoCharacter* Owner;
	UMovementComponent* PawnMoveComp;
	APawn* Pawn; //assign at start of tick, de-assign at end of tick

	void CheckRange();
	void RepelFromOwner(float DeltaTime);
	void MoveToRing(float DeltaTime);
	void MoveToHeight(float DeltaTime);
	void Orbit(float DeltaTime);

	bool ShouldOrbit;
	bool ShouldMoveToRing;
	bool ShouldMoveToHeight;
	float RotationAngle;

	FVector TargetPosition;

};
