#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowRelationTypes.h"
#include "CrowBehaviorModelComponent.generated.h"

// 根据玩家行为输入计算乌鸦关系输出的轻量模型组件，不执行移动或完整 AI。
UCLASS(ClassGroup = (Crow), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PROJECT_API UCrowBehaviorModelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCrowBehaviorModelComponent();

	// 评估玩家输入并返回乌鸦行为建议。
	UFUNCTION(BlueprintCallable, Category = "Crow|Behavior Model")
	FCrowBehaviorOutput EvaluateBehavior(const FPlayerBehaviorInput& PlayerInput);

	// 重置关系分数和当前状态，方便蓝图测试。
	UFUNCTION(BlueprintCallable, Category = "Crow|Behavior Model")
	void ResetRelation();

	// 获取当前关系分数。
	UFUNCTION(BlueprintPure, Category = "Crow|Behavior Model")
	FCrowRelationScore GetRelationScore() const;

	// 当前关系分数，可在蓝图中调参。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	FCrowRelationScore RelationScore;

	// 当前高层状态。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	ECrowState CurrentState = ECrowState::Idle;

	// 乌鸦与玩家之间的基础理想距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	float BaseDesiredDistance = 600.0f;

	// 玩家过近时触发后撤的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	float RetreatDistance = 350.0f;

	// 玩家较远时乌鸦可以等待的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	float WaitDistance = 900.0f;

	// 关系模型产生的最小延迟。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	float MinDelaySeconds = 0.15f;

	// 关系模型产生的最大延迟。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Behavior Model")
	float MaxDelaySeconds = 0.9f;

private:
	void UpdateRelationScore(const FPlayerBehaviorInput& PlayerInput);
	ECrowState ChooseState(const FPlayerBehaviorInput& PlayerInput) const;
};
