

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "SpineSkeletonDataAsset.generated.h"


namespace spine
{
	class Skeleton;
	class SkeletonData;
	class AnimationState;
	class AnimationStateData;
	class Atlas;
}

class USpineSkeletonComponent;
class USpineAtlasAsset;
class USpineSkeletonDataAsset;


struct SPINEPLUGIN_API FSpineSkeletonDataAssetObjectVersion
{
	enum Type
	{
		// Before any version changes were made
		BeforeCustomVersionWasAdded = 0,
		
		AddAnimEventRemapToGameplayTag,
		Add_bIsJsonRawData_Flag,
		Add_bIsValid_RawData_Flag,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	}; //
	
	

	// The GUID for this custom version number
	const static FGuid GUID;

	FSpineSkeletonDataAssetObjectVersion() = delete;
};


USTRUCT(BlueprintType, Category = "Spine")
struct SPINEPLUGIN_API FSpineAnimationStateMixData
{
	GENERATED_BODY();

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString From;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString To;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mix = 0;
};

USTRUCT(BlueprintType, Category = "Spine")
struct FSpineAnimEventPair
{
	GENERATED_BODY();
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag RemapToGameplayEventTag;
};


UCLASS(BlueprintType, ClassGroup = (Spine))
class SPINEPLUGIN_API USpineSkeletonDataAsset : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<spine::SkeletonData> GetSpineSkeletonData()const;
	TSharedPtr<spine::AnimationStateData> CreateAnimationStateData() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DefaultMix = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSpineAnimationStateMixData> MixData;

	UPROPERTY( VisibleAnywhere)
	TArray<FString> Bones;

	UPROPERTY( VisibleAnywhere)
	TArray<FString> Slots;

	UPROPERTY(VisibleAnywhere)
	TArray<FString> Attachments;

	UPROPERTY(VisibleAnywhere)
	TArray<FString> Skins;

	UPROPERTY( VisibleAnywhere)
	TArray<FString> Animations;

	UPROPERTY( VisibleAnywhere)
	TArray<FString> Events;

	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FSpineAnimEventPair> AnimEventSetupArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USpineAtlasAsset* RelatedAtlasAsset;
	
	virtual void PostLoad() override;

	void RebuildCachedSkeletonData();
protected:
	UPROPERTY()
	TArray<uint8> rawData;	

	/* 指示加载方式是当做json还二进制文件 */
	UPROPERTY()
	bool bIsValidJsonRawData = false;

	UPROPERTY()
	bool bIsValidBinaryRawData = false;

	
	TSharedPtr<spine::SkeletonData> CachedSkeletonData;

	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#if WITH_EDITORONLY_DATA

public:
	UPROPERTY(EditAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif


#if WITH_EDITOR
public:
	void SetRawData(TArray<uint8> &Data);
protected:

	void LoadInfo();	
	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
