


#include "SpineSkeletonAnimationComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"

#include "SpinePlugin.h"
#include "spine/spine.h"

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
	if (entry.IsValid()) entry->setTimeScale(timeScale);
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



USpineSkeletonAnimationComponent::USpineSkeletonAnimationComponent ()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

}

void USpineSkeletonAnimationComponent::BeginPlay() 
{
	Super::BeginPlay();

	trackEntries.Empty();

	AnimationStateMachineOn.Broadcast();
}

void USpineSkeletonAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InternalTick_SkeletonPose(DeltaTime);

	InternalTick_Renderer();
}

void USpineSkeletonAnimationComponent::InternalTick_SkeletonPose(float DeltaTime)
{
	CheckState();

	if (bIsPaused)
	{
		return;
	}

	if (SpineAnimState.IsValid() && ensure(GetSkeleton().IsValid()))
	{
		if (GetWorld()->IsPreviewWorld())
		{
			if (lastPreviewAnimation != PreviewAnimation)
			{
				if (!PreviewAnimation.IsEmpty())
				{
					SetAnimation(0, PreviewAnimation, true);
				}
				else
				{
					SetEmptyAnimation(0, 0);
				}
				lastPreviewAnimation = PreviewAnimation;
			}

			if (lastPreviewSkin != PreviewSkin)
			{
				if (!PreviewSkin.IsEmpty())
				{
					SetSkin(PreviewSkin);
				}
				else
				{
					SetSkin("default");
				}
				lastPreviewSkin = PreviewSkin;
			}
		}

		SpineAnimState->update(DeltaTime);
		SpineAnimState->apply(GetSkeleton()->AsShared().Get());

		GetSkeleton()->updateWorldTransform();
	}
}

void USpineSkeletonAnimationComponent::CheckState()
{
	bool bNeedsUpdate = (lastSkeletonAsset != SkeletonData);

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

				TSharedPtr<spine::AnimationStateData> SpineAnimStateData = SkeletonData->GetAnimationStateData();
				if (SpineAnimStateData.IsValid())
				{
					SpineAnimState = MakeShared<spine::AnimationState>(SpineAnimStateData->AsShared());
					SpineAnimState->SetRendererObject(this);
					SpineAnimState->setListener(FSpineAnimationStateCallbackDelegate::CreateUObject(this, &ThisClass::OnSpineAnimStateEventReceived));
				}

			}
		}

		lastSkeletonAsset = SkeletonData;
	}
}

void USpineSkeletonAnimationComponent::DisposeState ()
{	
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

void USpineSkeletonAnimationComponent::FinishDestroy () {
	DisposeState();
	Super::FinishDestroy();
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

		if (Type == EventType_Start)
		{
			if (AnimNameInTrackEntry != EmptyAnimName)
			{
				component->AnimationStart.Broadcast(uEntry);
				uEntry->AnimationStart.Broadcast(uEntry);
			}
		}
		else if (Type == EventType_Interrupt)
		{
			//不是循环的动画,我们认为Complete就算是结束了,不再触发后续的Interrupt.
			bool bShouldSkipEvent = uEntry->GetTrackEntry()->isComplete() && uEntry->GetLoop() == false;

			if (AnimNameInTrackEntry != EmptyAnimName && !bShouldSkipEvent)
			{
				component->AnimationInterrupt.Broadcast(uEntry);
				uEntry->AnimationInterrupt.Broadcast(uEntry);
			}

		}
		else if (Type == EventType_Event)
		{
			if (AnimNameInTrackEntry != EmptyAnimName)
			{
				FSpineEvent evt;
				evt.SetEvent(Event);
				component->Internal_OnAnimationEvent(uEntry, evt);
				component->AnimationEvent.Broadcast(uEntry, evt);
				uEntry->AnimationEvent.Broadcast(uEntry, evt);
			}

		}
		else if (Type == EventType_Complete)
		{
			//loop 动画的Complete事件对我们毫无意义.
			if ((AnimNameInTrackEntry != EmptyAnimName)
				&& (uEntry->GetLoop() == false))
			{
				component->AnimationComplete.Broadcast(uEntry);
				uEntry->AnimationComplete.Broadcast(uEntry);
			}

		}
		else if (Type == EventType_End)
		{
			if (AnimNameInTrackEntry != EmptyAnimName)
			{
				component->AnimationEnd.Broadcast(uEntry);
				uEntry->AnimationEnd.Broadcast(uEntry);
			}
		}
		else if (Type == EventType_Dispose)
		{
			if (AnimNameInTrackEntry != EmptyAnimName)
			{
				component->AnimationDispose.Broadcast(uEntry);
				uEntry->AnimationDispose.Broadcast(uEntry);
			}

			//		uEntry->SetTrackEntry(nullptr);
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

	String SpineAnimString = TCHAR_TO_UTF8(*animationName);

	if (SpineAnimState.IsValid())
	{
		check(GetSkeleton());

		if (GetSkeleton()->getData()->findAnimation(SpineAnimString).Get())
		{
			//	StateHandle->disableQueue();
			TSharedRef<TrackEntry> entry = SpineAnimState->setAnimation(trackIndex, SpineAnimString, loop)->AsShared();
			//	StateHandle->enableQueue();

			FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), UTF8_TO_TCHAR(entry->getAnimation()->getName().buffer()));

			UTrackEntry* uEntry = NewObject<UTrackEntry>(this, *TrackEntryName);
			uEntry->SetTrackEntry(entry);
			trackEntries.Add(uEntry);
			return uEntry;
		}
	}

	return nullptr;
}

UTrackEntry* USpineSkeletonAnimationComponent::PlayAbilityAnimation(const FSpineAnimationSpec& AnimationSpec, bool loop)
{
	//todo 要求动画状态机释放动画控制
	AnimationStateMachineOff.Broadcast();

	if (GetCurrent(0) != nullptr && GetCurrent(0)->GetAnimationName() != TEXT("<empty>"))
	{
		UE_LOG(SpineLog, Display, TEXT("PlayAbilityAnimation but already has Playing:[%s]"), *GetCurrent(0)->GetAnimationName());

		SetEmptyAnimation(0, 0);
	}

	return SetAnimationWitSpec(AnimationSpec, loop);
}

void USpineSkeletonAnimationComponent::StopAbilityAnimation()
{
	SetEmptyAnimation(0, 0);

	AnimationStateMachineOn.Broadcast();
}

UTrackEntry* USpineSkeletonAnimationComponent::SetAnimationWitSpec(const FSpineAnimationSpec& AnimationSpec, bool loop)
{
	if (AnimationSpec.IsValid())
	{
		bool bChangedSkeletonData = false;
		if (SkeletonData != AnimationSpec.RelatedSpineSkeletonDataAsset)
		{
			SetEmptyAnimation(0, 0);

			SetSkeletonAsset(AnimationSpec.RelatedSpineSkeletonDataAsset);
			bChangedSkeletonData = true;
		}
		UTrackEntry* Track = SetAnimation(0, AnimationSpec.AnimationName, loop);

		if (Track)
		{
			if (bChangedSkeletonData) //try update.
			{
				check(SpineAnimState.IsValid());
				SpineAnimState->update(0);
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
		//SpineAnimState->disableQueue();
		TSharedRef<TrackEntry> entry = SpineAnimState->addAnimation(trackIndex, TCHAR_TO_UTF8(*animationName), loop, delay)->AsShared();
		//SpineAnimState->enableQueue();

		FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), UTF8_TO_TCHAR(entry->getAnimation()->getName().buffer()));

		UTrackEntry* uEntry = NewObject<UTrackEntry>(this,*TrackEntryName);
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
	{
		return nullptr;
	}
}

UTrackEntry* USpineSkeletonAnimationComponent::SetEmptyAnimation (int trackIndex, float mixDuration) 
{
	CheckState();
	if (GetSkeleton().IsValid())
	{
		TSharedRef<TrackEntry> entry = SpineAnimState->setEmptyAnimation(trackIndex, mixDuration)->AsShared();

		FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), TEXT("EmptyAnim"));
		UTrackEntry* uEntry = NewObject<UTrackEntry>(this, *TrackEntryName);

		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
	{
		return nullptr;
	}
}

UTrackEntry* USpineSkeletonAnimationComponent::AddEmptyAnimation (int trackIndex, float mixDuration, float delay) {
	CheckState();
	if (GetSkeleton().IsValid()) {
		TSharedRef<TrackEntry> entry = SpineAnimState->addEmptyAnimation(trackIndex, mixDuration, delay)->AsShared();

		FString TrackEntryName = FString::Printf(TEXT("%s_%s"), TEXT("TrackEntry"), TEXT("EmptyAnim"));

		UTrackEntry* uEntry = NewObject<UTrackEntry>(this, *TrackEntryName);
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
	{
		return nullptr;
	}
}

UTrackEntry* USpineSkeletonAnimationComponent::GetCurrent(int trackIndex)
{
	CheckState();
	if (GetSkeleton())
	{
		TSharedPtr<TrackEntry> entry = SpineAnimState->getCurrent(trackIndex);
		if (!entry)
		{
			return nullptr;
		}
		else if (entry->GetRendererObject().IsValid())
		{
			return CastChecked<UTrackEntry>(entry->GetRendererObject().Get());
		}
		else
		{
			UTrackEntry* uEntry = NewObject<UTrackEntry>();
			uEntry->SetTrackEntry(entry->AsShared());
			trackEntries.Add(uEntry);
			return uEntry;
		}
	}
	else
	{
		return nullptr;
	}
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

#if WITH_EDITOR
void USpineSkeletonAnimationComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
#endif

#undef LOCTEXT_NAMESPACE

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
