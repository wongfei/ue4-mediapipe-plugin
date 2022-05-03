#include "MediaPipeObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"
#include "Engine/World.h"

UMediaPipeObserverComponent::UMediaPipeObserverComponent()
{
}

void UMediaPipeObserverComponent::OnPipelineStarting(IUmpPipeline* Pipeline)
{
	UmpSafeRelease(Impl);

	PLUGIN_LOG_INFO(TEXT("OnPipelineStarting: %s"), *StreamName);

	if (StreamName.IsEmpty())
	{
		PLUGIN_LOG(Error, TEXT("Invalid property: StreamName"));
		return;
	}

	Impl = Pipeline->CreateObserver(TCHAR_TO_ANSI(*StreamName));
	if (!Impl)
	{
		PLUGIN_LOG(Error, TEXT("Unable to create IUmpObserver"));
		return;
	}

	Impl->SetPacketCallback(static_cast<IUmpPacketCallback*>(this));
}

void UMediaPipeObserverComponent::OnPipelineStopping(IUmpPipeline* Pipeline)
{
	PLUGIN_LOG_INFO(TEXT("OnPipelineStopping: %s"), *StreamName);

	UmpSafeRelease(Impl);
}

void UMediaPipeObserverComponent::OnUmpPresence(class IUmpObserver* observer, bool present)
{
	if (!present)
		NumDetections = 0;
}

int UMediaPipeObserverComponent::GetNumDetections()
{
	return NumDetections;
}

bool UMediaPipeObserverComponent::HaveDetections()
{
	return NumDetections > 0;
}

void UMediaPipeObserverComponent::UpdateTimestamp()
{
	LastUpdate = GetWorld()->GetRealTimeSeconds();
	PacketCounter.Increment();
}
