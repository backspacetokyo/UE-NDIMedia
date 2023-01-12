


#include "NDIMediaCustomTimeStep.h"

#include "MediaPlayerFacade.h"

UNDIMediaCustomTimeStep::UNDIMediaCustomTimeStep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

bool UNDIMediaCustomTimeStep::Initialize(UEngine* InEngine)
{
	if (!MediaPlayer)
		return false;
	
	return true;
}

void UNDIMediaCustomTimeStep::Shutdown(UEngine* InEngine)
{
	NDIMediaPlayer.Reset();
}

bool UNDIMediaCustomTimeStep::UpdateTimeStep(UEngine* InEngine)
{
	auto PlayerAsset = MediaPlayer.LoadSynchronous();
	auto Player = StaticCastSharedPtr<FNDIMediaPlayer>(PlayerAsset->GetPlayerFacade()->GetPlayer());
	if (!Player.IsValid())
		return false;

	NDIMediaPlayer = Player;

	const bool bWaitedForSync = WaitForSync();

	return true;
}

ECustomTimeStepSynchronizationState UNDIMediaCustomTimeStep::GetSynchronizationState() const
{	if (!NDIMediaPlayer || !NDIMediaPlayer->IsHardwareReady())
		return ECustomTimeStepSynchronizationState::Closed;

	return ECustomTimeStepSynchronizationState::Synchronized;
}

FFrameRate UNDIMediaCustomTimeStep::GetFixedFrameRate() const
{
	if (!NDIMediaPlayer || !NDIMediaPlayer->IsHardwareReady())
		return FFrameRate(60, 1);

	return NDIMediaPlayer->FrameRate;
}

uint32 UNDIMediaCustomTimeStep::GetLastSyncCountDelta() const
{
	return 1;
}

bool UNDIMediaCustomTimeStep::IsLastSyncDataValid() const
{
	if (!NDIMediaPlayer || !NDIMediaPlayer->IsHardwareReady())
		return false;

	return true;
}

bool UNDIMediaCustomTimeStep::WaitForSync()
{
	if (!NDIMediaPlayer || !NDIMediaPlayer->IsHardwareReady())
		return false;

	NDIMediaPlayer->WaitForSync();
	
	return true;
}
