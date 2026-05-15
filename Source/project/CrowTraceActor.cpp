#include "CrowTraceActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowTraceActor::ACrowTraceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TraceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TraceMesh"));
	SetRootComponent(TraceMesh);
	TraceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TraceMesh->SetGenerateOverlapEvents(false);
	TraceMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.04f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		TraceMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ACrowTraceActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSeconds);
}
