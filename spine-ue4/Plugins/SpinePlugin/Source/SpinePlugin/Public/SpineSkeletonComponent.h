

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"

#include <spine/Atlas.h>
#include "SpineUnrealTypes.h"
#include "SpineSkeletonComponent.generated.h"

namespace spine
{
	class Skeleton;
	class SkeletonData;
	class AnimationState;
	class AnimationStateData;
	class Atlas;
	class AtlasPage;
}

class USpineSkeletonComponent;
class USpineAtlasAsset;
class USpineSkeletonDataAsset;
class USpineSkeletonAnimationComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpineSkinUpdatedDelegateSignature, FString,SkinName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpineSkeletonUpdatedDelegateSignature);

UCLASS(ClassGroup=(Spine), meta=(BlueprintSpawnableComponent))
class SPINEPLUGIN_API USpineSkeletonComponent: public UProceduralMeshComponent {
	GENERATED_BODY()

public:
	USpineSkeletonComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Transient,DuplicateTransient)
	USpineSkeletonDataAsset* SkeletonData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spine)
	bool bForceRefresh;
	
	TSharedPtr<spine::Skeleton> GetSkeleton ()const { return CurrentSpineSkeleton; };

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
	virtual bool SetSkin ( FString SkinName);

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	bool HasSkin( FString SkinName)const;
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	bool SetAttachment (const FString slotName, const FString attachmentName);
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	bool GetBoneWorldTransform (const FString& BoneName, FTransform& OutTransform)const;
	
	/*UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	void SetBoneWorldPosition (const FString& BoneName, const FVector& position);*/

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	void UpdateWorldTransform();
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	void SetToSetupPose ();
	
	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	void SetBonesToSetupPose ();

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
	bool HasBone( FName BoneName)const;

	UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
	void GetSlots(TArray<FString> &Slots)const;

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	bool HasSlot( FString SlotName)const;

	UFUNCTION(BlueprintPure, Category = "Components|Spine|Skeleton")
	void GetAnimations(TArray<FString> &Animations)const;

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	bool HasAnimation(FString AnimationName)const;

	UFUNCTION(BlueprintCallable, Category = "Components|Spine|Skeleton")
	float GetAnimationDuration(FString AnimationName)const;
	

	
//	virtual void BeginPlay () override;
	virtual void TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual bool HasAnySockets() const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	virtual bool DoesSocketExist(FName InSocketName) const override;

	//virtual void Activate(bool bReset = false) override;
	//virtual void Deactivate() override;

	UFUNCTION(BlueprintCallable)
	virtual bool ApplyReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup);

	UFUNCTION(BlueprintCallable)
	virtual bool CancelReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup);

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


