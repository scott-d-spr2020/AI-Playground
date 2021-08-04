// Fill out your copyright notice in the Description page of Project Settings.


#include "AIFriendController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/FloatingPawnMovement.h"

AAIFriendController::AAIFriendController()
{
}

void AAIFriendController::CheckRange()
{
	FVector FriendPosition{ Pawn->GetActorLocation() };
	FVector OwnerPosition{ Owner->GetActorLocation() };

	float DistSquared2D{ FVector::DistSquared2D(FriendPosition, OwnerPosition) };

	ShouldOrbit = false;
	ShouldMoveToRing = true;

	if (FMath::IsNearlyEqual(DistSquared2D, GoalRadius * GoalRadius, OrbitThreshold * OrbitThreshold))
	{
		ShouldOrbit = true;
		if (FMath::IsNearlyEqual(DistSquared2D, GoalRadius * GoalRadius, GoalThreshold * GoalThreshold))
		{
			ShouldMoveToRing = false; //if this is false, apply linear deceleration
		}
	}

	ShouldMoveToHeight = !FMath::IsNearlyEqual(FriendPosition.Z, OwnerPosition.Z + HeightRelativeToPlayer, HeightThreshold);

}

void AAIFriendController::MoveToRing(float DeltaTime)
{
	//find closest point on ring
	FVector OwnerPosition{ Owner->GetActorLocation() };
	FVector RelativeFriendPosition{ Pawn->GetActorLocation() - OwnerPosition };
	FRay OwnerToFriendDir(OwnerPosition, RelativeFriendPosition);
	FVector RingPoint{ OwnerToFriendDir.PointAt(GoalRadius) };

	//find direction to ring point
	FVector FriendToRing{ RingPoint - Pawn->GetActorLocation() };
	FriendToRing.Z = 0;
	FriendToRing.Normalize();

	FVector CurrentFriendVelocity{ Pawn->GetVelocity() };
	CurrentFriendVelocity.Z = 0;

	FVector ProjectedVelocity{ CurrentFriendVelocity.ProjectOnTo(FriendToRing) };
	FVector TangentialVelocity{ CurrentFriendVelocity - ProjectedVelocity };

	FVector NewVelocity;

	if (!ShouldMoveToRing)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(FMath::Max(0.0f, CurrentFriendVelocity.Size() - DecelerationTowardsRing * DeltaTime));
	}
	else
	{
		float FriendToRingMag = FriendToRing.Size();
		NewVelocity = CurrentFriendVelocity + (FriendToRing / FriendToRingMag) * AccelerationTowardsRing * FMath::Pow(FriendToRingMag, 1.5f) * DeltaTime;
	}
	
	NewVelocity.Z = PawnMoveComp->Velocity.Z; //MoveToHeight() will deal with Z
	PawnMoveComp->Velocity = NewVelocity;
}

void AAIFriendController::RepelFromOwner(float DeltaTime)
{
	FVector OwnerPosition{ Owner->GetActorLocation() };
	FVector RelativeFriendPosition{ Pawn->GetActorLocation() - OwnerPosition };
	//too close to player
	if (RelativeFriendPosition.SizeSquared2D() < (GoalRadius - 2 * OrbitThreshold) * (GoalRadius - 2 * OrbitThreshold))
	{
		FRay OwnerToFriendDir(OwnerPosition, RelativeFriendPosition);
		FVector RingPoint{ OwnerToFriendDir.PointAt(GoalRadius) };

		//find direction to ring point
		FVector FriendToRing{ RingPoint - Pawn->GetActorLocation() };
		FriendToRing.Z = 0;
		FriendToRing.Normalize();

		PawnMoveComp->Velocity += FriendToRing * 150 * DeltaTime;
	}
}

void AAIFriendController::Orbit(float DeltaTime)
{

}

void AAIFriendController::MoveToHeight(float DeltaTime)
{
	if (!ShouldMoveToHeight)
	{
		if (PawnMoveComp->Velocity.Z > 0)
		{
			PawnMoveComp->Velocity.Z = FMath::Max(0.0f, PawnMoveComp->Velocity.Z - DecelerationTowardsHeight * DeltaTime);
		}
		else if (PawnMoveComp->Velocity.Z < 0)
		{
			PawnMoveComp->Velocity.Z = FMath::Min(0.0f, PawnMoveComp->Velocity.Z + DecelerationTowardsHeight * DeltaTime);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Nope %f"), PawnMoveComp->Velocity.Z));
	}
	else
	{
		float CurrentHeight = Pawn->GetActorLocation().Z;
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("H1 %f"), CurrentHeight));
		float DesiredHeight = Owner->GetActorLocation().Z + HeightRelativeToPlayer;
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("H2 %f"), DesiredHeight));
		float RelativeHeight = CurrentHeight - DesiredHeight;
		int Direction = RelativeHeight < 0 ? 1 : -1;
		PawnMoveComp->Velocity.Z += Direction * AccelerationTowardsHeight * FMath::Pow(FMath::Abs(RelativeHeight / HeightThreshold), 1.5f) * DeltaTime;
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("%f"), PawnMoveComp->Velocity.Z));
	}
}

int Next = 20;
FVector RandAcceleration{};
void AAIFriendController::Tick(float DeltaTime)
{
	Pawn = GetPawn();
	//determine whether we can orbit and what our target destination is
	CheckRange();
	//RepelFromOwner(DeltaTime);
	MoveToRing(DeltaTime);
	MoveToHeight(DeltaTime);

	//Next--;
	//if (Next <= 0)
	//{
	//	RandAcceleration = FMath::VRand() * 1000.0f;
	//	Next = 20;
	//}
	//PawnMoveComp->Velocity += RandAcceleration * DeltaTime;

	Pawn = nullptr;
}

void AAIFriendController::BeginPlay()
{
	Super::BeginPlay();
	Owner = (AAIDemoCharacter*)UGameplayStatics::GetActorOfClass(GetWorld(), AAIDemoCharacter::StaticClass());
	if (Owner && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Owner exists"));
	}
	PawnMoveComp = (UMovementComponent*)GetPawn()->GetComponentByClass(UFloatingPawnMovement::StaticClass());
	if (PawnMoveComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Movement exists"));
	}
}