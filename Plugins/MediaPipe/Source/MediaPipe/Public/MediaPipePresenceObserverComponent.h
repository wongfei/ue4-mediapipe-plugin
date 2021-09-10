#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediaPipePresenceObserverComponent.generated.h"

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipePresenceObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipePresenceObserverComponent();

	UPROPERTY(Category="MediaPipe", BlueprintReadOnly)
	bool bPresent;

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;
};
