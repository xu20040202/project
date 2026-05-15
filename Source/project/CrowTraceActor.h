#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowTraceActor.generated.h"

class UStaticMeshComponent;

// 追逐时留下的简单痕迹占位物；只负责可见和自动消失，不承载玩法伤害或交互。
UCLASS(BlueprintType, Blueprintable)
class PROJECT_API ACrowTraceActor : public AActor
{
	GENERATED_BODY()

public:
	ACrowTraceActor();

	virtual void BeginPlay() override;

	// 痕迹可见占位体，蓝图子类可以替换 Mesh 或材质。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crow|Trace")
	UStaticMeshComponent* TraceMesh;

	// 痕迹保留时间，避免测试场景里无限堆积。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crow|Trace", meta = (ClampMin = "0.1"))
	float LifeSeconds = 6.0f;
};
