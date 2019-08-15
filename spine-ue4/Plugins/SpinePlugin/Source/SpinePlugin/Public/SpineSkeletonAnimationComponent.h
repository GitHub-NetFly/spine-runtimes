

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpineSkeletonComponent.h"

#include "SpineAnimationGroupDataAsset.h"
#include "SpineAnimNotify.h"
#include "GameplayTagAssetInterface.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

#include "SpineSkeletonAnimationComponent.generated.h"

namespace spine
{
	class Skeleton;
	class SkeletonData;
	class AnimationState;
	class AnimationStateData;
	class Atlas;
	class Event;
	class TrackEntry;
	enum EventType;
}

class USpineSkeletonComponent;
class USpineAtlasAsset;
class USpineSkeletonDataAsset;
class USpineSkeletonAnimationComponent;

USTRUCT(BlueprintType, Category="Spine")
struct SPINEPLUGIN_API FSpineEvent 
{
	GENERATED_BODY();

public:
	void SetEvent(const spine::Event& InEvent);
	

	UPROPERTY(BlueprintReadonly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	FString StringValue;

	UPROPERTY(BlueprintReadOnly)
	int IntValue;

	UPROPERTY(BlueprintReadOnly)
	float FloatValue;

	UPROPERTY(BlueprintReadOnly)
	float Time;
};

class UTrackEntry;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineAnimationStartDelegate, UTrackEntry*, entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpineAnimationEventDelegate, UTrackEntry*, entry, FSpineEvent, evt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineAnimationInterruptDelegate, UTrackEntry*, entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineAnimationCompleteDelegate, UTrackEntry*, entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineAnimationEndDelegate, UTrackEntry*, entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineAnimationDisposeDelegate, UTrackEntry*, entry);


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpineAnimationStateMachineDelegate);


UCLASS(ClassGroup=(Spine), meta=(BlueprintSpawnableComponent), BlueprintType)
class SPINEPLUGIN_API UTrackEntry: public UObject {
	GENERATED_BODY ()

public:

	void SetTrackEntry (TSharedPtr<spine::TrackEntry> trackEntry);
	TSharedPtr<spine::TrackEntry> GetTrackEntry() { return entry; }
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
	int GetTrackIndex();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		bool GetLoop();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetLoop(bool loop);
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetEventThreshold();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetEventThreshold(float eventThreshold);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAttachmentThreshold();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetAttachmentThreshold(float attachmentThreshold);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetDrawOrderThreshold();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetDrawOrderThreshold(float drawOrderThreshold);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAnimationStart();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetAnimationStart(float animationStart);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAnimationEnd();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetAnimationEnd(float animationEnd);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAnimationLast();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetAnimationLast(float animationLast);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetDelay();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetDelay(float delay);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetTrackTime();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetTrackTime(float trackTime);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetTrackEnd();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetTrackEnd(float trackEnd);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetTimeScale();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetTimeScale(float timeScale);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAlpha();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetAlpha(float alpha);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetMixTime();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetMixTime(float mixTime);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetMixDuration();
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		void SetMixDuration(float mixDuration);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		FString GetAnimationName();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
		float GetAnimationDuration();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|TrackEntry")
	bool IsValidAnimation() { return entry.IsValid(); }

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationStartDelegate AnimationStart;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationInterruptDelegate AnimationInterrupt;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationEventDelegate AnimationEvent;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationCompleteDelegate AnimationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationEndDelegate AnimationEnd;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|TrackEntry")
	FSpineAnimationDisposeDelegate AnimationDispose;

protected:
	TSharedPtr<spine::TrackEntry> entry;
};

class USpineAtlasAsset;
UCLASS(ClassGroup=(Spine), meta=(BlueprintSpawnableComponent))
class SPINEPLUGIN_API USpineSkeletonAnimationComponent: public USpineSkeletonComponent {
	GENERATED_BODY()

public:
	TSharedPtr<spine::AnimationState> GetAnimationState () { return SpineAnimState; };
		
	USpineSkeletonAnimationComponent ();
	
	virtual void BeginPlay () override;
		
	virtual void TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void FinishDestroy () override;

	//Added functions for manual configuration

	void OnSpineAnimStateEventReceived(spine::AnimationState* State, enum spine::EventType Type, TSharedRef<spine::TrackEntry> Entry, const spine::Event& Event);

	/* Manages if this skeleton should update automatically or is paused. */
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	void SetPaused(bool bPaused);

	/* Directly set the time of the current animation, will clamp to animation range. */
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Animation")
	void SetPlaybackTime(float InPlaybackTime);
	
	// Blueprint functions
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	void SetTimeScale(float timeScale);

	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	float GetTimeScale();

	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	UTrackEntry* SetAnimation (int trackIndex, FString animationName, bool loop);

	//
	UTrackEntry* PlayAbilityAnimation(const FSpineAnimationSpec& AnimationSpec, bool loop);

	void StopAbilityAnimation();

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Animation")
	UTrackEntry* SetAnimationWitSpec(const FSpineAnimationSpec& AnimationSpec, bool loop);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	UTrackEntry* AddAnimation (int trackIndex, FString animationName, bool loop, float delay);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	UTrackEntry* SetEmptyAnimation (int trackIndex, float mixDuration);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	UTrackEntry* AddEmptyAnimation (int trackIndex, float mixDuration, float delay);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	UTrackEntry* GetCurrent (int trackIndex);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	void ClearTracks ();
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	void ClearTrack (int trackIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Animation")
	void UnbindSpineAnimCallbackFrom(UObject* Object);

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationStartDelegate AnimationStart;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationInterruptDelegate AnimationInterrupt;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationEventDelegate AnimationEvent;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|Animation")
	FSpineAnimationStateMachineDelegate  AnimationStateMachineOn;

	UPROPERTY(BlueprintAssignable, Category = "Components|Spine|Animation")
	FSpineAnimationStateMachineDelegate AnimationStateMachineOff;

	void Internal_OnAnimationEvent(UTrackEntry* Entry, FSpineEvent Evt);

	UFUNCTION(BlueprintCallable)
	void AddDynamicAnimEventListener(const FSpineAnimationSpec& AnimDesc, FString AnimEvent, FGameplayTag RemapToTag);

	UPROPERTY(VisibleAnywhere,Transient)
	TMap<FSpineAnimEventDesc, FGameplayTag>  DynamicAnimEventListenerMap;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationCompleteDelegate AnimationComplete;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationEndDelegate AnimationEnd;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationDisposeDelegate AnimationDispose;

	UPROPERTY(EditAnywhere, Category=Spine)
	FString PreviewAnimation;

	UPROPERTY(EditAnywhere, Category=Spine)
	FString PreviewSkin;
	
	// used in C event callback. Needs to be public as we can't call
	// protected methods from plain old C function.
	void GCTrackEntry(UTrackEntry* entry) { trackEntries.Remove(entry); }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	virtual void CheckState () override;
	virtual void InternalTick_SkeletonPose(float DeltaTime) override;
	virtual void DisposeState () override;

	virtual bool ApplyReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup) override;

	virtual bool CancelReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup) override;

	virtual bool SetSkin(FString SkinName) override;

protected:
	TSharedPtr<spine::AnimationState> SpineAnimState;

	// keep track of track entries so they won't get GCed while
	// in transit within a blueprint
	UPROPERTY()
	TSet<UTrackEntry*> trackEntries;

private:
	UPROPERTY(Transient)
	bool bIsPaused = false;

	FString lastPreviewAnimation;
	FString lastPreviewSkin;
};
