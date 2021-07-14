#include "MediaPipePresenceObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"

#include "ump_api.h"

UMediaPipePresenceObserverComponent::UMediaPipePresenceObserverComponent()
{
}

void UMediaPipePresenceObserverComponent::OnUmpPacket(IUmpObserver* Observer)
{
	if (!UmpCompareType<bool>(Observer))
	{
		PLUGIN_LOG(Error, TEXT("Invalid Presence message"));
		return;
	}

	bPresent = UmpCastPacket<bool>(Observer->GetData());
	UpdateTimestamp();
}
