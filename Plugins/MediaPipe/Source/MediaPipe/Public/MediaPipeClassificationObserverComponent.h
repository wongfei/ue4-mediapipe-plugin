#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediaPipeClassificationObserverComponent.generated.h"

// classification.proto

USTRUCT(BlueprintType)
struct FMediaPipeClassification
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int Index;

	UPROPERTY(BlueprintReadOnly)
	float Score;
	
	UPROPERTY(BlueprintReadOnly)
	FString Label;
	
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;
};

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipeClassificationObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipeClassificationObserverComponent();

	// Getters

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const TArray<FMediaPipeClassification>& GetClassificationList(int ObjectId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const FMediaPipeClassification& GetClassification(int ObjectId, int ClassificationId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetClassificationList(int ObjectId, TArray<FMediaPipeClassification>& ClassificationList);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetClassification(int ObjectId, int ClassificationId, FMediaPipeClassification& Classification);

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;

	TArray<TArray<FMediaPipeClassification>> MultiClassifications;
};
