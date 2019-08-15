﻿/******************************************************************************
* Spine Runtimes Software License v2.5
*
* Copyright (c) 2013-2016, Esoteric Software
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the Spine
* Runtimes software and derivative works solely for personal or internal
* use. Without the written permission of Esoteric Software (see Section 2 of
* the Spine Software License Agreement), you may not (a) modify, translate,
* adapt, or develop new applications using the Spine Runtimes or otherwise
* create derivative works or improvements of the Spine Runtimes or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
* USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
* IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"

//#include "SpineWidget.generated.h"

class SSpineWidget;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineWidgetBeforeUpdateWorldTransformDelegate, USpineWidget*, skeleton);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpineWidgetAfterUpdateWorldTransformDelegate, USpineWidget*, skeleton);



//UCLASS(ClassGroup = (Spine), meta = (BlueprintSpawnableComponent))
//class SPINEPLUGIN_API USpineWidget: public UWidget {
//	GENERATED_BODY()
//
//public:
//	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
//	virtual void SynchronizeProperties() override;
//#if WITH_EDITOR
//	virtual const FText GetPaletteCategory() override;
//#endif
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadWrite)
//	float Scale = 1;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spine)
//	USpineAtlasAsset* Atlas;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spine)
//	USpineSkeletonDataAsset* SkeletonData;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadOnly)
//	UMaterialInterface* NormalBlendMaterial;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadOnly)
//	UMaterialInterface* AdditiveBlendMaterial;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadOnly)
//	UMaterialInterface* MultiplyBlendMaterial;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadOnly)
//	UMaterialInterface* ScreenBlendMaterial;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadWrite)
//	FName TextureParameterName;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadWrite)
//	float DepthOffset = 0.1f;
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadWrite)
//	FLinearColor Color = FLinearColor(1, 1, 1, 1);
//
//	UPROPERTY(Category = Spine, EditAnywhere, BlueprintReadOnly)
//	FSlateBrush Brush;
//
//	UFUNCTION(BlueprintPure, Category = "Widget|Spine|Skeleton")
//	void GetSkins(TArray<FString> &Skins);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool SetSkin(const FString SkinName);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool HasSkin(const FString SkinName);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool SetAttachment(const FString slotName, const FString attachmentName);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void UpdateWorldTransform();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void SetToSetupPose();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void SetBonesToSetupPose();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void SetSlotsToSetupPose();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void SetScaleX(float scaleX);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	float GetScaleX();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	void SetScaleY(float scaleY);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	float GetScaleY();
//
//	UFUNCTION(BlueprintPure, Category = "Widget|Spine|Skeleton")
//	void GetBones(TArray<FString> &Bones);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool HasBone(const FString BoneName);
//
//	UFUNCTION(BlueprintPure, Category = "Widget|Spine|Skeleton")
//	void GetSlots(TArray<FString> &Slots);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool HasSlot(const FString SlotName);
//
//	UFUNCTION(BlueprintPure, Category = "Widget|Spine|Skeleton")
//	void GetAnimations(TArray<FString> &Animations);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	bool HasAnimation(FString AnimationName);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Skeleton")
//	float GetAnimationDuration(FString AnimationName);
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Skeleton")
//	FSpineWidgetBeforeUpdateWorldTransformDelegate BeforeUpdateWorldTransform;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Skeleton")
//	FSpineWidgetAfterUpdateWorldTransformDelegate AfterUpdateWorldTransform;
//
//	/* Manages if this skeleton should update automatically or is paused. */
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//		void SetAutoPlay(bool bInAutoPlays);
//
//	/* Directly set the time of the current animation, will clamp to animation range. */
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	void SetPlaybackTime(float InPlaybackTime, bool bCallDelegates = true);
//
//	// Blueprint functions
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	void SetTimeScale(float timeScale);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	float GetTimeScale();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	UTrackEntry* SetAnimation(int trackIndex, FString animationName, bool loop);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	UTrackEntry* AddAnimation(int trackIndex, FString animationName, bool loop, float delay);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	UTrackEntry* SetEmptyAnimation(int trackIndex, float mixDuration);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	UTrackEntry* AddEmptyAnimation(int trackIndex, float mixDuration, float delay);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	UTrackEntry* GetCurrent(int trackIndex);
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	void ClearTracks();
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	void ClearTrack(int trackIndex);
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationStartDelegate AnimationStart;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationInterruptDelegate AnimationInterrupt;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationEventDelegate AnimationEvent;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationCompleteDelegate AnimationComplete;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationEndDelegate AnimationEnd;
//
//	UPROPERTY(BlueprintAssignable, Category = "Widget|Spine|Animation")
//	FSpineAnimationDisposeDelegate AnimationDispose;
//
//	UFUNCTION(BlueprintCallable, Category = "Widget|Spine|Animation")
//	void SpineWidgetTick(float DeltaTime, bool CallDelegates = true);
//
//	virtual void FinishDestroy() override;
//
//	// used in C event callback. Needs to be public as we can't call
//	// protected methods from plain old C function.
//	void GCTrackEntry(UTrackEntry* entry) { trackEntries.Remove(entry); }
//protected:
//	friend class SSpineWidget;
//
//	virtual TSharedRef<SWidget> RebuildWidget() override;
//	virtual void CheckState();
//	virtual void DisposeState();
//
//	TSharedPtr<SSpineWidget> slateWidget;	
//
//	spine::Skeleton* skeleton;
//	spine::AnimationState* state;
//	USpineAtlasAsset* lastAtlas = nullptr;
//	spine::Atlas* lastSpineAtlas = nullptr;
//	USpineSkeletonDataAsset* lastData = nullptr;
//
//	// Need to hold on to the dynamic instances, or the GC will kill us while updating them
//	UPROPERTY()
//	TArray<UMaterialInstanceDynamic*> atlasNormalBlendMaterials;
//	TMap<spine::AtlasPage*, UMaterialInstanceDynamic*> pageToNormalBlendMaterial;
//
//	UPROPERTY()
//	TArray<UMaterialInstanceDynamic*> atlasAdditiveBlendMaterials;
//	TMap<spine::AtlasPage*, UMaterialInstanceDynamic*> pageToAdditiveBlendMaterial;
//
//	UPROPERTY()
//	TArray<UMaterialInstanceDynamic*> atlasMultiplyBlendMaterials;
//	TMap<spine::AtlasPage*, UMaterialInstanceDynamic*> pageToMultiplyBlendMaterial;
//
//	UPROPERTY()
//	TArray<UMaterialInstanceDynamic*> atlasScreenBlendMaterials;
//	TMap<spine::AtlasPage*, UMaterialInstanceDynamic*> pageToScreenBlendMaterial;
//
//	spine::Vector<float> worldVertices;
//	spine::SkeletonClipping clipper;
//
//	// keep track of track entries so they won't get GCed while
//	// in transit within a blueprint
//	UPROPERTY()
//	TSet<UTrackEntry*> trackEntries;
//
//private:
//	/* If the animation should update automatically. */
//	UPROPERTY()
//	bool bAutoPlaying;
//};