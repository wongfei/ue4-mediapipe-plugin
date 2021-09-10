#pragma once

#include "Components/ActorComponent.h"
#include "MediaPipeObserverComponent.generated.h"

class IMediaPipePipelineCallback
{
public:
	virtual void OnPipelineStarting(class IUmpPipeline* Pipeline) = 0;
	virtual void OnPipelineStopping(class IUmpPipeline* Pipeline) = 0;
};

class IMediaPipePacketCallback // IUmpPacketCallback
{
public:
	virtual void OnUmpPresence(class IUmpObserver* observer, bool present) = 0;
	virtual void OnUmpPacket(class IUmpObserver* observer) = 0;
};

UCLASS(ClassGroup="MediaPipe")
class MEDIAPIPE_API UMediaPipeObserverComponent : 
	public UActorComponent, 
	public IMediaPipePipelineCallback, 
	public IMediaPipePacketCallback
{
	GENERATED_BODY()

public:
	UMediaPipeObserverComponent();

	// Config

	UPROPERTY(Category="MediaPipe|Observer", BlueprintReadWrite, EditAnywhere)
	FString PipelineName;

	UPROPERTY(Category="MediaPipe|Observer", BlueprintReadWrite, EditAnywhere)
	FString StreamName;

	UPROPERTY(Category="MediaPipe|Observer", BlueprintReadWrite, EditAnywhere)
	bool bAllowAutoBind = true;

	// Runtime

	UPROPERTY(Category="MediaPipe", BlueprintReadOnly)
	float LastUpdate;

	// Getters

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	int GetNumDetections();

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool HaveDetections();

protected:
	void UpdateTimestamp();

protected:
	friend class UMediaPipePipelineComponent;
	void OnPipelineStarting(class IUmpPipeline* Pipeline) override;
	void OnPipelineStopping(class IUmpPipeline* Pipeline) override;
	void OnUmpPresence(class IUmpObserver* observer, bool present) override;
	void OnUmpPacket(class IUmpObserver* observer) override {}

protected:
	class IUmpObserver* Impl = nullptr;
	TAtomic<int> NumDetections = 0;
};
