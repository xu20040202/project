#include "CrowBehaviorModelComponent.h"

UCrowBehaviorModelComponent::UCrowBehaviorModelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FCrowBehaviorOutput UCrowBehaviorModelComponent::EvaluateBehavior(const FPlayerBehaviorInput& PlayerInput)
{
	UpdateRelationScore(PlayerInput);
	CurrentState = ChooseState(PlayerInput);

	FCrowBehaviorOutput Output;
	Output.State = CurrentState;

	const float ClampedTease = FMath::Clamp(RelationScore.Tease, 0.0f, 1.0f);
	const float ClampedAlertness = FMath::Clamp(RelationScore.Alertness, 0.0f, 1.0f);
	const float ClampedCuriosity = FMath::Clamp(RelationScore.Curiosity, 0.0f, 1.0f);

	Output.DesiredDistance = BaseDesiredDistance + (ClampedAlertness * 250.0f) - (RelationScore.Familiarity * 120.0f);
	Output.DesiredDistance = FMath::Max(Output.DesiredDistance, RetreatDistance);
	Output.DelaySeconds = FMath::Lerp(MaxDelaySeconds, MinDelaySeconds, ClampedAlertness);
	Output.LookAtPlayerWeight = ClampedCuriosity;
	Output.bShouldRetreat = CurrentState == ECrowState::Retreating || CurrentState == ECrowState::Teasing;
	Output.bShouldWait = CurrentState == ECrowState::Waiting || CurrentState == ECrowState::Watching;

	if (CurrentState == ECrowState::Teasing)
	{
		Output.DelaySeconds = FMath::Lerp(Output.DelaySeconds, MinDelaySeconds, ClampedTease);
	}

	return Output;
}

void UCrowBehaviorModelComponent::ResetRelation()
{
	RelationScore = FCrowRelationScore();
	CurrentState = ECrowState::Idle;
}

FCrowRelationScore UCrowBehaviorModelComponent::GetRelationScore() const
{
	return RelationScore;
}

void UCrowBehaviorModelComponent::UpdateRelationScore(const FPlayerBehaviorInput& PlayerInput)
{
	const float DeltaSeconds = FMath::Max(PlayerInput.DeltaSeconds, 0.0f);
	const float PursuitAmount = PlayerInput.bIsPursuing ? PlayerInput.PursuitIntensity : PlayerInput.PursuitIntensity * 0.35f;
	const float DirectionBonus = PlayerInput.bMovingTowardCrow ? 1.0f : 0.0f;
	const float SightBonus = PlayerInput.bHasLineOfSight ? 1.0f : -0.5f;

	RelationScore.Alertness += (PursuitAmount + DirectionBonus) * DeltaSeconds * 0.25f;
	RelationScore.Curiosity += SightBonus * DeltaSeconds * 0.08f;
	RelationScore.Tease += (PlayerInput.bMovingTowardCrow ? 0.12f : -0.05f) * DeltaSeconds;

	if (PlayerInput.DistanceToCrow > WaitDistance)
	{
		RelationScore.Familiarity += DeltaSeconds * 0.03f;
	}
	else if (PlayerInput.DistanceToCrow < RetreatDistance)
	{
		RelationScore.Familiarity -= DeltaSeconds * 0.08f;
	}

	RelationScore.Familiarity = FMath::Clamp(RelationScore.Familiarity, -1.0f, 1.0f);
	RelationScore.Curiosity = FMath::Clamp(RelationScore.Curiosity, 0.0f, 1.0f);
	RelationScore.Alertness = FMath::Clamp(RelationScore.Alertness, 0.0f, 1.0f);
	RelationScore.Tease = FMath::Clamp(RelationScore.Tease, 0.0f, 1.0f);
}

ECrowState UCrowBehaviorModelComponent::ChooseState(const FPlayerBehaviorInput& PlayerInput) const
{
	if (!PlayerInput.bHasLineOfSight && PlayerInput.DistanceToCrow > WaitDistance)
	{
		return ECrowState::LostInterest;
	}

	if (PlayerInput.DistanceToCrow <= RetreatDistance)
	{
		return ECrowState::Retreating;
	}

	if (PlayerInput.bMovingTowardCrow && PlayerInput.PursuitIntensity > 0.25f)
	{
		return ECrowState::Teasing;
	}

	if (PlayerInput.DistanceToCrow >= WaitDistance)
	{
		return ECrowState::Waiting;
	}

	if (PlayerInput.bHasLineOfSight)
	{
		return ECrowState::Watching;
	}

	return ECrowState::Idle;
}
