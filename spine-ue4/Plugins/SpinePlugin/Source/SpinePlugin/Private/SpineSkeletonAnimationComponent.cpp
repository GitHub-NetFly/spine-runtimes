


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
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;




USpineSkeletonAnimationComponent::USpineSkeletonAnimationComponent (const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	DepthOffset = 0.002;

	Prior = 0;

	bUseComplexAsSimpleCollision = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);


	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/SpinePlugin/Custom/M_Spine_Lit_Masked"));
	NormalBlendMaterial = NormalMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Unit_Additive"));
	AdditiveBlendMaterial = AdditiveMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Unit_Additive"));
	MultiplyBlendMaterial = MultiplyMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ScreenMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Lit_Translucent"));
	ScreenBlendMaterial = ScreenMaterialRef.Object;

	TextureParameterName = FName(TEXT("SpriteTexture"));
	DepthOffset = 0.001f;
	Color = FLinearColor(1, 1, 1, 1);
	bCreateCollision = false;
}

void USpineSkeletonAnimationComponent::BeginPlay()
{
	trackEntries.Empty();

	Super::BeginPlay();

	//ConditionStartAnimationStateMachine();
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
		UE_LOG(SpineLog, Display, TEXT("USpineSkeletonAnimationComponent::CheckState Force Refresh!!!!!!!!!!!!!!!"));
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

	CurrentSpineSkeleton.Reset();
	lastSkeletonData.Reset();
}





void USpineSkeletonAnimationComponent::SetSkeletonAsset(USpineSkeletonDataAsset* InSkeletonAsset)
{
	if (IsValid(InSkeletonAsset) && InSkeletonAsset != SkeletonData)
	{
		SkeletonData = InSkeletonAsset;
		CheckState();
		OnSpineSkeletonUpdated.Broadcast();
		SetSkin(CurrentSkinName);
	}
}

bool USpineSkeletonAnimationComponent::Internal_SetSkin(FString SkinName)
{
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		TSharedPtr<Skin> NewSkin = CurrentSpineSkeleton->getData()->findSkin(SkinName);
		if (!NewSkin)
		{
			return false;
		}

		TSharedPtr<Skin> OldSkin = CurrentSpineSkeleton->getSkin();

		if (OldSkin == NewSkin)
		{
			return false;
		}

		CurrentSpineSkeleton->setSkin(NewSkin);
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

	UWorld* World = GetWorld();

#if WITH_EDITOR
	if (World && (World->IsPreviewWorld() || World->IsEditorWorld()))
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

	if (World && World->IsGameWorld())
	{
		SetSkin(defaultSkinName);
	}
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

	DisposeState();

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

void USpineSkeletonAnimationComponent::SetSkin(FString SkinName)
{
	CurrentSkinName = SkinName;

	TSet<FString> AllValidSkinNameSet = GetAllValidSkinNames();
	if (nullptr == AllValidSkinNameSet.Find(SkinName))
	{
		return;
	}

	Internal_SetSkin(CurrentSkinName);
}

TSet<FString> USpineSkeletonAnimationComponent::GetAllValidSkinNames() const
{
	const_cast<ThisClass*>(this)->CheckState();
	TSet<FString> Result;

	if (CurrentSpineSkeleton.IsValid())
	{
		for (auto SkinObjPtr : CurrentSpineSkeleton->getData()->getSkins())
		{
			check(SkinObjPtr.IsValid());
			Result.Add(SkinObjPtr->getName());
		}
	}
	return Result;
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





void USpineSkeletonAnimationComponent::GetSkins(TArray<FString> &Skins) const
{
	const_cast<ThisClass*>(this)->CheckState();

	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getData()->getSkins().Num(); i < n; i++) {
			Skins.Add(CurrentSpineSkeleton->getData()->getSkins()[i]->getName());
		}
	}
}

bool USpineSkeletonAnimationComponent::HasSkin(FString skinName)const
{
	const_cast<ThisClass*>(this)->CheckState();

	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findSkin(skinName).IsValid();
	}
	return false;
}

bool USpineSkeletonAnimationComponent::SetAttachment(const FString slotName, const FString attachmentName) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		if (!CurrentSpineSkeleton->getAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName)))
		{
			return false;
		}
		CurrentSpineSkeleton->setAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName));
		return true;
	}
	return false;
}

bool USpineSkeletonAnimationComponent::GetBoneWorldTransform(const FString& BoneName, FTransform& OutTransform)const
{
	OutTransform = FTransform::Identity;

	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		spine::Bone* bone = CurrentSpineSkeleton->findBone(TCHAR_TO_UTF8(*BoneName));
		if (!bone)
		{

			return false;
		}
		if (!bone->isAppliedValid())
		{
			const_cast<ThisClass*>(this)->InternalTick_SkeletonPose(0);
		}

		// Need to fetch the renderer component to get world transform of actor plus
		// offset by renderer component and its parent component(s). If no renderer
		// component is found, this components owner's transform is used as a fallback
		FTransform baseTransform = GetComponentTransform();
		FVector position(bone->getWorldX(), 0, bone->getWorldY());
		FMatrix localTransform;
		localTransform.SetIdentity();
		localTransform.SetAxis(2, FVector(bone->getA(), 0, bone->getC()));
		localTransform.SetAxis(0, FVector(bone->getB(), 0, bone->getD()));
		localTransform.SetOrigin(FVector(bone->getWorldX(), 0, bone->getWorldY()));
		localTransform = localTransform * baseTransform.ToMatrixWithScale();

		FTransform result;
		result.SetFromMatrix(localTransform);

		OutTransform = result;
		return true;
	}
	return false;
}

//void USpineSkeletonAnimationComponent::SetBoneWorldPosition (const FString& BoneName, const FVector& position) 
//{
//	CheckState();
//	if (CurrentSpineSkeleton.IsValid())
//	{
//		Bone* bone = CurrentSpineSkeleton->findBone(TCHAR_TO_UTF8(*BoneName));
//		if (!bone) return;
//		if (!bone->isAppliedValid()) this->InternalTick_SkeletonPose(0, false);
//
//		// Need to fetch the renderer component to get world transform of actor plus
//		// offset by renderer component and its parent component(s). If no renderer
//		// component is found, this components owner's transform is used as a fallback
//		FTransform baseTransform;
//		AActor* owner = GetOwner();
//		if (owner) {
//			USpineSkeletonRendererComponent* rendererComponent = static_cast<USpineSkeletonRendererComponent*>(owner->GetComponentByClass(USpineSkeletonRendererComponent::StaticClass()));
//			if (rendererComponent) baseTransform = rendererComponent->GetComponentTransform();
//			else baseTransform = owner->GetActorTransform();
//		}
//
//		baseTransform = baseTransform.Inverse();
//		FVector localPosition = baseTransform.TransformPosition(position);
//		float localX = 0, localY = 0;
//		if (bone->getParent()) {
//			bone->getParent()->worldToLocal(localPosition.X, localPosition.Z, localX, localY);
//		} else {
//			bone->worldToLocal(localPosition.X, localPosition.Z, localX, localY);
//		}
//		bone->setX(localX);
//		bone->setY(localY);
//	}
//}

void USpineSkeletonAnimationComponent::UpdateWorldTransform() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->updateWorldTransform();
	}
}

void USpineSkeletonAnimationComponent::SetToSetupPose() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setToSetupPose();
	}
}

void USpineSkeletonAnimationComponent::SetBonesToSetupPose() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setBonesToSetupPose();
	}


}

void USpineSkeletonAnimationComponent::SetSlotsToSetupPose() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setSlotsToSetupPose();
	}
}

void USpineSkeletonAnimationComponent::SetScaleX(float scaleX) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setScaleX(scaleX);
	}
}

float USpineSkeletonAnimationComponent::GetScaleX() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getScaleX();
	}
	return 1;
}

void USpineSkeletonAnimationComponent::SetScaleY(float scaleY) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setScaleY(scaleY);
	}
}

float USpineSkeletonAnimationComponent::GetScaleY() {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getScaleY();
	}
	return 1;
}

void USpineSkeletonAnimationComponent::GetBones(TArray<FName> &BoneNames) const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getBones().size(); i < n; i++)
		{
			BoneNames.Add(CurrentSpineSkeleton->getBones()[i]->getData().getName());
		}
	}
}

bool USpineSkeletonAnimationComponent::HasBone(FName BoneName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findBone(BoneName) != nullptr;
	}
	return false;
}

void USpineSkeletonAnimationComponent::GetSlots(TArray<FString> &Slots)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getSlots().Num(); i < n; i++) {
			Slots.Add(CurrentSpineSkeleton->getSlots()[i]->getData().getName());
		}
	}
}

bool USpineSkeletonAnimationComponent::HasSlot(FString SlotName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findSlot(TCHAR_TO_UTF8(*SlotName)) != nullptr;
	}
	return false;
}

void USpineSkeletonAnimationComponent::GetAnimations(TArray<FString> &Animations)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getData()->getAnimations().Num(); i < n; i++) {
			Animations.Add(CurrentSpineSkeleton->getData()->getAnimations()[i]->getName().buffer());
		}
	}
}

bool USpineSkeletonAnimationComponent::HasAnimation(FString AnimationName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName)).Get() != nullptr;
	}
	return false;
}

float USpineSkeletonAnimationComponent::GetAnimationDuration(FString AnimationName) const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		Animation *animation = CurrentSpineSkeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName)).Get();
		if (animation == nullptr) return 0;
		else return animation->getDuration();
	}
	return 0;
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

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}








void USpineSkeletonAnimationComponent::InternalTick_Renderer()
{
	if (IsValid(SkeletonData) && IsValid(SkeletonData->RelatedAtlasAsset) &&
		SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid() &&
		GetSkeleton().IsValid())
	{
		USpineAtlasAsset* CurrentAtlasAsset = SkeletonData->RelatedAtlasAsset;
		USpineAtlasAsset* LastAtlasAsset = IsValid(LastSkeletonAsset) ? LastSkeletonAsset->RelatedAtlasAsset : nullptr;

		GetSkeleton()->getColor().set(Color.R, Color.G, Color.B, Color.A);

		pageToNormalBlendMaterial.Empty(6);
		pageToAdditiveBlendMaterial.Empty(6);
		pageToMultiplyBlendMaterial.Empty(6);
		pageToScreenBlendMaterial.Empty(6);

		if (LastAtlasAsset != CurrentAtlasAsset || atlasNormalBlendMaterials.Num() != CurrentAtlasAsset->GetAtlas()->getPages().Num())
		{
			atlasNormalBlendMaterials.Empty(6);
			atlasAdditiveBlendMaterials.Empty(6);
			atlasMultiplyBlendMaterials.Empty(6);
			atlasScreenBlendMaterials.Empty(6);

			for (const spine::AtlasPage& Page : CurrentAtlasAsset->GetAtlas()->getPages())
			{
				UTexture2D* Texture = Page.GetRendererObject().Get();
				ensure(IsValid(Texture));

				UMaterialInstanceDynamic* NormalMID = UMaterialInstanceDynamic::Create(NormalBlendMaterial, this);
				UMaterialInstanceDynamic* AdditiveMID = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, this);
				UMaterialInstanceDynamic* MultiplyMID = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, this);
				UMaterialInstanceDynamic* ScreenMID = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, this);

				NormalMID->SetTextureParameterValue(TextureParameterName, Texture);
				AdditiveMID->SetTextureParameterValue(TextureParameterName, Texture);
				MultiplyMID->SetTextureParameterValue(TextureParameterName, Texture);
				ScreenMID->SetTextureParameterValue(TextureParameterName, Texture);

				atlasNormalBlendMaterials.Add(NormalMID);
				atlasAdditiveBlendMaterials.Add(AdditiveMID);
				atlasMultiplyBlendMaterials.Add(MultiplyMID);
				atlasScreenBlendMaterials.Add(ScreenMID);

				pageToNormalBlendMaterial.Add(Page, NormalMID);
				pageToAdditiveBlendMaterial.Add(Page, AdditiveMID);
				pageToMultiplyBlendMaterial.Add(Page, MultiplyMID);
				pageToScreenBlendMaterial.Add(Page, ScreenMID);
			}
		}
		else
		{
			for (int32 i = 0; i < CurrentAtlasAsset->GetAtlas()->getPages().Num(); i++)
			{
				const spine::AtlasPage& PageRef = CurrentAtlasAsset->GetAtlas()->getPages()[i];

				UTexture2D* texture = PageRef.GetRendererObject().Get();
				UTexture* oldTexture = nullptr;

				UMaterialInstanceDynamic* CurrentNormalMID = atlasNormalBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentAdditiveMID = atlasAdditiveBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentMultiplyMID = atlasMultiplyBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentScreenMID = atlasScreenBlendMaterials[i];

				if (!CurrentNormalMID || !CurrentNormalMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(NormalBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasNormalBlendMaterials[i] = material;
				}
				//pageToNormalBlendMaterial.Add(PageRef, atlasNormalBlendMaterials[i]);

				if (!CurrentAdditiveMID || !CurrentAdditiveMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasAdditiveBlendMaterials[i] = material;
				}

				if (!CurrentMultiplyMID || !CurrentMultiplyMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasMultiplyBlendMaterials[i] = material;
				}

				if (!CurrentScreenMID || !CurrentScreenMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasScreenBlendMaterials[i] = material;
				}

				pageToNormalBlendMaterial.Add(PageRef, atlasNormalBlendMaterials[i]);
				pageToAdditiveBlendMaterial.Add(PageRef, atlasAdditiveBlendMaterials[i]);
				pageToMultiplyBlendMaterial.Add(PageRef, atlasMultiplyBlendMaterials[i]);
				pageToScreenBlendMaterial.Add(PageRef, atlasScreenBlendMaterials[i]);

			}
		}
		UpdateMesh(GetSkeleton()->AsShared());
	}
	else
	{
		ClearAllMeshSections();
	}
}

void USpineSkeletonAnimationComponent::UpdateMesh(TSharedRef<class spine::Skeleton> SkeletonRef)
{
	TArray<FVector> vertices;
	TArray<int32> indices;
	TArray<FVector> normals;
	TArray<FVector2D> uvs;
	TArray<FColor> colors;
	TArray<FVector> darkColors;

	int idx = 0;
	int meshSection = 0;
	UMaterialInstanceDynamic* lastMaterial = nullptr;

	ClearAllMeshSections();

	// Early out if skeleton is invisible
	if (GetSkeleton()->getColor().a == 0) return;

	struct FStaticSpineVector
	{

		spine::Vector<float> worldVertices;

		FStaticSpineVector()
		{
			worldVertices.ensureCapacity(1024 * 2);
		}
	};

	static FStaticSpineVector StaticSpineVector;

	spine::SkeletonClipping clipper;



	float depthOffset = 0;
	unsigned short quadIndices[] = { 0, 1, 2, 0, 2, 3 };

	const int32 SlotNum = GetSkeleton()->getSlots().Num();
	for (int32 i = 0; i < SlotNum; ++i) {
		Vector<float> &attachmentVertices = StaticSpineVector.worldVertices;
		unsigned short* attachmentIndices = nullptr;
		int numVertices;
		int numIndices;
		TSharedPtr<AtlasRegion>  attachmentAtlasRegion;
		spine::Color attachmentColor;
		attachmentColor.set(1, 1, 1, 1);
		float* attachmentUvs = nullptr;

		TSharedPtr<Slot> slot = GetSkeleton()->getDrawOrder()[i];
		Attachment* attachment = slot->getAttachment();

		if (slot->getColor().a == 0 || !slot->getBone().isActive()) {
			clipper.clipEnd(*slot);
			continue;
		}

		if (!attachment)
		{
			clipper.clipEnd(*slot);
			continue;
		}

		if (!attachment->getRTTI().isExactly(RegionAttachment::rtti) && !attachment->getRTTI().isExactly(MeshAttachment::rtti) && !attachment->getRTTI().isExactly(ClippingAttachment::rtti))
		{
			clipper.clipEnd(*slot);
			continue;
		}

		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			RegionAttachment* regionAttachment = (RegionAttachment*)attachment;

			// Early out if region is invisible
			if (regionAttachment->getColor().a == 0) {
				clipper.clipEnd(*slot);
				continue;
			}

			attachmentColor.set(regionAttachment->getColor());
			attachmentAtlasRegion = regionAttachment->AttachmentAtlasRegion;
			regionAttachment->computeWorldVertices(slot->getBone(), attachmentVertices, 0, 2);
			attachmentIndices = quadIndices;
			attachmentUvs = regionAttachment->getUVs().GetData();
			numVertices = 4;
			numIndices = 6;
		}
		else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			MeshAttachment* mesh = (MeshAttachment*)attachment;

			// Early out if region is invisible
			if (mesh->getColor().a == 0) {
				clipper.clipEnd(*slot);
				continue;
			}

			attachmentColor.set(mesh->getColor());
			attachmentAtlasRegion = mesh->AttachmentAtlasRegion;
			mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), attachmentVertices, 0, 2);
			attachmentIndices = mesh->getTriangles().buffer();
			attachmentUvs = mesh->getUVs().buffer();
			numVertices = mesh->getWorldVerticesLength() >> 1;
			numIndices = mesh->getTriangles().size();
		}
		else /* clipping */ {
			ClippingAttachment* clip = (ClippingAttachment*)attachment;
			clipper.clipStart(*slot, clip);
			continue;
		}

		// if the user switches the atlas data while not having switched
		// to the correct skeleton data yet, we won't find any regions.
		// ignore regions for which we can't find a material
		UMaterialInstanceDynamic* material = nullptr;
		const auto& AtlasPage = attachmentAtlasRegion->Page;

		switch (slot->getData().getBlendMode()) {
		case BlendMode_Normal:
			if (!pageToNormalBlendMaterial.Contains(AtlasPage))
			{
				clipper.clipEnd(*slot);
				continue;
			}
			material = pageToNormalBlendMaterial[AtlasPage];
			break;
		case BlendMode_Additive:
			if (!pageToAdditiveBlendMaterial.Contains(AtlasPage))
			{
				clipper.clipEnd(*slot);
				continue;
			}
			material = pageToAdditiveBlendMaterial[AtlasPage];
			break;
		case BlendMode_Multiply:
			if (!pageToMultiplyBlendMaterial.Contains(AtlasPage))
			{
				clipper.clipEnd(*slot);
				continue;
			}
			material = pageToMultiplyBlendMaterial[AtlasPage];
			break;
		case BlendMode_Screen:
			if (!pageToScreenBlendMaterial.Contains(AtlasPage))
			{
				clipper.clipEnd(*slot);
				continue;
			}
			material = pageToScreenBlendMaterial[AtlasPage];
			break;
		default:
			if (!pageToNormalBlendMaterial.Contains(AtlasPage))
			{
				clipper.clipEnd(*slot);
				continue;
			}
			material = pageToNormalBlendMaterial[AtlasPage];
		}

		if (clipper.isClipping()) {
			clipper.clipTriangles(attachmentVertices.buffer(), attachmentIndices, numIndices, attachmentUvs, 2);
			attachmentVertices = clipper.getClippedVertices();
			numVertices = clipper.getClippedVertices().size() >> 1;
			attachmentIndices = clipper.getClippedTriangles().buffer();
			numIndices = clipper.getClippedTriangles().size();
			attachmentUvs = clipper.getClippedUVs().buffer();
			if (clipper.getClippedTriangles().size() == 0)
			{
				clipper.clipEnd(*slot);
				continue;
			}
		}

		if (lastMaterial != material) {
			Flush(meshSection, vertices, indices, normals, uvs, colors, darkColors, lastMaterial);
			lastMaterial = material;
			idx = 0;
		}

		SetMaterial(meshSection, material);

		uint8 r = static_cast<uint8>(GetSkeleton()->getColor().r * slot->getColor().r * attachmentColor.r * 255);
		uint8 g = static_cast<uint8>(GetSkeleton()->getColor().g * slot->getColor().g * attachmentColor.g * 255);
		uint8 b = static_cast<uint8>(GetSkeleton()->getColor().b * slot->getColor().b * attachmentColor.b * 255);
		uint8 a = static_cast<uint8>(GetSkeleton()->getColor().a * slot->getColor().a * attachmentColor.a * 255);

		float dr = slot->hasDarkColor() ? slot->getDarkColor().r : 0.0f;
		float dg = slot->hasDarkColor() ? slot->getDarkColor().g : 0.0f;
		float db = slot->hasDarkColor() ? slot->getDarkColor().b : 0.0f;

		float* verticesPtr = attachmentVertices.buffer();
		for (int j = 0; j < numVertices << 1; j += 2) {
			colors.Add(FColor(r, g, b, a));
			darkColors.Add(FVector(dr, dg, db));
			vertices.Add(FVector(verticesPtr[j], depthOffset, verticesPtr[j + 1]));
			normals.Add(FVector(0, -1, 0));
			uvs.Add(FVector2D(attachmentUvs[j], attachmentUvs[j + 1]));
		}

		for (int j = 0; j < numIndices; j++) {
			indices.Add(idx + attachmentIndices[j]);
		}

		idx += numVertices;
		depthOffset += this->DepthOffset;

		clipper.clipEnd(*slot);
	}


	//UE_LOG(LogTemp, Warning, TEXT("depthOffset: %f"), depthOffset);

	Flush(meshSection, vertices, indices, normals, uvs, colors, darkColors, lastMaterial);
	clipper.clipEnd();
}

void USpineSkeletonAnimationComponent::Flush(int &Idx, TArray<FVector> &Vertices, TArray<int32> &Indices, TArray<FVector> &Normals, TArray<FVector2D> &Uvs, TArray<FColor> &Colors, TArray<FVector> &Colors2, UMaterialInstanceDynamic* Material)
{
	if (Vertices.Num() == 0) return;
	SetMaterial(Idx, Material);

	CreateMeshSection(Idx, Vertices, Indices, Normals, Uvs, Colors, TArray<FProcMeshTangent>(), bCreateCollision);

	Vertices.SetNum(0);
	Indices.SetNum(0);
	Normals.SetNum(0);
	Uvs.SetNum(0);
	Colors.SetNum(0);
	Colors2.SetNum(0);
	Idx++;
}




FTransform USpineSkeletonAnimationComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace /*= RTS_World*/) const
{
	FTransform WorldSocketTransform;

	if (!GetBoneWorldTransform(InSocketName.ToString(), WorldSocketTransform))
	{
		WorldSocketTransform = GetComponentTransform();
	}

	switch (TransformSpace)
	{
	case RTS_World:
	{
		return WorldSocketTransform;
		break;
	}
	case RTS_Actor:
	{
		if (const AActor* Actor = GetOwner())
		{
			return WorldSocketTransform.GetRelativeTransform(Actor->GetTransform());
		}
		break;
	}
	case RTS_Component:
	{
		return WorldSocketTransform.GetRelativeTransform(GetComponentTransform());
	}
	}

	return WorldSocketTransform;
}

bool USpineSkeletonAnimationComponent::HasAnySockets() const
{
	if (GetSkeleton().IsValid() == false)
	{
		return false;
	}
	return GetSkeleton()->getBones().size() > 0;
}

void USpineSkeletonAnimationComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	//Super::QuerySupportedSockets(OutSockets);

	if (GetSkeleton().IsValid() == false)
	{
		return;
	}

	for (int32 i = 0; i < GetSkeleton()->getBones().size(); i++)
	{
		spine::Bone* Bone = GetSkeleton()->getBones()[i];
		FName SocketName = Bone->getData().getName();
		new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
	}
}

bool USpineSkeletonAnimationComponent::DoesSocketExist(FName InSocketName) const
{
	if (GetSkeleton().IsValid() == false)
	{
		return false;
	}

	spine::Bone* Bone = GetSkeleton()->findBone(TCHAR_TO_UTF8(*InSocketName.GetPlainNameString()));

	return Bone != nullptr;
}

bool USpineSkeletonAnimationComponent::ApplyReplaceAttachment(USpineAttachmentOverrideConfigAsset* OverrideConfigAsset)
{
	CheckState();

	if (!OverrideConfigAsset)
	{
		return false;
	}

	if (!CurrentSpineSkeleton)
	{
		return false;
	}

	if (!ensure(SkeletonData))
	{
		return false;
	}

	if (!ensure(SkeletonData->RelatedAtlasAsset))
	{
		return false;
	}

	check(CurrentSpineSkeleton.IsValid());
	check(SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid());

	TSharedPtr< spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
	check(SpineSkeletonData.IsValid());
	TSharedPtr< spine::Skin> FoundSkin = CurrentSpineSkeleton->getSkin(); /*SpineSkeletonData->findSkin(GetCurrentSkinName());*/

	if (!FoundSkin)
	{
		return false;
	}

	// 拷贝一份避免替换skin中的attachment时会影响原来的Skin ,同名确保了可以直接通过名字找回原来的默认版本的skin.
	TSharedPtr< spine::Skin> CopyedSkin = FoundSkin->GetClone(FoundSkin->getName());

	bool bAnyReplaced = false;

	for (const FReplaceSkinAttachmentDesc& OneReplace : OverrideConfigAsset->Replacements)
	{
		int32 SlotIndex = INDEX_NONE;

		TSharedPtr<spine::Attachment> NewAttachment = OneReplace.GenerateCopyAttachmentWithNewAtlasRegionIn(
			*CurrentSpineSkeleton,
			*FoundSkin,
			*SkeletonData->RelatedAtlasAsset->GetAtlas(),
			SlotIndex);

		if (!NewAttachment)
		{
			continue;
		}

		check(SlotIndex > INDEX_NONE);

		//override Attachment.
		CopyedSkin->setAttachment(SlotIndex, OneReplace.AttachmentName, NewAttachment);
		bAnyReplaced = true;
	}

	if (bAnyReplaced)
	{
		CurrentSpineSkeleton->setSkin(CopyedSkin);
		CurrentSpineSkeleton->setSlotsToSetupPose();

		if (SpineAnimState)
		{
			SpineAnimState->apply(GetSkeleton()->AsShared().Get());
		}

		return bAnyReplaced;
	}

	return bAnyReplaced;
}


bool USpineSkeletonAnimationComponent::CancelReplaceAttachment(USpineAttachmentOverrideConfigAsset* OverrideConfigAsset)
{
	CheckState();

	if (!OverrideConfigAsset)
	{
		return false;
	}

	if (!CurrentSpineSkeleton)
	{
		return false;
	}

	if (!ensure(SkeletonData))
	{
		return false;
	}

	if (!ensure(SkeletonData->RelatedAtlasAsset))
	{
		return false;
	}

	check(CurrentSpineSkeleton.IsValid());
	check(SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid());

	TSharedPtr< spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
	check(SpineSkeletonData.IsValid());

	TSharedPtr< spine::Skin> CurrentSkin = CurrentSpineSkeleton->getSkin(); //因为拷贝的skin和原来的skin同名,所以可以通过名字找到原来的,

	TSharedPtr< spine::Skin> FoundOriginSkin = CurrentSkin ? SpineSkeletonData->findSkin(CurrentSkin->getName()) : nullptr;

	if (!FoundOriginSkin)
	{
		return false;
	}

	TSharedPtr< spine::Skin> CopyedSkin = CurrentSkin->GetClone(CurrentSkin->getName());

	bool bAnyReplaced = false;
	for (const FReplaceSkinAttachmentDesc& OneReplace : OverrideConfigAsset->Replacements)
	{
		int32 SlotIndex = INDEX_NONE;

		TSharedPtr<spine::Attachment> OriginAttachment = OneReplace.FindOriginAttachment(
			*CurrentSpineSkeleton,
			*FoundOriginSkin,
			*SkeletonData->RelatedAtlasAsset->GetAtlas(),
			SlotIndex
		);

		if (!OriginAttachment)
		{
			continue;
		}

		CopyedSkin->setAttachment(SlotIndex, OneReplace.AttachmentName, OriginAttachment);
		bAnyReplaced = true;
	}

	if (bAnyReplaced)
	{
		CurrentSpineSkeleton->setSkin(CopyedSkin);
		CurrentSpineSkeleton->setSlotsToSetupPose();

		if (SpineAnimState)
		{
			SpineAnimState->apply(GetSkeleton()->AsShared().Get());
		}

	}
	return bAnyReplaced;
}




#undef LOCTEXT_NAMESPACE
