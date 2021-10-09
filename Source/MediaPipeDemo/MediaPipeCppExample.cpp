#include "MediaPipeCppExample.h"
#include "DrawDebugHelpers.h"

AMediaPipeCppExample::AMediaPipeCppExample()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Pipeline = CreateDefaultSubobject<UMediaPipePipelineComponent>(TEXT("Pipeline"));
	Observer = CreateDefaultSubobject<UMediaPipeLandmarkObserverComponent>(TEXT("Observer"));

	Pipeline->PipelineName = TEXT("pipe0");
	Pipeline->GraphNodes.Add(TEXT("pose_landmarks"));

	Observer->PipelineName = TEXT("pipe0");
	Observer->StreamName = TEXT("pose_world_landmarks");
	Observer->LandmarkListType = EMediaPipeLandmarkListType::LandmarkList;
	Observer->AxisX = EMediaPipeLandmarkAxisMapping::Z;
	Observer->AxisY = EMediaPipeLandmarkAxisMapping::X;
	Observer->AxisZ = EMediaPipeLandmarkAxisMapping::NegY;
	Observer->WorldScale = FVector(100, 100, 100);
	Observer->MinVisibility = 0.5;
	Observer->MinPresence = 0.5;

	Pipeline->AddObserver(Observer);
}

void AMediaPipeCppExample::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RegisterAllComponents();
}

void AMediaPipeCppExample::BeginPlay()
{
	Super::BeginPlay();
	Pipeline->Start();
}

void AMediaPipeCppExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Pipeline->Stop();
	Super::EndPlay(EndPlayReason);
}

void AMediaPipeCppExample::Tick(float DeltaSeconds)
{
	if (Observer && Observer->HaveDetections())
	{
		int ObjectId = 0;
		const auto& Landmarks = Observer->GetLandmarkList(ObjectId);
		const int Count = Landmarks.Num();

		auto* World = GetWorld();
		auto Transform = GetActorTransform();

		for (int i = 0; i < Count; i++)
		{
			const auto& L = Landmarks[i];
			if ((L.Visibility > Observer->MinVisibility) && (L.Presence > Observer->MinPresence))
			{
				auto Pos = Transform.TransformPosition(L.Pos);
				DrawDebugPoint(World, Pos, 10.0f, FColor(255, 0, 0));
			}
		}
	}
}
