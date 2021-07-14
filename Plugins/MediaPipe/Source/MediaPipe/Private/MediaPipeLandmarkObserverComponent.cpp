#include "MediaPipeLandmarkObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"

#include "ump_api.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "DrawDebugHelpers.h"

using FMediaPipeLandmarkList = TArray<FMediaPipeLandmark>;

UMediaPipeLandmarkObserverComponent::UMediaPipeLandmarkObserverComponent()
{
}

struct LandmarkParser
{
	template<typename TLandmark>
	void ParseLandmark(const TLandmark& Src, FMediaPipeLandmark& Dst)
	{
		Dst.Pos = FVector(Src.x(), Src.y(), Src.z());
		Dst.Visibility = Src.visibility();
		Dst.Presence = Src.presence();
	}

	template<typename TList>
	void ParseList(const TList& Src, FMediaPipeLandmarkList& Dst)
	{
		const int Count = Src.landmark_size();
		Dst.SetNum(Count);
		for (int i = 0; i < Count; ++i)
			ParseLandmark(Src.landmark(i), Dst[i]);
	}

	template<typename TMsg>
	void Parse(const TMsg& Src, TArray<FMediaPipeLandmarkList>& MultiLandmarks, int& Count)
	{
		Count = 1;
		Grow(MultiLandmarks, 1);
		ParseList(Src, MultiLandmarks[0]);
	}

	template<typename TMsg>
	void Parse(const std::vector<TMsg>& Src, TArray<FMediaPipeLandmarkList>& MultiLandmarks, int& Count)
	{
		Count = Src.size();
		Grow(MultiLandmarks, Count);
		for (int i = 0; i < Count; ++i)
			ParseList(Src[i], MultiLandmarks[i]);
	}
};

template<typename TList>
void Parse(IUmpObserver* Observer, TArray<FMediaPipeLandmarkList>& MultiLandmarks, int& Count)
{
	if (!UmpCompareType<TList>(Observer))
	{
		PLUGIN_LOG(Error, TEXT("Invalid Landmark message"));
		return;
	}

	const auto& Message = UmpCastPacket<TList>(Observer->GetData());

	LandmarkParser Parser;
	Parser.Parse(Message, MultiLandmarks, Count);
}

// WARNING: executed in MediaPipe thread context!
void UMediaPipeLandmarkObserverComponent::OnUmpPacket(IUmpObserver* Observer)
{
	int InCount = 0;
	switch (LandmarkListType)
	{
		case EMediaPipeLandmarkListType::LandmarkList:					Parse<mediapipe::LandmarkList>(Observer, MultiLandmarks, InCount); break;
		case EMediaPipeLandmarkListType::NormalizedLandmarkList:		Parse<mediapipe::NormalizedLandmarkList>(Observer, MultiLandmarks, InCount); break;
		case EMediaPipeLandmarkListType::MultiLandmarkList:				Parse<std::vector<mediapipe::LandmarkList>>(Observer, MultiLandmarks, InCount); break;
		case EMediaPipeLandmarkListType::MultiNormalizedLandmarkList:	Parse<std::vector<mediapipe::NormalizedLandmarkList>>(Observer, MultiLandmarks, InCount); break;
		default: check(false); break;
	}

	// convert coordinate system
	for (int i = 0; i < InCount; ++i)
	{
		auto& Object = MultiLandmarks[i];
		const int LandmarkCount = Object.Num();

		for (int j = 0; j < LandmarkCount; ++j)
		{
			auto& L = Object[j];
			L.Pos = Scale3D(ShuffleAxes(L.Pos, (int)AxisX, (int)AxisY, (int)AxisZ), WorldScale);
		}
	}

	NumDetections = InCount;
	UpdateTimestamp();
}

const TArray<FMediaPipeLandmark>& UMediaPipeLandmarkObserverComponent::GetLandmarkList(int ObjectId)
{
	if (ObjectId >= 0 && ObjectId < NumDetections)
	{
		return MultiLandmarks[ObjectId];
	}
	PLUGIN_LOG(Error, TEXT("ObjectId out of range"));
	static FMediaPipeLandmarkList Dummy;
	return Dummy;
}

const FMediaPipeLandmark& UMediaPipeLandmarkObserverComponent::GetLandmark(int ObjectId, int LandmarkId)
{
	const auto& List = GetLandmarkList(ObjectId);
	if (LandmarkId >= 0 && LandmarkId < List.Num())
	{
		return List[LandmarkId];
	}
	PLUGIN_LOG(Error, TEXT("LandmarkId out of range"));
	static FMediaPipeLandmark Dummy;
	return Dummy;
}

void UMediaPipeLandmarkObserverComponent::DrawDebugLandmarks(int ObjectId, const FTransform& Transform, float PrimitiveScale, FLinearColor Color)
{
	#if ENABLE_DRAW_DEBUG

	if (ObjectId >= 0 && ObjectId < NumDetections)
	{
		auto* World = GetWorld();
		const auto& List = MultiLandmarks[ObjectId];
		const int Count = List.Num();
		const auto RawColor = Color.ToFColor(false);

		for (int i = 0; i < Count; i++)
		{
			const auto& L = List[i];
			if ((L.Visibility > MinVisibility) && (L.Presence > MinPresence))
			{
				auto Pos = Transform.TransformPosition(L.Pos);
				DrawDebugPoint(World, Pos, PrimitiveScale, RawColor);
				//DrawDebugCoordinateSystem(World, Pos, FRotator::ZeroRotator, PrimitiveScale);
			}
		}
	}

	#endif
}
