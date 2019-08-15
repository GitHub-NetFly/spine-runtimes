// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagAssetInterface.h"
#include "SpineAnimationGroupDataAsset.h"
#include "SpineAnimNotify.generated.h"

/**
 * 
 */
class USpineSkeletonDataAsset;

class USpineAnimGroupAsset;
class USpineSkeletonAnimationComponent;


USTRUCT(BlueprintType)
struct SPINEPLUGIN_API FSpineAnimEventDesc
{
	GENERATED_BODY()

public:
	FSpineAnimEventDesc() = default;

	FSpineAnimEventDesc(const FSpineAnimationSpec& InAnimationSpec, const FString& InEventName) :
		SpineAnimationSpec(InAnimationSpec), EventName(InEventName)
	{

	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSpineAnimationSpec SpineAnimationSpec;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString EventName;

	inline friend uint32 GetTypeHash(const FSpineAnimEventDesc& Item)
	{

		auto hash1 = GetTypeHash(Item.EventName);

		auto hash2 = GetTypeHash(Item.SpineAnimationSpec);

		return HashCombine(hash1, hash2);
	}

	FORCEINLINE bool operator==(const FSpineAnimEventDesc& Rhs) const
	{
		return SpineAnimationSpec == Rhs.SpineAnimationSpec&&EventName == Rhs.EventName;
	}
};


USTRUCT(BlueprintType)
struct SPINEPLUGIN_API FSpineAnimEventPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float EventMagnitude;
	
};

UCLASS(NotBlueprintable)
class SPINEPLUGIN_API USpineAnimNotifyPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USpineSkeletonDataAsset* SkeletonData;

	UPROPERTY()
	FString AnimName;

	UPROPERTY()
	class UTrackEntry* TrackEntry;

	static USpineAnimNotifyPayload* CreatePayload(USpineSkeletonDataAsset* SkeletonData, const FString& AnimName, UTrackEntry* TrackEntry);
};
