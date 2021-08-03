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
			ShouldMoveToRing = false;
		}
	}
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

	FVector ProjectedVelocity{ CurrentFriendVelocity.ProjectOnTo(FriendToRing) };
	FVector TangentialVelocity{ CurrentFriendVelocity - ProjectedVelocity };

	FVector NewVelocity;

	if (!ShouldMoveToRing) //no velocity
	{
		NewVelocity = FVector::ZeroVector;
	}
	else if (!ShouldOrbit)
	{
		//strip away the tangential velocity
		NewVelocity = TangentialVelocity * 0.95f + ProjectedVelocity;
		//accelerate towards ring
		NewVelocity += FriendToRing.GetSafeNormal() * AccelerationTowardsRing * DeltaTime;
	}
	else //leave tangential velocity alone, Orbit() will deal with it
	{
		NewVelocity = CurrentFriendVelocity;
		FVector TempVelocity{ NewVelocity - FriendToRing.GetSafeNormal() * DecelerationTowardsRing * DeltaTime };
		if (FVector::DotProduct(NewVelocity, TempVelocity) < 0)
		{
			NewVelocity = FVector::ZeroVector;
		}
		else
		{
			NewVelocity = TempVelocity;
		}
	}

	NewVelocity.Z = CurrentFriendVelocity.Z; //MoveToHeight() will deal with Z
	PawnMoveComp->Velocity = NewVelocity;
}

void AAIFriendController::RepelFromOwner(float DeltaTime)
{
	FVector OwnerPosition{ Owner->GetActorLocation() };
	FVector RelativeFriendPosition{ Pawn->GetActorLocation() - OwnerPosition };
	//too close to player
	if (RelativeFriendPosition.SizeSquared2D() < (GoalRadius - OrbitThreshold) * (GoalRadius - OrbitThreshold))
	{
		FRay OwnerToFriendDir(OwnerPosition, RelativeFriendPosition);
		FVector RingPoint{ OwnerToFriendDir.PointAt(GoalRadius) };

		//find direction to ring point
		FVector FriendToRing{ RingPoint - Pawn->GetActorLocation() };
		FriendToRing.Z = 0;
		FriendToRing.Normalize();

		PawnMoveComp->Velocity += FriendToRing * 1500 * DeltaTime;
	}
}

void AAIFriendController::Orbit(float DeltaTime)
{

}

void AAIFriendController::MoveToHeight(float DeltaTime)
{

}

void AAIFriendController::Tick(float DeltaTime)
{
	Pawn = GetPawn();
	//determine whether we can orbit and what our target destination is
	CheckRange();
	RepelFromOwner(DeltaTime);
	MoveToRing(DeltaTime);
	MoveToHeight(DeltaTime);
	//	FVector Position = GetPawn()->GetActorLocation();
	//	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Pawn %f %f %f"), Position.X, Position.Y, Position.Z));
	//	FVector OwnerPosition = Owner->GetActorLocation();
	//	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Owner %f %f %f"), OwnerPosition.X, OwnerPosition.Y, OwnerPosition.Z));
	//	float DistSquared2D = FVector::DistSquared2D(Position, OwnerPosition);
	//	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Dist %f"), DistSquared2D));
	//	if (FMath::IsNearlyEqual(DistSquared2D, GoalRadius))
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("A"));
	//		//do nothing
	//	}
	//	else
	//	{
	//		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("B"));
	//		//normalized direction from owner to position
	//		FVector OwnerToFriend = Position - OwnerPosition;
	//		OwnerToFriend.Normalize();
	//		//FRay Ray(OwnerPosition, OwnerToFriend, true);
	//		//FVector DesiredPos = Ray.PointAt(GoalRadius);
	//		//GetPawn()->SetActorLocation(FVector(DesiredPos.X, DesiredPos.Y, OwnerPosition.Z + HeightRelativeToPlayer));
	//		if (GoalRadius * GoalRadius * 0.95f < DistSquared2D && GoalRadius * GoalRadius * 1.05f > DistSquared2D)
	//		{
	//
	//			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("C %d"), DistSquared2D));
	//			FVector HorizVelocity = GetPawn()->GetVelocity();
	//			FVector VelocityTowardsOwner = HorizVelocity.ProjectOnToNormal(-OwnerToFriend);
	//			FVector NewVelocity = (HorizVelocity - VelocityTowardsOwner) * 0.95f + VelocityTowardsOwner; //temporary
	//			NewVelocity -= (OwnerPosition - Position).GetSafeNormal() * 1500.0f * DeltaTime;
	//			NewVelocity.Z = PawnMoveComp->Velocity.Z;
	//			PawnMoveComp->Velocity = NewVelocity;
	//		}
	//		else
	//		{
	//			GEngine->AddOnScreenDebugMessage(
	//				-1, 1.0f, FColor::Yellow,
	//				FString::Printf(TEXT("OldVel %f %f %f"),
	//					PawnMoveComp->Velocity.X,
	//					PawnMoveComp->Velocity.Y,
	//					PawnMoveComp->Velocity.Z));
	//			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("D"));
	//			FVector HorizVelocity = GetPawn()->GetVelocity();
	//			HorizVelocity.Z = 0;
	//			GEngine->AddOnScreenDebugMessage(
	//				-1, 1.0f, FColor::Yellow, 
	//				FString::Printf(TEXT("HorizVel %f %f %f"),
	//					HorizVelocity.X,
	//					HorizVelocity.Y,
	//					HorizVelocity.Z));
	//			FVector VelocityTowardsOwner = HorizVelocity.ProjectOnToNormal(-OwnerToFriend);
	//			VelocityTowardsOwner.Z = 0;
	//			//GEngine->AddOnScreenDebugMessage(
	//			//	-1, 1.0f, FColor::Yellow,
	//			//	FString::Printf(TEXT("TowardOwner %f %f %f"),
	//			//		VelocityTowardsOwner.X,
	//			//		VelocityTowardsOwner.Y,
	//			//		VelocityTowardsOwner.Z));
	//			FVector NewVelocity = (HorizVelocity - VelocityTowardsOwner) * 0.95f + VelocityTowardsOwner;
	//			NewVelocity += (OwnerPosition - Position).GetSafeNormal() * 1500.0f * DeltaTime;
	//			GEngine->AddOnScreenDebugMessage(
	//				-1, 1.0f, FColor::Yellow,
	//				FString::Printf(TEXT("NewVel %f %f %f"),
	//					NewVelocity.X,
	//					NewVelocity.Y,
	//					NewVelocity.Z));
	//			NewVelocity.Z = PawnMoveComp->Velocity.Z;
	//			PawnMoveComp->Velocity = NewVelocity;
	//			//increase velocity
	//		}
	//		Position = GetPawn()->GetActorLocation();
	//		OwnerPosition = Owner->GetActorLocation();
	//		GetPawn()->SetActorLocation(FVector(Position.X, Position.Y, OwnerPosition.Z + HeightRelativeToPlayer));

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