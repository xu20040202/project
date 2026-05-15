#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowRelationTypes.h"
#include "PlayerProbeComponent.generated.h"

// 采样玩家移动与追逐意图的轻量组件，不依赖输入系统或现有角色蓝图。
UCLASS(ClassGroup = (Crow), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PROJECT_API UPlayerProbeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerProbeComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 根据指定乌鸦 Actor 生成玩家行为输入快照。
	UFUNCTION(BlueprintCallable, Category = "Crow|Player Probe")
	FPlayerBehaviorInput GetBehaviorInput(AActor* CrowActor) const;

	// 手动设置视线状态，方便蓝图在没有完整感知系统时测试。
	UFUNCTION(BlueprintCallable, Category = "Crow|Player Probe")
	void SetHasLineOfSight(bool bNewHasLineOfSight);

	// 手动设置追逐状态，避免绑定或替换现有输入系统。
	UFUNCTION(BlueprintCallable, Category = "Crow|Player Probe")
	void SetIsPursuing(bool bNewIsPursuing);

	// 获取最近一次采样出的玩家速度。
	UFUNCTION(BlueprintPure, Category = "Crow|Player Probe")
	FVector GetEstimatedVelocity() const;

	// 玩家速度超过该值时才认为存在有效移动。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Player Probe")
	float MovementThreshold = 10.0f;

	// 玩家朝向乌鸦移动的点乘阈值，越高要求方向越准确。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Player Probe")
	float TowardDotThreshold = 0.35f;

	// 追逐强度归一化参考速度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Player Probe")
	float PursuitSpeedReference = 600.0f;

	// 当前是否认为玩家能看见乌鸦。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Player Probe")
	bool bHasLineOfSight = true;

	// 当前是否认为玩家正在主动追逐。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Player Probe")
	bool bIsPursuing = false;

private:
	FVector PreviousLocation = FVector::ZeroVector;
	FVector EstimatedVelocity = FVector::ZeroVector;
	float LastDeltaSeconds = 0.0f;
};
