#pragma once

#include "CoreMinimal.h"
#include "PixelFormat.h"
#include "Engine/Texture2D.h"
#include <functional>

struct FPixelBuffer
{
	void* Handle = nullptr;
	const void* Data = nullptr;
	EPixelFormat Format = EPixelFormat::PF_Unknown;
	int Pitch = 0;
	int Width = 0;
	int Height = 0;
};

class MEDIAPIPE_API FDynamicTexture
{
public:
	FDynamicTexture();
	~FDynamicTexture();

	void EnqueBuffer(const FPixelBuffer& InBuffer);
	inline UTexture2D* GetTextureObject() { return TextureObject; }

	std::function<void(UTexture2D* Texture)> FuncTextureCreated;
	std::function<void(void* Handle)> FuncBufferSubmitted;

private:
	FPixelBuffer* AllocBuffer();
	void ReturnBufferToPool(FPixelBuffer* Buffer);
	void ReleaseBufferPool();
	void Resize(int InWidth, int InHeight, EPixelFormat InFormat);
	void Release();
	void RenderCmd_CreateTexture();
	void RenderCmd_UpdateTexture(FPixelBuffer* InBuffer);

private:
	UTexture2D* TextureObject = nullptr;
	TArray<FPixelBuffer*> BufferPool;
	FCriticalSection BufferMux;
	EPixelFormat Format = EPixelFormat::PF_Unknown;
	int Pitch = 0;
	int Width = 0;
	int Height = 0;
};
