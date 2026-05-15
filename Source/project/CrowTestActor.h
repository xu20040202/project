#pragma once

#include "CoreMinimal.h"
#include "CrowRelationTypes.h"
#include "GameFramework/Actor.h"
#include "CrowTestActor.generated.h"

class ACrowTraceActor;
class UCrowBehaviorModelComponent;
class UPlayerProbeComponent;
class USceneComponent;
class UStaticMeshComponent;

// 第二阶段可见行为闭环状态：描述体验表现，不把乌鸦升级成敌人 AI。
UENUM(BlueprintType)
enum class ECrowVisibleLoopState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attract UMETA(DisplayName = "Attract"),
	MaintainDistance UMETA(DisplayName = "MaintainDistance"),
	Relocate UMETA(DisplayName = "Relocate"),
	Perch UMETA(DisplayName = "Perch")
};

// 最小可见乌鸦测试 Actor：玩家能看见和追逐，但永远差一点抓不到。
UCLASS(BlueprintType, Blueprintable)
class PROJECT_API ACrowTestActor : public AActor
{
	GENERATED_BODY()

public:
	ACrowTestActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 可见占位体，默认使用 Engine 基础球体，蓝图子类可替换。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Visual")
	UStaticMeshComponent* VisibleMesh;

	// 复用现有关系模型，每 Tick 输入玩家行为并读取模型输出。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Behavior")
	UCrowBehaviorModelComponent* BehaviorModel;

	// 场景或蓝图中指定的玩家；不指定时自动取 0 号 PlayerPawn。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior")
	AActor* PlayerActor = nullptr;

	// 乌鸦希望与玩家保持的“差一点”距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "1.0"))
	float IdealDistance = 600.0f;

	// 玩家进入该距离时立刻触发后撤，禁止被抓住。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "1.0"))
	float MinCatchDistance = 180.0f;

	// 普通吸引和维持距离移动速度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "0.0"))
	float MoveSpeed = 260.0f;

	// 过近时快速后撤速度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "0.0"))
	float EscapeSpeed = 900.0f;

	// 玩家停止且仍注视乌鸦时，乌鸦停留观察的时长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "0.0"))
	float PerchDuration = 1.5f;

	// 追逐过程中生成痕迹占位物的间隔。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Tuning", meta = (ClampMin = "0.05"))
	float TraceInterval = 0.8f;

	// 生成的痕迹 Actor 类型，默认使用 ACrowTraceActor。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Trace")
	TSubclassOf<ACrowTraceActor> TraceActorClass;

	// 当前可见行为状态，给策划蓝图和 Details 面板观察。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Debug")
	ECrowVisibleLoopState CurrentState = ECrowVisibleLoopState::Idle;

	// 当前状态的中文原因，便于运行时理解为什么这样动。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Debug")
	FString DebugReason = TEXT("尚未开始");

	// 最近一次现有行为模型的输出，方便对照关系模型状态。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Debug")
	FCrowBehaviorOutput LastBehaviorOutput;

protected:
	// 玩家视线点乘阈值，越高表示必须看得越准。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Advanced", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float LookAtDotThreshold = 0.72f;

	// 低于该速度视为玩家停止。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Advanced", meta = (ClampMin = "0.0"))
	float StopSpeedThreshold = 20.0f;

private:
	void ResolvePlayerActor();
	FPlayerBehaviorInput BuildPlayerInput(float DeltaSeconds);
	bool IsPlayerLookingAtCrow() const;
	bool IsPlayerStopped(const FPlayerBehaviorInput& Input) const;
	void ApplyState(float DeltaSeconds, const FPlayerBehaviorInput& Input);
	void MoveTowardPoint(const FVector& TargetLocation, float Speed, float DeltaSeconds);
	void FacePlayer();
	void SpawnTrace();
	void SetVisibleState(ECrowVisibleLoopState NewState, const FString& NewDebugReason);

	FVector PreviousPlayerLocation = FVector::ZeroVector;
	float PerchTimeRemaining = 0.0f;
	float TraceTimeAccumulator = 0.0f;
	bool bCanStartPerch = true;
};
