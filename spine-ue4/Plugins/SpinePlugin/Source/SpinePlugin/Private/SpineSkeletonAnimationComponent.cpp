


#include "SpineSkeletonAnimationComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"

#include "SpinePlugin.h"
#include "spine/spine.h"

#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;

void UTrackEntry::SetTrackEntry(TSharedPtr<spine::TrackEntry> trackEntry)
{
	this->entry = trackEntry;
	if (entry.IsValid())
	{
		entry->SetRendererObject(this);
	}
}

int UTrackEntry::GetTrackIndex()
{
	return entry.IsValid() ? entry->getTrackIndex() : 0;
}

bool UTrackEntry::GetLoop()
{
	return entry.IsValid() ? entry->getLoop() : false;
}

void UTrackEntry::SetLoop(bool loop)
{
	if (entry.IsValid()) entry->setLoop(loop);
}

float UTrackEntry::GetEventThreshold()
{
	return entry.IsValid() ? entry->getEventThreshold() : 0;
}

void UTrackEntry::SetEventThreshold(float eventThreshold)
{
	if (entry.IsValid()) entry->setEventThreshold(eventThreshold);
}

float UTrackEntry::GetAttachmentThreshold()
{
	return entry.IsValid() ? entry->getAttachmentThreshold() : 0;
}

void UTrackEntry::SetAttachmentThreshold(float attachmentThreshold)
{
	if (entry.IsValid()) entry->setAttachmentThreshold(attachmentThreshold);
}

float UTrackEntry::GetDrawOrderThreshold()
{
	return entry.IsValid() ? entry->getDrawOrderThreshold() : 0;
}

void UTrackEntry::SetDrawOrderThreshold(float drawOrderThreshold)
{
	if (entry.IsValid()) entry->setDrawOrderThreshold(drawOrderThreshold);
}

float UTrackEntry::GetAnimationStart()
{
	return entry.IsValid() ? entry->getAnimationStart() : 0;
}

void UTrackEntry::SetAnimationStart(float animationStart)
{
	if (entry.IsValid()) entry->setAnimationStart(animationStart);
}

float UTrackEntry::GetAnimationEnd()
{
	return entry.IsValid() ? entry->getAnimationEnd() : 0;
}

void UTrackEntry::SetAnimationEnd(float animationEnd)
{
	if (entry.IsValid()) entry->setAnimationEnd(animationEnd);
}

float UTrackEntry::GetAnimationLast()
{
	return entry.IsValid() ? entry->getAnimationLast() : 0;
}

void UTrackEntry::SetAnimationLast(float animationLast)
{
	if (entry.IsValid()) entry->setAnimationLast(animationLast);
}

float UTrackEntry::GetDelay()
{
	return entry.IsValid() ? entry->getDelay() : 0;
}

void UTrackEntry::SetDelay(float delay)
{
	if (entry.IsValid()) entry->setDelay(delay);
}

float UTrackEntry::GetTrackTime()
{
	return entry.IsValid() ? entry->getTrackTime() : 0;
}

void UTrackEntry::SetTrackTime(float trackTime)
{
	if (entry.IsValid()) entry->setTrackTime(trackTime);
}

float UTrackEntry::GetTrackEnd()
{
	return entry.IsValid() ? entry->getTrackEnd() : 0;
}

void UTrackEntry::SetTrackEnd(float trackEnd)
{
	if (entry.IsValid()) entry->setTrackEnd(trackEnd);
}

float UTrackEntry::GetTimeScale()
{
	return entry.IsValid() ? entry->getTimeScale() : 0;
}

void UTrackEntry::SetTimeScale(float timeScale)
{
	if (entry.IsValid()) entry->setTimeScale(FMath::Max(SMALL_NUMBER, timeScale));
}

float UTrackEntry::GetAlpha()
{
	return entry.IsValid() ? entry->getAlpha() : 0;
}

void UTrackEntry::SetAlpha(float alpha)
{
	if (entry.IsValid()) entry->setAlpha(alpha);
}

float UTrackEntry::GetMixTime()
{
	return entry.IsValid() ? entry->getMixTime() : 0;
}

void UTrackEntry::SetMixTime(float mixTime)
{
	if (entry.IsValid()) entry->setMixTime(mixTime);
}

float UTrackEntry::GetMixDuration()
{
	return entry ? entry->getMixDuration() : 0;
}

void UTrackEntry::SetMixDuration(float mixDuration)
{
	if (entry.IsValid()) entry->setMixDuration(mixDuration);
}

FString UTrackEntry::GetAnimationName()
{
	if (entry.IsValid())
	{
		check(entry->getAnimation());
		return entry->getAnimation()->getName().buffer();
	}
	else
	{
		return FString();
	}
}

float UTrackEntry::GetAnimationDuration()
{
	if (entry.IsValid())
	{
		check(entry->getAnimation());
		return entry->getAnimation()->getDuration();
	}
	else
	{
		return 0;
	}
}



USpineSkeletonAnimationComponent::USpineSkeletonAnimationComponent (const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	DepthOffset = 0.002;

	Prior = 0;
}

void USpineSkeletonAnimationComponent::BeginPlay()
{
	trackEntries.Empty();

	Super::BeginPlay();

	//ConditionStartAnimationStateMachine();
}

void USpineSkeletonAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (bIsPaused == false)
	{
		ConditionStartAnimationStateMachine();
	}

	InternalTick_SkeletonPose(bIsPaused ? SMALL_NUMBER : DeltaTime);

	InternalTick_Renderer();

	InternalTick_UpdateMaterialParam(bIsPaused ? SMALL_NUMBER : DeltaTime);

	Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USpineSkeletonAnimationComponent::ConditionStartAnimationStateMachine()
{

	if (bCanPlayNormalAnimStateMachine)
	{
		if (!bIsAnimStateMachineWorking)
		{
			bIsAnimStateMachineWorking = true;

			AnimationStateMachineOn.Broadcast();
		}
	}
}

bool USpineSkeletonAnimationComponent::IsSpineAnimSpecPlaying(const FSpineAnimationSpec& InSpec)
{
	if (!InSpec.IsValid())
	{
		return false;
	}

	UTrackEntry* CurrentTrack = GetCurrent(0);

	if (!IsValid(CurrentTrack))
	{
		return false;
	}

	if (CurrentTrack->GetAnimationName() != InSpec.AnimationName || GetSkeletonAsset() != InSpec.RelatedSpineSkeletonDataAsset)
	{
		return false;
	}

	if (CurrentTrack->GetLoop()) //如果是loop 则不可能停止,视为true.
	{
		return true;
	}

	// 如果是非loop.当前Track持续时间只有在小于AnimationEnd的情况下,才是true.
	return CurrentTrack->GetTrackTime() <= CurrentTrack->GetAnimationEnd();
}

	

bool USpineSkeletonAnimationComponent::IsAnySpineAnimPlaying()
{
	UTrackEntry* CurrentTrack = GetCurrent(0);

	if (!IsValid(CurrentTrack))
	{
		return false;
	}

	if (CurrentTrack->GetLoop()) //如果是loop 则不可能停止,视为true.
	{
		return true;
	}

	// 如果是非loop.当前Track持续时间只有在小于AnimationEnd的情况下,才是true.
	return CurrentTrack->GetTrackTime() <= CurrentTrack->GetAnimationEnd();
}

FUpdateMaterialHandle USpineSkeletonAnimationComponent::SetMaterialParam(TArray<FUpdateMaterialParam> Params,
	FName GroupName, float Duration,float BlendInTime, float BlendOutTime,
	FOnUpdateMaterialFinishedDelegate OnFinished, FK2OnUpdateMaterialFinishedDelegate K2OnFinished/*= FK2OnUpdateMaterialFinishedDelegate()*/)
{
	if (GroupName != NAME_None)
	{
		if (FUpdateMaterialRuntime* FoundSameIndexRuntime = UpdateMaterialRuntimes.FindByPredicate(
			[GroupName](const FUpdateMaterialRuntime& TestRunTime) {return TestRunTime.GroupName == GroupName; }))
		{
			bool bIsCancel = true;
			bool bImmediately = true;
			bool bDisableCallback = false;
			RemoveMaterialParam(FoundSameIndexRuntime->Handle, bIsCancel, bImmediately, bDisableCallback);
		}
	}

	check(Duration == -1 || Duration > 0);
	check(BlendInTime >= 0);
	check(BlendOutTime >= 0);

	Params.RemoveAll([](const FUpdateMaterialParam& TestParam) {return !TestParam.IsValid(); });

	FUpdateMaterialRuntime Runtime;

	Runtime.Handle = FUpdateMaterialHandle::GenerateNewHandle();
	Runtime.Params = Params;
	Runtime.Duration = Duration;
	Runtime.TotalBlendInTime = BlendInTime;
	Runtime.TotalBlendOutTime = BlendOutTime;
	
	Runtime.GroupName = GroupName;

	Runtime.OnFinished = OnFinished;
	Runtime.K2OnFinished = K2OnFinished;

	Runtime.TimeRemaining = Duration;

	if (BlendInTime > 0.f)
	{
		Runtime.bBlendingIn = true;
	}

	UpdateMaterialRuntimes.Add(Runtime);

	//update now.
	InternalTick_UpdateMaterialParam(0);

	return Runtime.Handle;
}

void USpineSkeletonAnimationComponent::RemoveMaterialParam(FUpdateMaterialHandle InHandle,
	bool bIsCancel, bool bImmediately, bool bDisableCallback)
{
	if (!InHandle.IsValid())
	{
		return;
	}

	FUpdateMaterialRuntime* FoundRuntime = UpdateMaterialRuntimes.FindByPredicate([InHandle](const FUpdateMaterialRuntime& Runtime) {return Runtime.Handle == InHandle; });

	if (!FoundRuntime)
	{
		return;
	}

	if (bImmediately)
	{
		FUpdateMaterialRuntime CopyFoundRuntime = *FoundRuntime;
		UpdateMaterialRuntimes.Remove(CopyFoundRuntime);

		if (bIsTicking)
		{
			InTickPendingTriggerFinishCallbackArray.Add(
				FPendingTriggerFinishCallback{ bIsCancel ,CopyFoundRuntime.OnFinished ,CopyFoundRuntime.K2OnFinished });

			InTickPendingZeroedFinishState.Append(CopyFoundRuntime.GetZeroedFinishState());
		}
		else
		{
			//rollback applied material params.
			ApplyCurrentInForceParamToMaterial(CopyFoundRuntime.GetZeroedFinishState());

			if (!bDisableCallback)
			{
				CopyFoundRuntime.OnFinished.ExecuteIfBound(bIsCancel);
				CopyFoundRuntime.K2OnFinished.ExecuteIfBound(bIsCancel);
			}
		}
	}
	else
	{
		// advance to the blend out time

		FoundRuntime->TimeRemaining = (FoundRuntime->TimeRemaining > 0.0f) ?
			FMath::Min(FoundRuntime->TimeRemaining, FoundRuntime->TotalBlendOutTime) :
			FoundRuntime->TotalBlendOutTime;

		if (bDisableCallback)
		{
			FoundRuntime->K2OnFinished.Unbind();
			FoundRuntime->OnFinished.Unbind();
		}
	}
}

void USpineSkeletonAnimationComponent::ExternaStopMaterialParamByGroupName(FName GroupName, bool bImmediately /*= true*/)
{
	if (GroupName != NAME_None)
	{
		TArray<FUpdateMaterialHandle> PendingRemoveHandles;

		for (auto It = UpdateMaterialRuntimes.CreateIterator(); It; ++It)
		{
			if (It->GroupName == GroupName)
			{
				PendingRemoveHandles.Add(It->Handle);
			}
		}

		for (FUpdateMaterialHandle Handle : PendingRemoveHandles)
		{
			bool bIsCancel = true;
			bool bDisableCallback = false;
			RemoveMaterialParam(Handle, bIsCancel, bImmediately, bDisableCallback);

		}
	}
}

void USpineSkeletonAnimationComponent::ExternaStopAllMaterialParams(bool bImmediately /*= true*/)
{
	TArray<FUpdateMaterialHandle> PendingRemoveHandles;

	for (auto It = UpdateMaterialRuntimes.CreateIterator(); It; ++It)
	{
		PendingRemoveHandles.Add(It->Handle);
	}

	for (FUpdateMaterialHandle Handle : PendingRemoveHandles)
	{
		bool bIsCancel = true;
		bool bDisableCallback = false;
		RemoveMaterialParam(Handle, bIsCancel, bImmediately, bDisableCallback);
	}
}

void USpineSkeletonAnimationComponent::SetAvoidMaterialCrossYOffset(float InOffset)
{
	TArray<UMaterialInstanceDynamic*> MIDArray;

	MIDArray.Append(atlasAdditiveBlendMaterials);
	MIDArray.Append(atlasMultiplyBlendMaterials);
	MIDArray.Append(atlasNormalBlendMaterials);
	MIDArray.Append(atlasScreenBlendMaterials);

	for (UMaterialInstanceDynamic* MID: MIDArray)
	{
		if (IsValid(MID))
		{
			MID->SetScalarParameterValue(TEXT("AvoidCrossYOffset"), InOffset);
		}
	}

}

void USpineSkeletonAnimationComponent::InternalTick_UpdateMaterialParam(float DeltaTime)
{
	TGuardValue<bool>  GuardValue_bIsTicking(bIsTicking, true);

	TArray<FUpdateMaterialHandle, TInlineAllocator<8>> PendingFinishHandles;
	TMap<FParamID, FParamCurrentData> PendingAppliedParamDataMap;

	ensure(UpdateMaterialRuntimes.Num() < 30); //设定一个阈值,除非出bug,不然不可能超过这个数字.

	for (auto It = UpdateMaterialRuntimes.CreateIterator(); It; ++It)
	{
		TMap<FParamID, FParamCurrentData> TempData = It->UpdateAndEvaluateCurrentState(DeltaTime);

		if (It->IsFinished())
		{
			PendingFinishHandles.Add(It->Handle);
		}
		else
		{
			PendingAppliedParamDataMap.Append(MoveTemp(TempData));
		}
	}

	for (auto Handle : PendingFinishHandles)
	{
		bool bIsCancel = false;
		bool bImmediately = true;
		bool bDisableCallback = false;
		RemoveMaterialParam(Handle, bIsCancel, bImmediately, bDisableCallback);
	}

	ApplyCurrentInForceParamToMaterial(InTickPendingZeroedFinishState); //zero first.
	InTickPendingZeroedFinishState.Empty(8);

	ApplyCurrentInForceParamToMaterial(PendingAppliedParamDataMap);

	for (FPendingTriggerFinishCallback Item : InTickPendingTriggerFinishCallbackArray)
	{
		Item.OnFinished.ExecuteIfBound(Item.bIsCancel);
		Item.K2OnFinished.ExecuteIfBound(Item.bIsCancel);
	}
	InTickPendingTriggerFinishCallbackArray.Empty(8);
}

TArray<UMaterialInstanceDynamic*>& USpineSkeletonAnimationComponent::GetBlendMaterial(ESpineMaterialBlendType InBlendType)
{
	if (InBlendType == ESpineMaterialBlendType::Additive)
	{
		return atlasAdditiveBlendMaterials;
	}
	else if (InBlendType == ESpineMaterialBlendType::Multiply)
	{
		return atlasMultiplyBlendMaterials;
	}
	else if (InBlendType == ESpineMaterialBlendType::Normal)
	{
		return atlasNormalBlendMaterials;
	}
	else if (InBlendType == ESpineMaterialBlendType::Screen)
	{
		return atlasScreenBlendMaterials;
	}
	else
	{
		ensure(0); //Error BlendType.fallback to normal.

		return atlasNormalBlendMaterials;
	}
}



void USpineSkeletonAnimationComponent::ApplyCurrentInForceParamToMaterial(const TMap<FParamID, FParamCurrentData>& CurrentInForceParamState)
{
	for (auto It = CurrentInForceParamState.CreateConstIterator(); It; ++It)
	{
		for (UMaterialInstanceDynamic* MID : GetBlendMaterial(It->Key.BlendType))
		{
			if (It->Value.bIsFloat)
			{
				check(It->Value.FloatValue.IsSet());
				MID->SetScalarParameterValue(It->Key.Name, It->Value.FloatValue.GetValue());
			}
			else
			{
				check(It->Value.VectorValue.IsSet());
				MID->SetVectorParameterValue(It->Key.Name, It->Value.VectorValue.GetValue());
			}

		}
	}
}

void USpineSkeletonAnimationComponent::InternalTick_SkeletonPose(float DeltaTime)
{
	CheckState();


	if (SpineAnimState.IsValid() && ensure(GetSkeleton().IsValid()))
	{

		SpineAnimState->update(DeltaTime);
		SpineAnimState->apply(GetSkeleton()->AsShared().Get());

		GetSkeleton()->updateWorldTransform();
	}
}

void USpineSkeletonAnimationComponent::CheckState()
{
	bool bNeedsUpdate = (LastSkeletonAsset != SkeletonData);

	if (!bNeedsUpdate)
	{
		// Are we doing a re-import? Then check if the underlying spine-cpp data
		if (IsValid(SkeletonData))
		{
			auto TestSkeletonData = SkeletonData->GetSpineSkeletonData();
			if (lastSkeletonData != TestSkeletonData)
			{
				bNeedsUpdate = true;
			}
		}
		
	}

	if (bForceRefresh)
	{
		bForceRefresh = false;
		bNeedsUpdate = true;
		UE_LOG(SpineLog, Display, TEXT("USpineSkeletonComponent::CheckState Force Refresh!!!!!!!!!!!!!!!"));
	}

	if (bNeedsUpdate)
	{
		DisposeState();

		if (SkeletonData)
		{
			TSharedPtr<spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
			lastSkeletonData = SpineSkeletonData;

			if (SpineSkeletonData.IsValid())
			{
				CurrentSpineSkeleton = MakeShared<spine::Skeleton>(SpineSkeletonData->AsShared());

				TSharedPtr<spine::AnimationStateData> SpineAnimStateData = SkeletonData->CreateAnimationStateData();
				if (SpineAnimStateData.IsValid())
				{
					SpineAnimState = MakeShared<spine::AnimationState>(SpineAnimStateData->AsShared());
					SpineAnimState->SetRendererObject(this);
					SpineAnimState->setListener(FSpineAnimationStateCallbackDelegate::CreateUObject(this, &ThisClass::OnSpineAnimStateEventReceived));
				}

			}
		}

		LastSkeletonAsset = SkeletonData;
	}
}

void USpineSkeletonAnimationComponent::DisposeState()
{
	ensure(trackEntries.Num() < 100);

	for (int32 i = 0; i < trackEntries.Num(); i++)
	{
		auto uEntry = trackEntries[i];
		if (IsValid(uEntry))
		{
			//FString AnimNameInTrackEntry = uEntry->GetAnimationName();

			bool bShouldCareEvent = !uEntry->GetTrackEntry()->isComplete() || uEntry->GetLoop();

			if (bShouldCareEvent)
			{
				//	UE_LOG(LogTemp, Warning, TEXT("DisposeState uEntry:%s  %s "), *GetNameSafe(uEntry), *AnimNameInTrackEntry);
				AnimationInterrupt.Broadcast(uEntry);
				uEntry->AnimationInterrupt.Broadcast(uEntry);
			}
		}
	}

	SpineAnimState.Reset();
	trackEntries.Empty();

	Super::DisposeState();
}


bool USpineSkeletonAnimationComponent::ApplyReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup)
{
	bool bAnyReplaced = Super::ApplyReplaceAttachment(ReplacementGroup);

	if (bAnyReplaced&&SpineAnimState)
	{
		SpineAnimState->apply(GetSkeleton()->AsShared().Get());
	}

	return bAnyReplaced;
}

bool USpineSkeletonAnimationComponent::CancelReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup)
{
	bool bAnyReplaced = Super::CancelReplaceAttachment(ReplacementGroup);

	if (bAnyReplaced&&SpineAnimState)
	{
		SpineAnimState->apply(GetSkeleton()->AsShared().Get());
	}
	return bAnyReplaced;
}

bool USpineSkeletonAnimationComponent::SetSkin(FString SkinName)
{
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		TSharedPtr<Skin> skin = CurrentSpineSkeleton->getData()->findSkin(SkinName);
		if (!skin)
		{
			return false;
		}

		CurrentSpineSkeleton->setSkin(skin);
		CurrentSpineSkeleton->setSlotsToSetupPose();

		if (SpineAnimState)
		{
			SpineAnimState->apply(GetSkeleton()->AsShared().Get());
		}
		OnSpineSkinUpdated.Broadcast(SkinName);
		return true;
	}
	return false;
}

void USpineSkeletonAnimationComponent::StopAnimationStateMachine()
{
	if (bCanPlayNormalAnimStateMachine)
	{
		bCanPlayNormalAnimStateMachine = false;
	}

	if (bIsAnimStateMachineWorking)
	{
		bIsAnimStateMachineWorking = false;

		AnimationStateMachineOff.Broadcast();

		if (auto Track = GetCurrent(0))
		{
			//Track->SetTrackTime((Track->GetAnimationEnd() - Track->GetAnimationStart()) + SMALL_NUMBER);
				SetEmptyAnimation(0, 0);
		}
	}
}

void USpineSkeletonAnimationComponent::AllowAnimationStateMachine()
{
	bCanPlayNormalAnimStateMachine = true;
}




void USpineSkeletonAnimationComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITOR
	if (GetWorld() && (GetWorld()->IsPreviewWorld() || GetWorld()->IsEditorWorld()))
	{
		if (IsValid(PreviewAnimSpec.RelatedSpineSkeletonDataAsset))
		{
			SetSkeletonAsset(PreviewAnimSpec.RelatedSpineSkeletonDataAsset);

			if (PreviewAnimSpec.IsValid())
			{
				SetAnimationWitSpec(PreviewAnimSpec, true);
			}
			else
			{
				SetEmptyAnimation(0, 0);
			}

			SetSkin(PreviewSkinName);

			InternalTick_SkeletonPose(0);

			InternalTick_Renderer();
		}
	}
#endif
}

void USpineSkeletonAnimationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<FUpdateMaterialRuntime> CopyUpdateMaterialRuntimes = UpdateMaterialRuntimes;

	for (const FUpdateMaterialRuntime& Item: CopyUpdateMaterialRuntimes)
	{
		bool bIsCancel = true;
		bool bImmediately = true;
		bool bDisableCallback = false;
		RemoveMaterialParam(Item.Handle, bIsCancel, bImmediately, bDisableCallback);
	}

	Super::EndPlay(EndPlayReason);
}

void USpineSkeletonAnimationComponent::OnSpineAnimStateEventReceived(spine::AnimationState* State, spine::EventType Type, TSharedRef<TrackEntry> Entry, const Event& Event)
{
	USpineSkeletonAnimationComponent* component = CastChecked<USpineSkeletonAnimationComponent>(State->GetRendererObject().Get());
	check(component&&component == this);

	static const FString EmptyAnimName = TEXT("<empty>");

	if (Entry->GetRendererObject().IsValid())
	{
		UTrackEntry* uEntry = CastChecked<UTrackEntry>(Entry->GetRendererObject());
		FString AnimNameInTrackEntry = uEntry->GetAnimationName();

		check(AnimNameInTrackEntry != EmptyAnimName);

		if (Type == EventType_Start)
		{
			component->AnimationStart.Broadcast(uEntry);
			uEntry->AnimationStart.Broadcast(uEntry);
		}
		else if (Type == EventType_Interrupt)
		{
			//不是循环的动画,我们认为Complete就算是结束了,不再触发后续的Interrupt.
			bool bShouldCareEvent = !uEntry->GetTrackEntry()->isComplete() || uEntry->GetLoop();

			if (bShouldCareEvent)
			{
				component->AnimationInterrupt.Broadcast(uEntry);
				uEntry->AnimationInterrupt.Broadcast(uEntry);
			}

		}
		else if (Type == EventType_Event)
		{
			FSpineEvent evt;
			evt.SetEvent(Event);
			component->Internal_OnAnimationEvent(uEntry, evt);
			component->AnimationEvent.Broadcast(uEntry, evt);
			uEntry->AnimationEvent.Broadcast(uEntry, evt);

		}
		else if (Type == EventType_Complete)
		{
			//loop 动画的Complete事件对我们毫无意义.
			if ((uEntry->GetLoop() == false))
			{
				component->AnimationComplete.Broadcast(uEntry);
				uEntry->AnimationComplete.Broadcast(uEntry);
			}
		}
		else if (Type == EventType_End)
		{
			component->AnimationEnd.Broadcast(uEntry);
			uEntry->AnimationEnd.Broadcast(uEntry);
		}
		else if (Type == EventType_Dispose)
		{
			component->AnimationDispose.Broadcast(uEntry);
			uEntry->AnimationDispose.Broadcast(uEntry);

			//uEntry->SetTrackEntry(nullptr);
			component->GCTrackEntry(uEntry);
		}
	}
}



void USpineSkeletonAnimationComponent::SetPaused(bool bPaused)
{
	bIsPaused = bPaused;
}

void USpineSkeletonAnimationComponent::SetPlaybackTime(float InPlaybackTime)
{
	CheckState();

	if (SpineAnimState && SpineAnimState->getCurrent(0))
	{
		check(GetSkeleton());

		TSharedPtr<spine::TrackEntry> CurrentSpineTrack = SpineAnimState->getCurrent(0);
		check(CurrentSpineTrack);

		TSharedPtr<spine::Animation> CurrentAnimation = CurrentSpineTrack->getAnimation();
		check(CurrentAnimation);

		const float CurrentTime = CurrentSpineTrack->getTrackTime();
		InPlaybackTime = FMath::Clamp(InPlaybackTime, 0.0f, CurrentAnimation->getDuration());
		const float DeltaTime = InPlaybackTime - CurrentTime;
		SpineAnimState->update(DeltaTime);
		SpineAnimState->apply(GetSkeleton()->AsShared().Get());


		GetSkeleton()->updateWorldTransform();

	}
}

void USpineSkeletonAnimationComponent::SetTimeScale(float timeScale)
{
	CheckState();
	if (SpineAnimState)
	{
		timeScale = FMath::Max(SMALL_NUMBER, timeScale);
		SpineAnimState->setTimeScale(timeScale);
	}
}

float USpineSkeletonAnimationComponent::GetTimeScale()
{
	CheckState();
	if (SpineAnimState) 
	{
		return SpineAnimState->getTimeScale();
	}
	return 1;
}

UTrackEntry* USpineSkeletonAnimationComponent::SetAnimation(int trackIndex, FString animationName, bool loop)
{
	CheckState();

	spine::String SpineAnimString = TCHAR_TO_UTF8(*animationName);

	if (SpineAnimState.IsValid())
	{
		check(GetSkeleton());

		if (GetSkeleton()->getData()->findAnimation(SpineAnimString).Get())
		{
			SpineAnimState->disableQueue();
			TSharedRef<TrackEntry> entry = SpineAnimState->setAnimation(trackIndex, SpineAnimString, loop)->AsShared();
			FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), UTF8_TO_TCHAR(entry->getAnimation()->getName().buffer()));
			UTrackEntry* uEntry = NewObject<UTrackEntry>(this, *TrackEntryName);
			uEntry->SetTrackEntry(entry);
			trackEntries.Add(uEntry);
			SpineAnimState->enableQueue();

			return uEntry;
		}
	}

	return nullptr;
}

UTrackEntry* USpineSkeletonAnimationComponent::PlayAbilityAnimation(const FSpineAnimationSpec& AnimationSpec, bool loop)
{
	//要求动画状态机释放动画控制
	StopAnimationStateMachine();

	return SetAnimationWitSpec(AnimationSpec, loop);
}

void USpineSkeletonAnimationComponent::StopAbilityAnimation()
{
	if (auto Track=GetCurrent(0))
	{
		Track->SetTrackTime((Track->GetAnimationEnd() - Track->GetAnimationStart()) + SMALL_NUMBER);
	//	SetEmptyAnimation(0, 0);
		//SetPlaybackTime();
	}

	AllowAnimationStateMachine();
}

UTrackEntry* USpineSkeletonAnimationComponent::SetAnimationWitSpec(const FSpineAnimationSpec& AnimationSpec, bool loop)
{
	//UE_LOG(LogTemp, Warning, TEXT("SetAnimationWitSpec %s  "), *AnimationSpec.GetDebugString());

	if (AnimationSpec.IsValid())
	{
		bool bChangedSkeletonData = false;
		if (SkeletonData != AnimationSpec.RelatedSpineSkeletonDataAsset)
		{
			// if changed Skeleton,all current playing animation will trigger interrupt delegate in DisposeState().
			SetSkeletonAsset(AnimationSpec.RelatedSpineSkeletonDataAsset);
			bChangedSkeletonData = true;
		}

		if (UTrackEntry* Track = SetAnimation(0, AnimationSpec.AnimationName, loop))
		{
		//	if (bChangedSkeletonData) //try update.
			{
				check(SpineAnimState.IsValid());
				//SpineAnimState->update(0);
				SpineAnimState->apply(GetSkeleton()->AsShared().Get());
				GetSkeleton()->updateWorldTransform();
			}

			return  Track;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

UTrackEntry* USpineSkeletonAnimationComponent::AddAnimation (int trackIndex, FString animationName, bool loop, float delay) 
{
	CheckState();
	if (GetSkeleton()&& GetSkeleton()->getData()->findAnimation(TCHAR_TO_UTF8(*animationName)).Get())
	{
		SpineAnimState->disableQueue();
		TSharedRef<TrackEntry> entry = SpineAnimState->addAnimation(trackIndex, TCHAR_TO_UTF8(*animationName), loop, delay)->AsShared();

		FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), UTF8_TO_TCHAR(entry->getAnimation()->getName().buffer()));

		UTrackEntry* uEntry = NewObject<UTrackEntry>(this,*TrackEntryName);
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		SpineAnimState->enableQueue();

		return uEntry;
	}
	else
	{
		return nullptr;
	}
}

void USpineSkeletonAnimationComponent::SetEmptyAnimation (int trackIndex, float mixDuration) 
{
	CheckState();
	if (GetSkeleton().IsValid())
	{
		TSharedRef<TrackEntry> entry = SpineAnimState->setEmptyAnimation(trackIndex, mixDuration)->AsShared();
	}
}

void USpineSkeletonAnimationComponent::AddEmptyAnimation (int trackIndex, float mixDuration, float delay) {
	CheckState();
	if (GetSkeleton().IsValid())
	{
		TSharedRef<TrackEntry> entry = SpineAnimState->addEmptyAnimation(trackIndex, mixDuration, delay)->AsShared();
	}
	
}

UTrackEntry* USpineSkeletonAnimationComponent::GetCurrent(int trackIndex)
{
	CheckState();

	if (!GetSkeleton())
	{
		return nullptr;
	}

	TSharedPtr<TrackEntry> entry = SpineAnimState->getCurrent(trackIndex);

	return (entry&& entry->GetRendererObject().IsValid()) ?
		CastChecked<UTrackEntry>(entry->GetRendererObject().Get()) : nullptr;
}

void USpineSkeletonAnimationComponent::ClearTracks ()
{
	CheckState();
	if (GetSkeleton()) 
	{
		SpineAnimState->clearTracks();
	}
}

void USpineSkeletonAnimationComponent::ClearTrack (int trackIndex)
{
	CheckState();
	if (GetSkeleton())
	{
		SpineAnimState->clearTrack(trackIndex);
	}
}

void USpineSkeletonAnimationComponent::UnbindSpineAnimCallbackFrom(UObject* Object)
{
	AnimationStart.RemoveAll(Object);
	AnimationInterrupt.RemoveAll(Object);
	AnimationEvent.RemoveAll(Object);
	AnimationComplete.RemoveAll(Object);
	AnimationEnd.RemoveAll(Object);
	AnimationDispose.RemoveAll(Object);
}

void USpineSkeletonAnimationComponent::Internal_OnAnimationEvent(UTrackEntry* Entry, FSpineEvent Evt)
{
	check(Entry&&Entry->IsValidAnimation());

	const FString AnimName = Entry->GetAnimationName();

	FSpineAnimationSpec UsedAnimSpec = FSpineAnimationSpec{ this->SkeletonData, AnimName };

	FGameplayTag* FoundMatchedTag = DynamicAnimEventListenerMap.Find(FSpineAnimEventDesc(UsedAnimSpec, Evt.Name));

	if (FoundMatchedTag == nullptr)
	{
		int32 FoundPairIndex = this->SkeletonData->AnimEventSetupArray.FindLastByPredicate(
			[&EventName = Evt.Name](const FSpineAnimEventPair& Pair)
		{
			return  Pair.EventName == EventName && Pair.RemapToGameplayEventTag.IsValid();
		});

		if (FoundPairIndex != INDEX_NONE)
		{
			FoundMatchedTag = &this->SkeletonData->AnimEventSetupArray[FoundPairIndex].RemapToGameplayEventTag;
		}
	}


	if (FoundMatchedTag)
	{
		check(FoundMatchedTag->IsValid());

		const static FGameplayTag GameplayCueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue"));

		IAbilitySystemInterface* IASI = Cast<IAbilitySystemInterface>(GetOwner());
		UAbilitySystemComponent* ASC = IASI ? IASI->GetAbilitySystemComponent() : nullptr;

		if (ASC)
		{
			if (FoundMatchedTag->MatchesTagDepth(GameplayCueTag) > 0)
			{
				FGameplayCueParameters CueParameters;
				CueParameters.RawMagnitude = Evt.FloatValue;

				ASC->ExecuteGameplayCue(*FoundMatchedTag, CueParameters);
			}
			else
			{
				FGameplayEventData EventData;
				EventData.EventMagnitude = Evt.FloatValue;
				EventData.OptionalObject = USpineAnimNotifyPayload::CreatePayload(SkeletonData, AnimName, Entry);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), *FoundMatchedTag, EventData);
			}
		}
	}

}

void USpineSkeletonAnimationComponent::AddDynamicAnimEventListener(const FSpineAnimationSpec& AnimDesc, FString AnimEvent, FGameplayTag RemapToTag)
{
	if (ensure(RemapToTag.IsValid()))
	{
		FSpineAnimEventDesc EventDesc(AnimDesc, AnimEvent);
		DynamicAnimEventListenerMap.FindOrAdd(EventDesc) = RemapToTag;
	}
}

void USpineSkeletonAnimationComponent::GCTrackEntry(UTrackEntry* entry)
{
	trackEntries.Remove(entry);
}

#if WITH_EDITOR
void USpineSkeletonAnimationComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetWorld() && (GetWorld()->IsPreviewWorld() || GetWorld()->IsEditorWorld()))
	{
		/*if (PropertyChangedEvent.Property != nullptr &&
			(
				PropertyChangedEvent.MemberProperty->GetFName() ==
				GET_MEMBER_NAME_CHECKED(USpineSkeletonAnimationComponent, PreviewAnimSpec)
				||
				PropertyChangedEvent.MemberProperty->GetFName() ==
				GET_MEMBER_NAME_CHECKED(USpineSkeletonAnimationComponent, DefaultSkinName)
				)
			)
		{
			if (PreviewAnimSpec.IsValid())
			{
				SetAnimationWitSpec(PreviewAnimSpec, true);
			}
			else
			{
				SetEmptyAnimation(0, 0);
			}

			SetSkin(DefaultSkinName);
		}*/
	}
}
#endif



void FSpineEvent::SetEvent(const spine::Event& InEvent)
{
	spine::Event& MyEvent = const_cast<spine::Event&>(InEvent);

	Name = FString(UTF8_TO_TCHAR(MyEvent.getData().getName().buffer()));
	if (!MyEvent.getStringValue().isEmpty())
	{
		StringValue = FString(UTF8_TO_TCHAR(MyEvent.getStringValue().buffer()));
	}
	this->IntValue = MyEvent.getIntValue();
	this->FloatValue = MyEvent.getFloatValue();
	this->Time = MyEvent.getTime();
}




#undef LOCTEXT_NAMESPACE
