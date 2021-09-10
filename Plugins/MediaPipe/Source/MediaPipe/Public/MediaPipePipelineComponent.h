#pragma once

#include "Components/ActorComponent.h"
#include "MediaPipePipelineComponent.generated.h"

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipePipelineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMediaPipePipelineComponent();

	// UObject
	void BeginDestroy() override;

	// UActorComponent
	void InitializeComponent() override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void UninitializeComponent() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core config

	UPROPERTY(Category="MediaPipe|Graph", BlueprintReadWrite, EditAnywhere)
	FString PipelineName;

	UPROPERTY(Category="MediaPipe|Graph", BlueprintReadWrite, EditAnywhere)
	FString GraphPrefix = TEXT("mediapipe/unreal/");

	UPROPERTY(Category="MediaPipe|Graph", BlueprintReadWrite, EditAnywhere)
	FString GraphExt = TEXT(".pbtxt");

	UPROPERTY(Category="MediaPipe|Graph", BlueprintReadWrite, EditAnywhere)
	TArray<FString> GraphNodes;

	UPROPERTY(Category="MediaPipe|Graph", BlueprintReadWrite, EditAnywhere)
	bool bAutoBindObservers = true;

	// Capture config

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	FString InputFile;

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	int CameraId = 0;

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	int CaptureApi = 0;

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	int ResX = 0;

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	int ResY = 0;

	UPROPERTY(Category="MediaPipe|Capture", BlueprintReadWrite, EditAnywhere)
	int Fps = 0;

	// Utils config

	UPROPERTY(Category="MediaPipe|Utils", BlueprintReadWrite, EditAnywhere)
	bool bEnableOverlay;

	// Stats

	UPROPERTY(Category="MediaPipe|Stats", BlueprintReadOnly)
	int LastFrameId = 0;

	UPROPERTY(Category="MediaPipe|Stats", BlueprintReadOnly)
	float LastFrameTimestamp = 0;

	// Methods

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void AddObserver(class UMediaPipeObserverComponent* Observer);

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void RemoveObserver(class UMediaPipeObserverComponent* Observer);

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void RemoveAllObservers();

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	bool Start();

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void Stop();

protected:
	bool CreatePipeline();
	void ReleasePipeline();

protected:
	class IUmpPipeline* Impl = nullptr;
	bool IsPipelineRunning = false;

	UPROPERTY()
	TArray<class UMediaPipeObserverComponent*> Observers;
};
