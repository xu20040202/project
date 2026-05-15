#include "PlayerProbeComponent.h"
#include "GameFramework/Actor.h"

UPlayerProbeComponent::UPlayerProbeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerProbeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		PreviousLocation = Owner->GetActorLocation();
	}
}

void UPlayerProbeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || DeltaTime <= KINDA_SMALL_NUMBER)
	{
		LastDeltaSeconds = DeltaTime;
		return;
	}

	const FVector CurrentLocation = Owner->GetActorLocation();
	EstimatedVelocity = (CurrentLocation - PreviousLocation) / DeltaTime;
	PreviousLocation = CurrentLocation;
	LastDeltaSeconds = DeltaTime;
}

FPlayerBehaviorInput UPlayerProbeComponent::GetBehaviorInput(AActor* CrowActor) const
{
	FPlayerBehaviorInput Input;

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Input;
	}

	Input.PlayerLocation = Owner->GetActorLocation();
	Input.PlayerVelocity = EstimatedVelocity;
	Input.bHasLineOfSight = bHasLineOfSight;
	Input.bIsPursuing = bIsPursuing;
	Input.DeltaSeconds = LastDeltaSeconds;

	if (CrowActor)
	{
		const FVector ToCrow = CrowActor->GetActorLocation() - Input.PlayerLocation;
		Input.DistanceToCrow = ToCrow.Size();

		const float Speed = EstimatedVelocity.Size();
		if (Speed > MovementThreshold && Input.DistanceToCrow > KINDA_SMALL_NUMBER)
		{
			const FVector MoveDirection = EstimatedVelocity.GetSafeNormal();
			const FVector CrowDirection = ToCrow.GetSafeNormal();
			Input.bMovingTowardCrow = FVector::DotProduct(MoveDirection, CrowDirection) >= TowardDotThreshold;
		}
	}

	const float ReferenceSpeed = FMath::Max(PursuitSpeedReference, 1.0f);
	Input.PursuitIntensity = FMath::Clamp(EstimatedVelocity.Size() / ReferenceSpeed, 0.0f, 1.0f);

	if (!Input.bIsPursuing)
	{
		Input.PursuitIntensity *= 0.35f;
	}

	return Input;
}

void UPlayerProbeComponent::SetHasLineOfSight(bool bNewHasLineOfSight)
{
	bHasLineOfSight = bNewHasLineOfSight;
}

void UPlayerProbeComponent::SetIsPursuing(bool bNewIsPursuing)
{
	bIsPursuing = bNewIsPursuing;
}

FVector UPlayerProbeComponent::GetEstimatedVelocity() const
{
	return EstimatedVelocity;
}
