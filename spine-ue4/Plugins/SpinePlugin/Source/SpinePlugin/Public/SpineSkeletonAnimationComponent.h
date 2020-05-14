

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "spine/Atlas.h"
#include "SpineAnimationGroupDataAsset.h"
#include "SpineAnimNotify.h"
#include "GameplayTagAssetInterface.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "SpineUnrealTypes.h"
#include <spine/AnimationState.h>

#include "SpineSkeletonAnimationComponent.generated.h"


class USpineAtlasAsset;
class USpineSkeletonDataAsset;
class USpineSkeletonAnimationComponent;
class UTrackEntry;



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpineAnimationStateMachineDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpineSkinUpdatedDelegateSignature, FString, SkinName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpineSkeletonUpdatedDelegateSignature);

class USpineAtlasAsset;

UCLASS(ClassGroup=(Spine), hidecategories = (ProceduralMesh,Physics,Collision), meta=(BlueprintSpawnableComponent))
class SPINEPLUGIN_API USpineSkeletonAnimationComponent: public UProceduralMeshComponent {
	GENERATED_BODY()

public:
	TSharedPtr<spine::AnimationState> GetAnimationState () { return SpineAnimState; };
		
	USpineSkeletonAnimationComponent (const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay () override;
		
	virtual void TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Added functions for manual configuration

	void OnSpineAnimStateEventReceived(spine::AnimationState* State,spine::EventType Type, TSharedRef<spine::TrackEntry> Entry, const spine::Event& Event);

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
	void SetEmptyAnimation (int trackIndex, float mixDuration);
	
	UFUNCTION(BlueprintCallable, Category="Components|Spine|Animation")
	void AddEmptyAnimation (int trackIndex, float mixDuration, float delay);
	
	UFUNCTION(BlueprintPure, Category="Components|Spine|Animation")
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
	FSpineAnimationStateMachineDelegate  AnimationStateMachineOff;

	void Internal_OnAnimationEvent(UTrackEntry* Entry, FSpineEvent Evt);

	UFUNCTION(BlueprintCallable)
	void AddDynamicAnimEventListener(const FSpineAnimationSpec& AnimDesc, FString AnimEvent, FGameplayTag RemapToTag);

	UPROPERTY(VisibleAnywhere,Transient, Category = Animation)
	TMap<FSpineAnimEventDesc, FGameplayTag>  DynamicAnimEventListenerMap;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationCompleteDelegate AnimationComplete;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationEndDelegate AnimationEnd;

	UPROPERTY(BlueprintAssignable, Category="Components|Spine|Animation")
	FSpineAnimationDisposeDelegate AnimationDispose;

	// 数字越小,越优先被渲染.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spine)
	int32 Prior;


#if WITH_EDITOR

	UPROPERTY(EditAnywhere, Category = Spine)
	FSpineAnimationSpec PreviewAnimSpec;

	UPROPERTY(EditAnywhere, Category = Spine)
	FString PreviewSkinName = TEXT("default");
#endif
	
	// used in C event callback. Needs to be public as we can't call
	// protected methods from plain old C function.
	void GCTrackEntry(UTrackEntry* entry);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	virtual bool ApplyReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup) ;

	virtual bool CancelReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup) ;

	virtual bool SetSkin(FString SkinName) ;

	void StopAnimationStateMachine();

	void AllowAnimationStateMachine();


	virtual void OnRegister() override;


protected:
	TSharedPtr<spine::AnimationState> SpineAnimState;

	// keep track of track entries so they won't get GCed while
	// in transit within a blueprint
	UPROPERTY(Transient,DuplicateTransient)
	TArray<UTrackEntry*> trackEntries;

private:
	UPROPERTY(Transient)
	bool bIsPaused = false;

	UPROPERTY(Transient)
	bool bCanPlayNormalAnimStateMachine = true;

	UPROPERTY(Transient)
	bool bIsAnimStateMachineWorking = false;

	void ConditionStartAnimationStateMachine();
public:

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Animation")
	bool IsAnimStateMachineWorking()const { return bIsAnimStateMachineWorking; }

	UFUNCTION(BlueprintPure, Category = "Components|Spine|Animation")
	bool IsSpineAnimSpecPlaying(const FSpineAnimationSpec& InSpec);

	UFUNCTION(BlueprintPure, Category = "Components|Spine|Animation")
	bool IsAnySpineAnimPlaying();

public:
	FUpdateMaterialHandle SetMaterialParam(TArray<FUpdateMaterialParam> Params,FName GroupName,float Duration,
		float BlendInTime, float BlendOutTime,
		FOnUpdateMaterialFinishedDelegate OnFinished, FK2OnUpdateMaterialFinishedDelegate K2OnFinished= FK2OnUpdateMaterialFinishedDelegate());

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Material", meta = (DisplayName = "SetMaterialParam", ScriptName = "SetMaterialParam"))
		FUpdateMaterialHandle K2_SetMaterialParam(TArray<FUpdateMaterialParam> Params, FName GroupName, float Duration = 0.1f, float BlendInTime = 0.05f, float BlendOutTime = 0.05f)
	{
		return SetMaterialParam(Params, GroupName, Duration, BlendInTime, BlendOutTime, FOnUpdateMaterialFinishedDelegate());
	}


	void RemoveMaterialParam(FUpdateMaterialHandle InHandle, bool bIsCancel,bool bImmediately, bool bDisableCallback);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Material")
	void ExternaStopMaterialParamByGroupName(FName GroupName,bool bImmediately=true);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Material")
	void ExternaStopAllMaterialParams(bool bImmediately = true);


	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Material")
	void SetAvoidMaterialCrossYOffset(float InOffset);

private:

	UPROPERTY(Transient)
	bool bIsTicking = false;

	void InternalTick_UpdateMaterialParam(float DeltaTime);

	TArray<UMaterialInstanceDynamic*>& GetBlendMaterial(ESpineMaterialBlendType InBlendType);

	void ApplyCurrentInForceParamToMaterial(const TMap<FParamID, FParamCurrentData>& CurrentInForceParamState);

	TArray<FUpdateMaterialRuntime> UpdateMaterialRuntimes;



	struct FPendingTriggerFinishCallback
	{
		bool bIsCancel;

		FOnUpdateMaterialFinishedDelegate OnFinished;

		FK2OnUpdateMaterialFinishedDelegate K2OnFinished;
	};

	TArray<FPendingTriggerFinishCallback> InTickPendingTriggerFinishCallbackArray;

	TMap<FParamID, FParamCurrentData>  InTickPendingZeroedFinishState;

public:
		UPROPERTY(Transient, DuplicateTransient)
			USpineSkeletonDataAsset* SkeletonData;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spine)
			bool bForceRefresh;

		TSharedPtr<spine::Skeleton> GetSkeleton()const { return CurrentSpineSkeleton; };

		UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
			void GetSkins(TArray<FString> &Skins)const;

		UPROPERTY(BlueprintAssignable)
			FOnSpineSkinUpdatedDelegateSignature OnSpineSkinUpdated;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetSkeletonAsset(USpineSkeletonDataAsset* InSkeletonAsset);

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			USpineSkeletonDataAsset* GetSkeletonAsset()const { return SkeletonData; }

		UPROPERTY(BlueprintAssignable)
			FOnSpineSkeletonUpdatedDelegateSignature OnSpineSkeletonUpdated;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool HasSkin(FString SkinName)const;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool SetAttachment(const FString slotName, const FString attachmentName);

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool GetBoneWorldTransform(const FString& BoneName, FTransform& OutTransform)const;

		/*UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
		void SetBoneWorldPosition (const FString& BoneName, const FVector& position);*/

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void UpdateWorldTransform();

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetToSetupPose();

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetBonesToSetupPose();

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetSlotsToSetupPose();

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetScaleX(float scaleX);

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			float GetScaleX();

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			void SetScaleY(float scaleY);

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			float GetScaleY();

		UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
			void GetBones(TArray<FName> &BoneNames)const;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool HasBone(FName BoneName)const;

		UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
			void GetSlots(TArray<FString> &Slots)const;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool HasSlot(FString SlotName)const;

		UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
			void GetAnimations(TArray<FString> &Animations)const;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			bool HasAnimation(FString AnimationName)const;

		UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
			float GetAnimationDuration(FString AnimationName)const;



		virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
		virtual bool HasAnySockets() const override;
		virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
		virtual bool DoesSocketExist(FName InSocketName) const override;

		//virtual void Activate(bool bReset = false) override;
		//virtual void Deactivate() override;

		/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString CurrentSkinName;*/

		/*UFUNCTION(BlueprintCallable)
		FString GetCurrentSkinName()const { return CurrentSkinName; }*/


		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

		//virtual bool Sucker(const FCustomAtlasRegion& InRegion, FString TemplateSkin=TEXT("Skin_A"), FString SlotName= TEXT("Socket"), FString AttachmentName= TEXT("SkinPlaceholder"));

protected:
	virtual void CheckState();
	virtual void InternalTick_SkeletonPose(float DeltaTime);
	virtual void DisposeState();

	TSharedPtr<spine::Skeleton> CurrentSpineSkeleton;

	TSharedPtr<spine::SkeletonData> lastSkeletonData;

	UPROPERTY(Transient, DuplicateTransient)
		USpineSkeletonDataAsset* LastSkeletonAsset = nullptr;

public:

	/* Updates this skeleton renderer using the provided skeleton animation component. */
	void InternalTick_Renderer();

	// Material Instance parents
	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* NormalBlendMaterial;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* AdditiveBlendMaterial;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* MultiplyBlendMaterial;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* ScreenBlendMaterial;

	// Need to hold on to the dynamic instances, or the GC will kill us while updating them
	UPROPERTY(Category = "Spine Render", VisibleAnywhere, BlueprintReadOnly)
		TArray<UMaterialInstanceDynamic*> atlasNormalBlendMaterials;
	TMap< spine::AtlasPage, UMaterialInstanceDynamic*> pageToNormalBlendMaterial;

	UPROPERTY(Category = "Spine Render", VisibleAnywhere, BlueprintReadOnly)
		TArray<UMaterialInstanceDynamic*> atlasAdditiveBlendMaterials;
	TMap<spine::AtlasPage, UMaterialInstanceDynamic*> pageToAdditiveBlendMaterial;

	UPROPERTY(Category = "Spine Render", VisibleAnywhere, BlueprintReadOnly)
		TArray<UMaterialInstanceDynamic*> atlasMultiplyBlendMaterials;
	TMap<spine::AtlasPage, UMaterialInstanceDynamic*> pageToMultiplyBlendMaterial;

	UPROPERTY(Category = "Spine Render", VisibleAnywhere, BlueprintReadOnly)
		TArray<UMaterialInstanceDynamic*> atlasScreenBlendMaterials;
	TMap<spine::AtlasPage, UMaterialInstanceDynamic*> pageToScreenBlendMaterial;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		float DepthOffset;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		FName TextureParameterName;

	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		FLinearColor Color;

	/** Whether to generate collision geometry for the skeleton, or not. */
	UPROPERTY(Category = "Spine Render", EditAnywhere, BlueprintReadWrite)
		bool bCreateCollision;
protected:
	void UpdateMesh(TSharedRef<class spine::Skeleton> SkeletonRef);

	void Flush(int &Idx, TArray<FVector> &Vertices, TArray<int32> &Indices, TArray<FVector> &Normals, TArray<FVector2D> &Uvs, TArray<FColor> &Colors, TArray<FVector> &Colors2, UMaterialInstanceDynamic* Material);

};
