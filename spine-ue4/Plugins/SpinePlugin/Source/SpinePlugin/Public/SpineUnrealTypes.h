// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "SpineUnrealTypes.generated.h"

/**
 * 
 */

namespace spine
{
	class Skeleton;
	class SkeletonData;
	class AnimationState;
	class AnimationStateData;
	class Atlas;
	class AtlasPage;
	class AtlasRegion;
	class Attachment;
	class Skin;
}

class USpineSkeletonComponent;
class USpineAtlasAsset;
class USpineSkeletonDataAsset;

UCLASS()
class SPINEPLUGIN_API USpineUnrealTypes : public UObject
{
	GENERATED_BODY()
	
};




USTRUCT(BlueprintType)
struct FReplaceSkinAttachmentDesc:public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USpineSkeletonDataAsset> SkeletonAsset;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomPageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomRegionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttachmentName;

	TSharedPtr<spine::Attachment> GenerateCopyAttachmentWithNewAtlasRegionIn(spine::Skeleton& InSkeleton, spine::Skin& InSkin, spine::Atlas& InAtlas, int32& OutSlotIndex) const;

	TSharedPtr<spine::Attachment> FindOriginAttachment(spine::Skeleton& InSkeleton, spine::Skin& InSkin, spine::Atlas& InAtlas, int32& OutSlotIndex) const;
};


USTRUCT(BlueprintType)
struct FReplaceAttachmentGroup
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FReplaceSkinAttachmentDesc> Replacements;
};