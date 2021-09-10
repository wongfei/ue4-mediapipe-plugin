#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediaPipeLandmarkObserverComponent.generated.h"

UENUM(Blueprintable)
enum class EMediaPipeLandmarkListType : uint8
{
	LandmarkList = 0,
	NormalizedLandmarkList,
	MultiLandmarkList,
	MultiNormalizedLandmarkList,
};

UENUM(Blueprintable)
enum class EMediaPipeLandmarkAxisMapping : uint8
{
	X = 0,
	Y,
	Z,
	NegX,
	NegY,
	NegZ,
};

USTRUCT(BlueprintType)
struct FMediaPipeLandmark
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Pos = {0, 0, 0};

	UPROPERTY(BlueprintReadOnly)
	float Visibility = 0;

	UPROPERTY(BlueprintReadOnly)
	float Presence = 0;
};

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipeLandmarkObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipeLandmarkObserverComponent();

	// Config

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	EMediaPipeLandmarkListType LandmarkListType = EMediaPipeLandmarkListType::MultiNormalizedLandmarkList;

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	EMediaPipeLandmarkAxisMapping AxisX = EMediaPipeLandmarkAxisMapping::X;

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	EMediaPipeLandmarkAxisMapping AxisY = EMediaPipeLandmarkAxisMapping::Y;

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	EMediaPipeLandmarkAxisMapping AxisZ = EMediaPipeLandmarkAxisMapping::Z;

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	FVector WorldScale = FVector(1, 1, 1);

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	float MinVisibility = -1;

	UPROPERTY(Category="MediaPipe|Landmarks", BlueprintReadWrite, EditAnywhere)
	float MinPresence = -1;

	// Getters

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const TArray<FMediaPipeLandmark>& GetLandmarkList(int ObjectId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	const FMediaPipeLandmark& GetLandmark(int ObjectId, int LandmarkId);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetLandmarkList(int ObjectId, TArray<FMediaPipeLandmark>& LandmarkList);

	UFUNCTION(Category="MediaPipe", BlueprintCallable, BlueprintPure)
	bool TryGetLandmark(int ObjectId, int LandmarkId, FMediaPipeLandmark& Landmark);

	// Utils

	UFUNCTION(Category="MediaPipe", BlueprintCallable)
	void DrawDebugLandmarks(int ObjectId, const FTransform& Transform, float PrimitiveScale = 1.0f, FLinearColor Color = FLinearColor(0, 1, 0, 1));

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;

	TArray<TArray<FMediaPipeLandmark>> MultiLandmarks;
};
