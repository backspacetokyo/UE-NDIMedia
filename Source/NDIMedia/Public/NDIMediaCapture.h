// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <thread>
#include <atomic>
#include <deque>

#include "CoreMinimal.h"
#include "MediaCapture.h"
#include "NDIMediaOutput.h"

#define PROCESSINGNDILIB_STATIC
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Lib.cplusplus.h>

#include "NDIMediaCapture.generated.h"

class UNDIMediaOutput;
struct NDIFrameBuffer;

UCLASS(BlueprintType)
class NDIMEDIA_API UNDIMediaCapture
	: public UMediaCapture
{
	GENERATED_UCLASS_BODY()
public:
	virtual bool HasFinishedProcessing() const override;
	
protected:
	virtual bool ValidateMediaOutput() const override;
	virtual bool CaptureSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport) override;
	virtual bool CaptureRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget) override;
	virtual bool UpdateSceneViewportImpl(TSharedPtr<FSceneViewport>& InSceneViewport) override;
	virtual bool UpdateRenderTargetImpl(UTextureRenderTarget2D* InRenderTarget) override;
	virtual void StopCaptureImpl(bool bAllowPendingFrameToBeProcess) override;

	virtual void OnFrameCaptured_RenderingThread(const FCaptureBaseData& InBaseData,
												 TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData,
												 void* InBuffer, int32 Width, int32 Height, int32 BytesPerRow) override;

private:
	
	NDIlib_send_instance_t pNDI_send;

	FCriticalSection RenderThreadCriticalSection;
	ENDIMediaOutputPixelFormat OutputPixelFormat;

	std::deque<NDIFrameBuffer*> FrameBuffers;
	uint8_t CurrentFrameBufferIndex = 0;

	std::thread NDISendThread;
	std::atomic_bool NDISendThreadRunning = false;

	bool InitNDI(UNDIMediaOutput* Output);
	bool DisposeNDI();
};
