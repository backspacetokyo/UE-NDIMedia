

#pragma once

#include "CoreMinimal.h"
#include "GenlockedCustomTimeStep.h"
#include "GenlockedFixedRateCustomTimeStep.h"
#include "MediaPlayer.h"
#include "NDIMediaPlayer.h"
#include "NDIMediaCustomTimeStep.generated.h"


UCLASS(Blueprintable)
class NDIMEDIA_API UNDIMediaCustomTimeStep : public UGenlockedFixedRateCustomTimeStep
{
	GENERATED_UCLASS_BODY()

	TSharedPtr<FNDIMediaPlayer, ESPMode::ThreadSafe> NDIMediaPlayer;
	
public:
	//~ UFixedFrameRateCustomTimeStep interface
	virtual bool Initialize(UEngine* InEngine) override;
	virtual void Shutdown(UEngine* InEngine) override;
	virtual bool UpdateTimeStep(UEngine* InEngine) override;
	virtual ECustomTimeStepSynchronizationState GetSynchronizationState() const override;
	virtual FFrameRate GetFixedFrameRate() const override;

	//~ UGenlockedCustomTimeStep interface
	virtual uint32 GetLastSyncCountDelta() const override;
	virtual bool IsLastSyncDataValid() const override;
	virtual bool WaitForSync() override;

public:

	UPROPERTY(EditAnywhere, Category="NDI Media")
	TSoftObjectPtr<UMediaPlayer> MediaPlayer;
};
