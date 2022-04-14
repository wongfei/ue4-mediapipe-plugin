#include "DynamicTexture.h"
#include "TextureResource.h"
#include "RenderUtils.h"
#include "Async/Async.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MediaPipeShared.h"

FDynamicTexture::FDynamicTexture()
{
	PLUGIN_LOG(Display, TEXT("+FDynamicTexture %p"), this);
}

FDynamicTexture::~FDynamicTexture()
{
	PLUGIN_LOG(Display, TEXT("~FDynamicTexture %p"), this);
	Release();
}

// POOL

FPixelBuffer* FDynamicTexture::AllocBuffer()
{
	FPixelBuffer* Buffer = nullptr;

	if (BufferPool.Num() > 0)
	{
		FScopeLock Lock(&BufferMux);
		if (BufferPool.Num() > 0)
		{
			Buffer = BufferPool.Pop(false);
			//PLUGIN_LOG(Display, TEXT("reuse FPixelBuffer %p"), Buffer);
			return Buffer;
		}
	}

	Buffer = new FPixelBuffer();
	PLUGIN_LOG(Display, TEXT("new FPixelBuffer %p"), Buffer);
	return Buffer;
}

void FDynamicTexture::ReturnBufferToPool(FPixelBuffer* Buffer)
{
	//PLUGIN_LOG(Display, TEXT("pool FPixelBuffer %p"), Buffer);
	FScopeLock Lock(&BufferMux);
	BufferPool.Push(Buffer);
}

void FDynamicTexture::ReleaseBufferPool()
{
	if (BufferPool.Num() > 0)
	{
		FScopeLock Lock(&BufferMux);
		for (int i = 0; i < BufferPool.Num(); ++i)
		{
			PLUGIN_LOG(Display, TEXT("delete FPixelBuffer %p"), BufferPool[i]);
			delete BufferPool[i];
		}
		BufferPool.Reset();
	}
}

// ENQUE

void FDynamicTexture::EnqueBuffer(const FPixelBuffer& InBuffer)
{
	if (InBuffer.Width <= 0 || InBuffer.Height <= 0)
		return;
	
	switch (InBuffer.Format)
	{
		case EPixelFormat::PF_B8G8R8A8:
		case EPixelFormat::PF_R8G8B8A8:
			break;

		default:
			// UNSUPPORTED
			return;
	}

	auto* Buffer = AllocBuffer();
	*Buffer = InBuffer;

	auto Context = this;
	ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)([Context, Buffer](FRHICommandListImmediate& RHICmdList)
	{
		Context->RenderCmd_UpdateTexture(Buffer);
	});
}

void FDynamicTexture::RenderCmd_UpdateTexture(FPixelBuffer* InBuffer)
{
	Resize(InBuffer->Width, InBuffer->Height, InBuffer->Format);

	auto* Tex = GetTextureObject();
	if (Tex)
	{
		#if (ENGINE_MAJOR_VERSION == 5)
			#define TMP_TEX_RES Tex->GetResource()
		#else
			#define TMP_TEX_RES Tex->Resource
		#endif

		if (TMP_TEX_RES)
		{
			RHIUpdateTexture2D(
				TMP_TEX_RES->GetTexture2DRHI(),
				0,
				FUpdateTextureRegion2D(0, 0, 0, 0, InBuffer->Width, InBuffer->Height),
				InBuffer->Pitch,
				(const uint8*)InBuffer->Data
			);
		}

		#undef TMP_TEX_RES
	}

	if (FuncBufferSubmitted)
	{
		FuncBufferSubmitted(InBuffer->Handle);
	}

	ReturnBufferToPool(InBuffer);
}

// RESIZE

void FDynamicTexture::Resize(int InWidth, int InHeight, EPixelFormat InFormat)
{
	if (Format != InFormat
		|| Width != InWidth
		|| Height != InHeight)
	{
		Release();

		PLUGIN_LOG(Display, TEXT("FDynamicTexture::Resize Width=%d Height=%d Format=%d"), InWidth, InHeight, (int)InFormat);

		Format = InFormat;
		Width = InWidth;
		Height = InHeight;
		Pitch = Width * GPixelFormats[Format].BlockBytes;

		auto Context = this;

		#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 26)

		// already in render thread context (see RenderCmd_UpdateTexture)
		//ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)([Context](FRHICommandListImmediate& RHICmdList)
		//{
			Context->RenderCmd_CreateTexture();
		//});

		#else // VERSION > 4.26

		AsyncTask(ENamedThreads::GameThread, [Context]()
		{
			Context->RenderCmd_CreateTexture();
		});

		#endif
	}
}

void FDynamicTexture::RenderCmd_CreateTexture()
{
	auto Tex = UTexture2D::CreateTransient(Width, Height, Format);
	if (Tex)
	{
		#if WITH_EDITORONLY_DATA
		Tex->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		#endif

		Tex->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		Tex->Filter = TextureFilter::TF_Nearest;
		Tex->SRGB = 0;

		Tex->AddToRoot();
		Tex->UpdateResource();

		TextureObject = Tex;

		if (FuncTextureCreated)
		{
			FuncTextureCreated(Tex);
		}
	}
}

// RELEASE

void FDynamicTexture::Release()
{
	if (TextureObject)
	{
		FlushRenderingCommands();
		TextureObject->RemoveFromRoot();
		TextureObject = nullptr;
	}
	
	Format = EPixelFormat::PF_Unknown;
	Pitch = 0;
	Width = 0;
	Height = 0;

	ReleaseBufferPool();
}
