#pragma once

#include "CoreMinimal.h"
#include "CrowRelationTypes.generated.h"

// 乌鸦当前的高层行为状态，只描述关系阶段，不绑定具体 AI 或导航实现。
UENUM(BlueprintType)
enum class ECrowState : uint8
{
	Idle UMETA(DisplayName = "空闲"),
	Watching UMETA(DisplayName = "观察玩家"),
	Teasing UMETA(DisplayName = "保持距离"),
	Retreating UMETA(DisplayName = "后撤"),
	Waiting UMETA(DisplayName = "等待"),
	LostInterest UMETA(DisplayName = "失去兴趣")
};

// 玩家与乌鸦之间的关系分数，用于表达乌鸦对追逐行为的反应倾向。
USTRUCT(BlueprintType)
struct PROJECT_API FCrowRelationScore
{
	GENERATED_BODY()

public:
	// 亲近度越高，乌鸦越愿意停留在玩家附近。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Relation")
	float Familiarity = 0.0f;

	// 好奇心越高，乌鸦越可能观察或等待玩家接近。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Relation")
	float Curiosity = 0.5f;

	// 警觉度越高，乌鸦越倾向于拉开距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Relation")
	float Alertness = 0.5f;

	// 挑逗感越高，乌鸦越倾向于保持“差一点追上”的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Relation")
	float Tease = 0.5f;
};

// 玩家行为输入快照，由探测组件提供给乌鸦行为模型。
USTRUCT(BlueprintType)
struct PROJECT_API FPlayerBehaviorInput
{
	GENERATED_BODY()

public:
	// 玩家当前位置。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	FVector PlayerLocation = FVector::ZeroVector;

	// 玩家当前速度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	FVector PlayerVelocity = FVector::ZeroVector;

	// 玩家与乌鸦之间的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	float DistanceToCrow = 0.0f;

	// 玩家是否正在朝乌鸦方向移动。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	bool bMovingTowardCrow = false;

	// 玩家是否处于主动追逐状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	bool bIsPursuing = false;

	// 玩家是否能看见乌鸦；当前阶段由蓝图或测试逻辑手动写入。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	bool bHasLineOfSight = true;

	// 追逐强度，范围建议为 0 到 1。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	float PursuitIntensity = 0.0f;

	// 本次采样使用的时间步长。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Input")
	float DeltaSeconds = 0.0f;
};

// 乌鸦行为模型输出，供蓝图或未来的乌鸦 Actor 决定动画、移动和调试表现。
USTRUCT(BlueprintType)
struct PROJECT_API FCrowBehaviorOutput
{
	GENERATED_BODY()

public:
	// 模型建议的乌鸦状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	ECrowState State = ECrowState::Idle;

	// 乌鸦希望和玩家保持的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	float DesiredDistance = 600.0f;

	// 行为延迟秒数，用于形成“总慢一步”的节奏。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	float DelaySeconds = 0.25f;

	// 观察玩家的权重，范围建议为 0 到 1。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	float LookAtPlayerWeight = 0.0f;

	// 是否建议乌鸦拉开距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	bool bShouldRetreat = false;

	// 是否建议乌鸦停留等待玩家。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Output")
	bool bShouldWait = false;
};
