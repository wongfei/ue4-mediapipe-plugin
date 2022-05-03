#include "MediaPipePipelineComponent.h"
#include "MediaPipeObserverComponent.h"
#include "MediaPipeModule.h"
#include "MediaPipeShared.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("PacketCounter"), STAT_MediaPipe_PacketCounter, STATGROUP_MediaPipe);

// const FObjectInitializer& ObjectInitializer
UMediaPipePipelineComponent::UMediaPipePipelineComponent()
{
	PLUGIN_LOG_INFO(TEXT("+UMediaPipePipelineComponent %p"), this);
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UMediaPipePipelineComponent::BeginDestroy()
{
	PLUGIN_LOG_INFO(TEXT("~UMediaPipePipelineComponent %p"), this);
	Super::BeginDestroy();
}

void UMediaPipePipelineComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CreatePipeline();
}

void UMediaPipePipelineComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMediaPipePipelineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UMediaPipePipelineComponent::UninitializeComponent()
{
	ReleasePipeline();
	Super::UninitializeComponent();
}

void UMediaPipePipelineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Impl)
	{
		LastFrameId = (int)Impl->GetLastFrameId();
		LastFrameTimestamp = (float)Impl->GetLastFrameTimestamp();
	}

	StatAccum += DeltaTime;
	if (StatAccum >= 1.0f)
	{
		StatAccum -= 1.0f;
		if (StatAccum >= 1.0f)
			StatAccum = 0;
		
		int32 Count = 0;
		for (auto* Iter : Observers)
		{
			Count += Iter->GetPacketCounter();
			Iter->ResetPacketCounter();
		}

		SET_DWORD_STAT(STAT_MediaPipe_PacketCounter, Count);
	}
}

bool UMediaPipePipelineComponent::CreatePipeline()
{
	PLUGIN_TRACE;
	if (!Impl)
	{
		auto& Module = FModuleManager::GetModuleChecked<IMediaPipeModule>(TEXT(PLUGIN_NAME));
		Impl = Module.CreatePipeline();
	}
	return Impl ? true : false;
}

void UMediaPipePipelineComponent::ReleasePipeline()
{
	PLUGIN_TRACE;
	if (Impl)
	{
		Impl->Release();
		Impl = nullptr;
	}
}

void UMediaPipePipelineComponent::AddObserver(class UMediaPipeObserverComponent* InObserver)
{
	PLUGIN_TRACE;
	if (IsPipelineRunning)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: pipeline running"));
		return;
	}
	if (!Observers.Contains(InObserver))
		Observers.Add(InObserver);
}

void UMediaPipePipelineComponent::RemoveObserver(class UMediaPipeObserverComponent* InObserver)
{
	PLUGIN_TRACE;
	if (IsPipelineRunning)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: pipeline running"));
		return;
	}
	if (Observers.Contains(InObserver))
		Observers.Remove(InObserver);
}

void UMediaPipePipelineComponent::RemoveAllObservers()
{
	PLUGIN_TRACE;
	if (IsPipelineRunning)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: pipeline running"));
		return;
	}
	Observers.Empty();
}

bool UMediaPipePipelineComponent::Start()
{
	PLUGIN_TRACE;

	if (!Impl)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: IUmpPipeline"));
		return false;
	}

	if (IsPipelineRunning)
	{
		PLUGIN_LOG(Error, TEXT("Invalid state: pipeline running"));
		return false;
	}

	FString ConfigStr;
	ConfigStr.Reserve(1024);
	for (const auto& Name : GraphNodes)
	{
		ConfigStr += GraphPrefix;
		ConfigStr += Name;
		ConfigStr += GraphExt;
		ConfigStr += TEXT(";");
	}

	Impl->SetGraphConfiguration(TCHAR_TO_ANSI(*ConfigStr));
	Impl->SetOverlay(bEnableOverlay);

	if (InputFile.IsEmpty())
		Impl->SetCaptureParams(CameraId, CaptureApi, ResX, ResY, Fps);
	else
		Impl->SetCaptureFromFile(TCHAR_TO_ANSI(*InputFile));

	if (bAutoBindObservers)
	{
		TInlineComponentArray<UMediaPipeObserverComponent*> Components;
		GetOwner()->GetComponents(Components);
		for (auto* Comp : Components)
		{
			if (Comp->bAllowAutoBind && Comp->PipelineName == PipelineName)
				AddObserver(Comp);
		}
	}

	if (bEnableVideoTexture)
	{
		auto* Pipeline = this;

		VideoTexture.Reset(new FDynamicTexture());

		VideoTexture->FuncTextureCreated = [Pipeline](UTexture2D* Texture)
		{
			if (Pipeline->OnVideoTextureCreated.IsBound())
			{
				AsyncTask(ENamedThreads::GameThread, [Pipeline]()
				{
					Pipeline->OnVideoTextureCreated.Broadcast(Pipeline->VideoTexture->GetTextureObject());
				});
			}
		};

		VideoTexture->FuncBufferSubmitted = [](void* Handle)
		{
			auto* Frame = (IUmpFrame*)Handle; // see OnUmpFrame
			Frame->Release();
		};

		Impl->SetFrameCallback(static_cast<IUmpFrameCallback*>(this)); // will trigger OnUmpFrame
	}

	for (int i = 0; i < Observers.Num(); ++i)
		Observers[i]->OnPipelineStarting(Impl);

	IsPipelineRunning = Impl->Start();
	return IsPipelineRunning;
}

void UMediaPipePipelineComponent::Stop()
{
	PLUGIN_TRACE;
	if (Impl && IsPipelineRunning)
	{
		for (int i = 0; i < Observers.Num(); ++i)
			Observers[i]->OnPipelineStopping(Impl);

		Impl->Stop();
		IsPipelineRunning = false;
	}
	VideoTexture.Reset();
	if (Impl)
		Impl->LogProfilerStats();
}

void UMediaPipePipelineComponent::OnUmpFrame(IUmpFrame* InFrame)
{
	if (!VideoTexture)
		return;

	EPixelFormat Format;
	switch (InFrame->GetFormat())
	{
		case EUmpPixelFormat::B8G8R8A8:
			Format = PF_B8G8R8A8;
			break;

		case EUmpPixelFormat::R8G8B8A8:
			Format = PF_R8G8B8A8;
			break;

		default:
			// UNSUPPORTED
			return;
	}

	FPixelBuffer Buf;
	Buf.Handle = InFrame;
	Buf.Data = InFrame->GetData();
	Buf.Format = Format;
	Buf.Pitch = InFrame->GetPitch();
	Buf.Width = InFrame->GetWidth();
	Buf.Height = InFrame->GetHeight();

	VideoTexture->EnqueBuffer(Buf); // when done triggers VideoTexture->FuncBufferSubmitted(InFrame)
}
