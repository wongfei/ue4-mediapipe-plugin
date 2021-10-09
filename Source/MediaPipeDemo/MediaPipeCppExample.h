#pragma once

#include "MediaPipePipelineComponent.h"
#include "MediaPipeLandmarkObserverComponent.h"
#include "MediaPipeCppExample.generated.h"

UCLASS(ClassGroup="MediaPipe")
class AMediaPipeCppExample : public AActor
{
	GENERATED_BODY()

public:
	AMediaPipeCppExample();
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void Tick(float DeltaSeconds) override;

	UPROPERTY()
	USceneComponent* Root = nullptr;

	UPROPERTY()
	UMediaPipePipelineComponent* Pipeline = nullptr;

	UPROPERTY()
	UMediaPipeLandmarkObserverComponent* Observer = nullptr;
};
