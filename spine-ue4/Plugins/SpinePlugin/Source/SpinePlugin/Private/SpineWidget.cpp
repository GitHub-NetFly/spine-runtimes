#include "SpineWidget.h"
#include "SSpineWidget.h"
#include "Engine.h"
#include "spine/spine.h"

#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;



//void callbackWidget(AnimationState* state, spine::EventType type, TrackEntry* entry, Event* event) {
void callbackWidget(spine::AnimationState* state, enum spine::EventType type, TSharedRef<spine::TrackEntry> entry, const spine::Event& event){
	USpineWidget* component = (USpineWidget*)state->GetRendererObject().Get();

	if (entry->GetRendererObject().Get()) {
		UTrackEntry* uEntry = (UTrackEntry*)entry->GetRendererObject().Get();
		if (type == EventType_Start) {
			component->AnimationStart.Broadcast(uEntry);
			uEntry->AnimationStart.Broadcast(uEntry);
		}
		else if (type == EventType_Interrupt) {
			component->AnimationInterrupt.Broadcast(uEntry);
			uEntry->AnimationInterrupt.Broadcast(uEntry);
		}
		else if (type == EventType_Event) {
			FSpineEvent evt;
			evt.SetEvent(event);
			component->AnimationEvent.Broadcast(uEntry, evt);
			uEntry->AnimationEvent.Broadcast(uEntry, evt);
		}
		else if (type == EventType_Complete) {
			component->AnimationComplete.Broadcast(uEntry);
			uEntry->AnimationComplete.Broadcast(uEntry);
		}
		else if (type == EventType_End) {
			component->AnimationEnd.Broadcast(uEntry);
			uEntry->AnimationEnd.Broadcast(uEntry);
		}
		else if (type == EventType_Dispose) {
			component->AnimationDispose.Broadcast(uEntry);
			uEntry->AnimationDispose.Broadcast(uEntry);
			uEntry->SetTrackEntry(nullptr);
			component->GCTrackEntry(uEntry);
		}
	}
}

USpineWidget::USpineWidget(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) {
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/SpinePlugin/UI/UI_SpineUnlitNormalMaterial"));
	NormalBlendMaterial = NormalMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterialRef(TEXT("/SpinePlugin/UI/UI_SpineUnlitAdditiveMaterial"));
	AdditiveBlendMaterial = AdditiveMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterialRef(TEXT("/SpinePlugin/UI/UI_SpineUnlitMultiplyMaterial"));
	MultiplyBlendMaterial = MultiplyMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ScreenMaterialRef(TEXT("/SpinePlugin/UI/UI_SpineUnlitScreenMaterial"));
	ScreenBlendMaterial = ScreenMaterialRef.Object;

	TextureParameterName = FName(TEXT("SpriteTexture"));

	worldVertices.ensureCapacity(1024 * 2);

	bAutoPlaying = true;
}

void USpineWidget::SynchronizeProperties() {
	Super::SynchronizeProperties();

	if (slateWidget.IsValid()) {
		CheckState();
		if (skeleton) {
			Tick(0, false);
			slateWidget->SetData(this);
		} else {
			slateWidget->SetData(nullptr);
		}
		trackEntries.Empty();
	}
}

void USpineWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);
	slateWidget.Reset();
}

TSharedRef<SWidget> USpineWidget::RebuildWidget() {
	this->slateWidget = SNew(SSpineWidget);
	return this->slateWidget.ToSharedRef();
}

#if WITH_EDITOR
const FText USpineWidget::GetPaletteCategory() {
	return LOCTEXT("Spine", "Spine");
}
#endif

void USpineWidget::Tick(float DeltaTime, bool CallDelegates) {
	CheckState();

	if (state && bAutoPlaying) {
		state->update(DeltaTime);
		state->apply(*skeleton);
		if (CallDelegates) BeforeUpdateWorldTransform.Broadcast(this);
		skeleton->updateWorldTransform();
		if (CallDelegates) AfterUpdateWorldTransform.Broadcast(this);
	}
}

void USpineWidget::CheckState() {
	bool needsUpdate = lastAtlas != Atlas || lastData != SkeletonData;

	if (!needsUpdate) {
		// Are we doing a re-import? Then check if the underlying spine-cpp data
		// has changed.
		if (lastAtlas && lastAtlas == Atlas && lastData && lastData == SkeletonData) {
			spine::Atlas* atlas = Atlas->GetAtlas().Get();
			if (lastSpineAtlas != atlas) {
				needsUpdate = true;
			}
			//if (skeleton && skeleton->getData() != SkeletonData->GetSkeletonData(atlas)) {
			if (skeleton && skeleton->getData() != SkeletonData->GetSpineSkeletonData()) {
				needsUpdate = true;
			}
		}
	}

	if (needsUpdate) {
		DisposeState();

		if (Atlas && SkeletonData) {
			TSharedPtr<spine::SkeletonData> data = SkeletonData->GetSpineSkeletonData();//SkeletonData->GetSkeletonData(Atlas->GetAtlas());
			if (data) {
				skeleton =  new spine::Skeleton(data->AsShared()); // new new (__FILE__, __LINE__)
				TSharedPtr<AnimationStateData> stateData = SkeletonData->CreateAnimationStateData();  //GetAnimationStateData(Atlas->GetAtlas());
				state = new AnimationState(stateData->AsShared()); // new (__FILE__, __LINE__) 
				state->SetRendererObject(TWeakObjectPtr<UObject>(this));
				state->setListener(FSpineAnimationStateCallbackDelegate::CreateStatic(&callbackWidget)); // not using create UOBject
				trackEntries.Empty();
			}
		}

		lastAtlas = Atlas;
		lastSpineAtlas = Atlas ? Atlas->GetAtlas().Get() : nullptr;
		lastData = SkeletonData;
	}
}

void USpineWidget::DisposeState() {
	if (state) {
		delete state;
		state = nullptr;
	}

	if (skeleton) {
		delete skeleton;
		skeleton = nullptr;
	}

	if (customSkin) {
		delete customSkin;
		customSkin = nullptr;
	}

	trackEntries.Empty();
}

void USpineWidget::FinishDestroy() {
	DisposeState();
	Super::FinishDestroy();
}

bool USpineWidget::SetSkin(const FString skinName) {
	CheckState();
	if (skeleton) {
		TSharedPtr<spine::Skin> skin = skeleton->getData()->findSkin(TCHAR_TO_UTF8(*skinName));
		if (!skin) return false;
		skeleton->setSkin(skin);
		return true;
	}
	else return false;
}

bool USpineWidget::SetSkins(UPARAM(ref) TArray<FString>& SkinNames) {
	CheckState();
	if (skeleton) {	
		spine::Skin* newSkin = new spine::Skin("__spine-ue3_custom_skin");
		for (auto& skinName : SkinNames) {
			TSharedPtr<spine::Skin> skin = skeleton->getData()->findSkin(TCHAR_TO_UTF8(*skinName));
			if (!skin) {
				delete newSkin;
				return false;
			}
			newSkin->addSkin(skin.Get());
		}
		skeleton->setSkin(TSharedPtr<spine::Skin>(newSkin));
		if (customSkin != nullptr) {
			delete customSkin;
		}
		customSkin = newSkin;
		return true;
	}
	else return false;
}

void USpineWidget::GetSkins(TArray<FString> &Skins) {
	CheckState();
	if (skeleton) {
		for (size_t i = 0, n = skeleton->getData()->getSkins().Num(); i < n; i++) {
			Skins.Add(skeleton->getData()->getSkins()[i]->getName());
		}
	}
}

bool USpineWidget::HasSkin(const FString skinName) {
	CheckState();
	if (skeleton) {
		return skeleton->getData()->findAnimation(TCHAR_TO_UTF8(*skinName)) != nullptr;
	}
	return false;
}

bool USpineWidget::SetAttachment(const FString slotName, const FString attachmentName) {
	CheckState();
	if (skeleton) {
		if (!skeleton->getAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName))) return false;
		skeleton->setAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName));
		return true;
	}
	return false;
}

void USpineWidget::UpdateWorldTransform() {
	CheckState();
	if (skeleton) {
		skeleton->updateWorldTransform();
	}
}

void USpineWidget::SetToSetupPose() {
	CheckState();
	if (skeleton) skeleton->setToSetupPose();
}

void USpineWidget::SetBonesToSetupPose() {
	CheckState();
	if (skeleton) skeleton->setBonesToSetupPose();
}

void USpineWidget::SetSlotsToSetupPose() {
	CheckState();
	if (skeleton) skeleton->setSlotsToSetupPose();
}

void USpineWidget::SetScaleX(float scaleX) {
	CheckState();
	if (skeleton) skeleton->setScaleX(scaleX);
}

float USpineWidget::GetScaleX() {
	CheckState();
	if (skeleton) return skeleton->getScaleX();
	return 1;
}

void USpineWidget::SetScaleY(float scaleY) {
	CheckState();
	if (skeleton) skeleton->setScaleY(scaleY);
}

float USpineWidget::GetScaleY() {
	CheckState();
	if (skeleton) return skeleton->getScaleY();
	return 1;
}

void USpineWidget::GetBones(TArray<FString> &Bones) {
	CheckState();
	if (skeleton) {
		for (size_t i = 0, n = skeleton->getBones().size(); i < n; i++) {
			Bones.Add(skeleton->getBones()[i]->getData().getName().ToString());
		}
	}
}

bool USpineWidget::HasBone(const FString BoneName) {
	CheckState();
	if (skeleton) {
		return skeleton->getData()->findBone(TCHAR_TO_UTF8(*BoneName)) != nullptr;
	}
	return false;
}

void USpineWidget::GetSlots(TArray<FString> &Slots) {
	CheckState();
	if (skeleton) {
		for (size_t i = 0, n = skeleton->getSlots().Num(); i < n; i++) {
			Slots.Add(skeleton->getSlots()[i]->getData().getName());
		}
	}
}

bool USpineWidget::HasSlot(const FString SlotName) {
	CheckState();
	if (skeleton) {
		return skeleton->getData()->findSlot(TCHAR_TO_UTF8(*SlotName)) != nullptr;
	}
	return false;
}

void USpineWidget::GetAnimations(TArray<FString> &Animations) {
	CheckState();
	if (skeleton) {
		for (size_t i = 0, n = skeleton->getData()->getAnimations().Num(); i < n; i++) {
			Animations.Add(skeleton->getData()->getAnimations()[i]->getName().buffer());
		}
	}
}

bool USpineWidget::HasAnimation(FString AnimationName) {
	CheckState();
	if (skeleton) {
		return skeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName)) != nullptr;
	}
	return false;
}

float USpineWidget::GetAnimationDuration(FString AnimationName) {
	CheckState();
	if (skeleton) {
		TSharedPtr<spine::Animation> animation = skeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName));
		if (animation == nullptr) return 0;
		else return animation->getDuration();
	}
	return 0;
}

void USpineWidget::SetAutoPlay(bool bInAutoPlays)
{
	bAutoPlaying = bInAutoPlays;
}

void USpineWidget::SetPlaybackTime(float InPlaybackTime, bool bCallDelegates)
{
	CheckState();

	if (state && state->getCurrent(0)) {
		TSharedPtr <spine::Animation> CurrentAnimation = state->getCurrent(0)->getAnimation();
		const float CurrentTime = state->getCurrent(0)->getTrackTime();
		InPlaybackTime = FMath::Clamp(InPlaybackTime, 0.0f, CurrentAnimation->getDuration());
		const float DeltaTime = InPlaybackTime - CurrentTime;
		state->update(DeltaTime);
		state->apply(*skeleton);

		//Call delegates and perform the world transform
		if (bCallDelegates)
		{
			BeforeUpdateWorldTransform.Broadcast(this);
		}
		skeleton->updateWorldTransform();
		if (bCallDelegates)
		{
			AfterUpdateWorldTransform.Broadcast(this);
		}
	}
}

void USpineWidget::SetTimeScale(float timeScale) {
	CheckState();
	if (state) state->setTimeScale(timeScale);
}

float USpineWidget::GetTimeScale() {
	CheckState();
	if (state) return state->getTimeScale();
	return 1;
}

UTrackEntry* USpineWidget::SetAnimation(int trackIndex, FString animationName, bool loop) {
	CheckState();
	if (state && skeleton->getData()->findAnimation(TCHAR_TO_UTF8(*animationName))) {
		state->disableQueue();
		TSharedPtr<TrackEntry> entry = state->setAnimation(trackIndex, TCHAR_TO_UTF8(*animationName), loop);
		state->enableQueue();
		UTrackEntry* uEntry = NewObject<UTrackEntry>();
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
		return NewObject<UTrackEntry>();

}

UTrackEntry* USpineWidget::AddAnimation(int trackIndex, FString animationName, bool loop, float delay) {
	CheckState();
	if (state && skeleton->getData()->findAnimation(TCHAR_TO_UTF8(*animationName))) {
		state->disableQueue();
		TSharedPtr<TrackEntry> entry = state->addAnimation(trackIndex, TCHAR_TO_UTF8(*animationName), loop, delay);
		state->enableQueue();
		UTrackEntry* uEntry = NewObject<UTrackEntry>();
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
		return NewObject<UTrackEntry>();
}

UTrackEntry* USpineWidget::SetEmptyAnimation(int trackIndex, float mixDuration) {
	CheckState();
	if (state) {
		TSharedPtr<TrackEntry> entry = state->setEmptyAnimation(trackIndex, mixDuration);
		UTrackEntry* uEntry = NewObject<UTrackEntry>();
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
		return NewObject<UTrackEntry>();
}

UTrackEntry* USpineWidget::AddEmptyAnimation(int trackIndex, float mixDuration, float delay) {
	CheckState();
	if (state) {
		TSharedPtr<TrackEntry> entry = state->addEmptyAnimation(trackIndex, mixDuration, delay);
		UTrackEntry* uEntry = NewObject<UTrackEntry>();
		uEntry->SetTrackEntry(entry);
		trackEntries.Add(uEntry);
		return uEntry;
	}
	else
		return NewObject<UTrackEntry>();
}

UTrackEntry* USpineWidget::GetCurrent(int trackIndex) {
	CheckState();
	if (state && state->getCurrent(trackIndex)) {
		TSharedPtr<TrackEntry> entry = state->getCurrent(trackIndex);
		if (entry->GetRendererObject().Get()) {
			return (UTrackEntry*)entry->GetRendererObject().Get();
		}
		else {
			UTrackEntry* uEntry = NewObject<UTrackEntry>();
			uEntry->SetTrackEntry(entry);
			trackEntries.Add(uEntry);
			return uEntry;
		}
	}
	else
		return NewObject<UTrackEntry>();
}

void USpineWidget::ClearTracks() {
	CheckState();
	if (state) {
		state->clearTracks();
	}
}

void USpineWidget::ClearTrack(int trackIndex) {
	CheckState();
	if (state) {
		state->clearTrack(trackIndex);
	}
}
