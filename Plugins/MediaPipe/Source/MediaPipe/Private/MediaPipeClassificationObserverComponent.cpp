#include "MediaPipeClassificationObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"

#include "mediapipe/framework/formats/classification.pb.h"

using FMediaPipeClassificationList = TArray<FMediaPipeClassification>;

UMediaPipeClassificationObserverComponent::UMediaPipeClassificationObserverComponent()
{
}

// WARNING: executed in MediaPipe thread context!
void UMediaPipeClassificationObserverComponent::OnUmpPacket(IUmpObserver* Observer)
{
	using TList = std::vector<mediapipe::ClassificationList>;

	if (!UmpCompareType<TList>(Observer))
	{
		PLUGIN_LOG(Error, TEXT("Invalid Classification message"));
		NumDetections = 0;
		return;
	}

	const auto& Message = UmpCastPacket<TList>(Observer->GetData());
	const int ListCount = Message.size();
	Grow(MultiClassifications, ListCount);

	for (int i = 0; i < ListCount; ++i)
	{
		const auto& ListRef = Message[i];
		const int NodeCount = ListRef.classification_size();

		auto& StorageRef = MultiClassifications[i];
		StorageRef.SetNum(NodeCount);
		
		for (int j = 0; j < NodeCount; ++j)
		{
			const auto& Src = ListRef.classification(j);
			auto& Dst = StorageRef[j];

			Dst.Index = Src.index();
			Dst.Score = Src.score();
			Dst.Label = ANSI_TO_TCHAR(Src.label().c_str());
			Dst.DisplayName = ANSI_TO_TCHAR(Src.display_name().c_str());
		}
	}

	NumDetections = ListCount;
	UpdateTimestamp();
}

const TArray<FMediaPipeClassification>& UMediaPipeClassificationObserverComponent::GetClassificationList(int ObjectId)
{
	if (ObjectId >= 0 && ObjectId < NumDetections)
	{
		return MultiClassifications[ObjectId];
	}
	PLUGIN_LOG(Error, TEXT("ObjectId out of range"));
	static FMediaPipeClassificationList Dummy;
	return Dummy;
}

const FMediaPipeClassification& UMediaPipeClassificationObserverComponent::GetClassification(int ObjectId, int ClassificationId)
{
	const auto& List = GetClassificationList(ObjectId);
	if (ClassificationId >= 0 && ClassificationId < List.Num())
	{
		return List[ClassificationId];
	}
	PLUGIN_LOG(Error, TEXT("ClassificationId out of range"));
	static FMediaPipeClassification Dummy;
	return Dummy;
}

bool UMediaPipeClassificationObserverComponent::TryGetClassificationList(int ObjectId, TArray<FMediaPipeClassification>& ClassificationList)
{
	if (ObjectId >= 0 && ObjectId < NumDetections)
	{
		ClassificationList = MultiClassifications[ObjectId];
		return true;
	}
	return false;
}

bool UMediaPipeClassificationObserverComponent::TryGetClassification(int ObjectId, int ClassificationId, FMediaPipeClassification& Classification)
{
	if (ObjectId >= 0 && ObjectId < NumDetections)
	{
		const auto& List = MultiClassifications[ObjectId];
		if (ClassificationId >= 0 && ClassificationId < List.Num())
		{
			Classification = List[ClassificationId];
			return true;
		}
	}
	return false;
}
