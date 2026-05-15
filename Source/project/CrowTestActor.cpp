#include "CrowTestActor.h"

#include "CrowBehaviorModelComponent.h"
#include "CrowTraceActor.h"
#include "PlayerProbeComponent.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ACrowTestActor::ACrowTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	VisibleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleMesh"));
	VisibleMesh->SetupAttachment(SceneRoot);
	VisibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisibleMesh->SetGenerateOverlapEvents(false);
	VisibleMesh->SetRelativeScale3D(FVector(0.32f, 0.22f, 0.22f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		VisibleMesh->SetStaticMesh(SphereMesh.Object);
	}

	BehaviorModel = CreateDefaultSubobject<UCrowBehaviorModelComponent>(TEXT("BehaviorModel"));
	TraceActorClass = ACrowTraceActor::StaticClass();
}

void ACrowTestActor::BeginPlay()
{
	Super::BeginPlay();

	ResolvePlayerActor();
	if (PlayerActor)
	{
		PreviousPlayerLocation = PlayerActor->GetActorLocation();
	}
}

void ACrowTestActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ResolvePlayerActor();
	if (!PlayerActor || !BehaviorModel)
	{
		SetVisibleState(ECrowVisibleLoopState::Idle, TEXT("缺少 PlayerActor 或 BehaviorModel，乌鸦保持可见等待配置"));
		return;
	}

	BehaviorModel->BaseDesiredDistance = IdealDistance;
	BehaviorModel->RetreatDistance = MinCatchDistance;
	BehaviorModel->WaitDistance = IdealDistance * 1.45f;

	const FPlayerBehaviorInput PlayerInput = BuildPlayerInput(DeltaSeconds);
	LastBehaviorOutput = BehaviorModel->EvaluateBehavior(PlayerInput);

	ApplyState(DeltaSeconds, PlayerInput);

	PreviousPlayerLocation = PlayerActor->GetActorLocation();
}

void ACrowTestActor::ResolvePlayerActor()
{
	if (!PlayerActor)
	{
		PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);
	}
}

FPlayerBehaviorInput ACrowTestActor::BuildPlayerInput(float DeltaSeconds)
{
	const bool bLookingAtCrow = IsPlayerLookingAtCrow();

	if (UPlayerProbeComponent* Probe = PlayerActor ? PlayerActor->FindComponentByClass<UPlayerProbeComponent>() : nullptr)
	{
		FPlayerBehaviorInput Input = Probe->GetBehaviorInput(this);
		Input.bHasLineOfSight = bLookingAtCrow;
		Input.bIsPursuing = bLookingAtCrow && Input.bMovingTowardCrow;
		Input.DeltaSeconds = DeltaSeconds;
		return Input;
	}

	FPlayerBehaviorInput Input;
	Input.PlayerLocation = PlayerActor->GetActorLocation();
	Input.PlayerVelocity = DeltaSeconds > KINDA_SMALL_NUMBER
		? (Input.PlayerLocation - PreviousPlayerLocation) / DeltaSeconds
		: FVector::ZeroVector;
	Input.DistanceToCrow = FVector::Dist(Input.PlayerLocation, GetActorLocation());
	Input.bHasLineOfSight = bLookingAtCrow;
	Input.DeltaSeconds = DeltaSeconds;

	const float Speed = Input.PlayerVelocity.Size();
	if (Speed > StopSpeedThreshold && Input.DistanceToCrow > KINDA_SMALL_NUMBER)
	{
		const FVector MoveDirection = Input.PlayerVelocity.GetSafeNormal();
		const FVector CrowDirection = (GetActorLocation() - Input.PlayerLocation).GetSafeNormal();
		Input.bMovingTowardCrow = FVector::DotProduct(MoveDirection, CrowDirection) >= 0.35f;
	}

	Input.bIsPursuing = bLookingAtCrow && Input.bMovingTowardCrow;
	Input.PursuitIntensity = FMath::Clamp(Speed / 600.0f, 0.0f, 1.0f);
	return Input;
}

bool ACrowTestActor::IsPlayerLookingAtCrow() const
{
	if (!PlayerActor)
	{
		return false;
	}

	FVector EyeLocation = PlayerActor->GetActorLocation();
	FRotator EyeRotation = PlayerActor->GetActorRotation();
	PlayerActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector ToCrow = (GetActorLocation() - EyeLocation).GetSafeNormal();
	if (ToCrow.IsNearlyZero())
	{
		return true;
	}

	return FVector::DotProduct(EyeRotation.Vector(), ToCrow) >= LookAtDotThreshold;
}

bool ACrowTestActor::IsPlayerStopped(const FPlayerBehaviorInput& Input) const
{
	return Input.PlayerVelocity.Size() <= StopSpeedThreshold;
}

void ACrowTestActor::ApplyState(float DeltaSeconds, const FPlayerBehaviorInput& Input)
{
	const FVector PlayerLocation = Input.PlayerLocation;
	FVector AwayFromPlayer = GetActorLocation() - PlayerLocation;
	AwayFromPlayer.Z = 0.0f;
	if (AwayFromPlayer.IsNearlyZero())
	{
		AwayFromPlayer = -GetActorForwardVector();
		AwayFromPlayer.Z = 0.0f;
	}
	AwayFromPlayer.Normalize();

	const FVector IdealPoint(PlayerLocation.X + AwayFromPlayer.X * IdealDistance, PlayerLocation.Y + AwayFromPlayer.Y * IdealDistance, GetActorLocation().Z);
	const bool bLookingAtCrow = Input.bHasLineOfSight;
	const bool bStopped = IsPlayerStopped(Input);
	bool bChaseMoment = false;

	if (!bLookingAtCrow || !bStopped)
	{
		bCanStartPerch = true;
	}

	if (Input.DistanceToCrow <= MinCatchDistance)
	{
		const FVector EscapeTarget(PlayerLocation.X + AwayFromPlayer.X * (IdealDistance + 120.0f), PlayerLocation.Y + AwayFromPlayer.Y * (IdealDistance + 120.0f), GetActorLocation().Z);
		MoveTowardPoint(EscapeTarget, EscapeSpeed, DeltaSeconds);

		const float SafetyDistance = MinCatchDistance + 40.0f;
		if (FVector::Dist2D(PlayerLocation, GetActorLocation()) < SafetyDistance)
		{
			SetActorLocation(FVector(PlayerLocation.X + AwayFromPlayer.X * SafetyDistance, PlayerLocation.Y + AwayFromPlayer.Y * SafetyDistance, GetActorLocation().Z));
		}

		FacePlayer();
		SetVisibleState(ECrowVisibleLoopState::Relocate, TEXT("玩家进入最小捕捉距离，立即快速后撤，禁止被抓住"));
		bChaseMoment = true;
		PerchTimeRemaining = 0.0f;
	}
	else if ((PerchTimeRemaining > 0.0f && bLookingAtCrow && bStopped) || (bLookingAtCrow && bStopped && PerchDuration > 0.0f && bCanStartPerch))
	{
		if (PerchTimeRemaining <= 0.0f)
		{
			PerchTimeRemaining = PerchDuration;
			bCanStartPerch = false;
		}

		PerchTimeRemaining = FMath::Max(PerchTimeRemaining - DeltaSeconds, 0.0f);
		FacePlayer();
		SetVisibleState(ECrowVisibleLoopState::Perch, TEXT("玩家停止且仍看着乌鸦，乌鸦短暂停留并回看玩家"));
	}
	else if (bLookingAtCrow && Input.bMovingTowardCrow)
	{
		MoveTowardPoint(IdealPoint, MoveSpeed, DeltaSeconds);
		FacePlayer();
		SetVisibleState(ECrowVisibleLoopState::MaintainDistance, TEXT("玩家看向并靠近，乌鸦沿远离玩家方向移动，维持理想距离"));
		bChaseMoment = true;
		PerchTimeRemaining = 0.0f;
	}
	else if (Input.DistanceToCrow > IdealDistance * 1.15f || LastBehaviorOutput.State == ECrowState::Waiting)
	{
		MoveTowardPoint(IdealPoint, MoveSpeed, DeltaSeconds);
		FacePlayer();
		SetVisibleState(ECrowVisibleLoopState::Attract, TEXT("玩家距离较远，乌鸦保持可见并靠近到可追逐范围"));
		PerchTimeRemaining = 0.0f;
	}
	else
	{
		FacePlayer();
		SetVisibleState(ECrowVisibleLoopState::Idle, TEXT("玩家未形成追逐压力，乌鸦保持可见等待"));
		PerchTimeRemaining = 0.0f;
	}

	if (bChaseMoment && TraceInterval > 0.0f)
	{
		TraceTimeAccumulator += DeltaSeconds;
		if (TraceTimeAccumulator >= TraceInterval)
		{
			TraceTimeAccumulator = 0.0f;
			SpawnTrace();
		}
	}
	else
	{
		TraceTimeAccumulator = FMath::Min(TraceTimeAccumulator, TraceInterval);
	}
}

void ACrowTestActor::MoveTowardPoint(const FVector& TargetLocation, float Speed, float DeltaSeconds)
{
	if (Speed <= 0.0f || DeltaSeconds <= 0.0f)
	{
		return;
	}

	const FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), TargetLocation, DeltaSeconds, Speed);
	SetActorLocation(NewLocation);
}

void ACrowTestActor::FacePlayer()
{
	if (!PlayerActor)
	{
		return;
	}

	const FVector ToPlayer = PlayerActor->GetActorLocation() - GetActorLocation();
	if (ToPlayer.IsNearlyZero())
	{
		return;
	}

	FRotator LookRotation = ToPlayer.Rotation();
	LookRotation.Pitch = 0.0f;
	LookRotation.Roll = 0.0f;
	SetActorRotation(LookRotation);
}

void ACrowTestActor::SpawnTrace()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UClass* ClassToSpawn = TraceActorClass ? TraceActorClass.Get() : ACrowTraceActor::StaticClass();
	if (!ClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLocation = GetActorLocation() - GetActorForwardVector() * 35.0f;
	World->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, GetActorRotation(), SpawnParameters);
}

void ACrowTestActor::SetVisibleState(ECrowVisibleLoopState NewState, const FString& NewDebugReason)
{
	CurrentState = NewState;
	DebugReason = NewDebugReason;
}
