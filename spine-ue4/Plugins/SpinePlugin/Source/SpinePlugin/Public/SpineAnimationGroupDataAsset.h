// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpineAnimationGroupDataAsset.generated.h"

/**
 * 
 */
class USpineSkeletonDataAsset;



USTRUCT(BlueprintType)
struct SPINEPLUGIN_API FSpineAnimationSpec
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		USpineSkeletonDataAsset* RelatedSpineSkeletonDataAsset;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FString AnimationName;

	inline friend uint32 GetTypeHash(const FSpineAnimationSpec& Item)
	{

		auto hash2 = GetTypeHash(Item.RelatedSpineSkeletonDataAsset);

		auto hash3 = GetTypeHash(Item.AnimationName);

		return HashCombine(hash3, hash2);
	}

	FORCEINLINE bool operator==(const FSpineAnimationSpec& Rhs) const
	{
		return AnimationName == Rhs.AnimationName&&RelatedSpineSkeletonDataAsset == Rhs.RelatedSpineSkeletonDataAsset;
	}

	FORCEINLINE bool operator!=(const FSpineAnimationSpec& Rhs) const
	{
		return !(*this == Rhs);
	}

	 bool IsValid()const;
	

	FString GetDebugString()const;
};

UCLASS()
class SPINEPLUGIN_API USpineAnimGroupAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSet<USpineSkeletonDataAsset*> SpineSkeletonDataSet;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FSpineAnimationSpec> AnimationSpecs;

#if WITH_EDITOR
	void RebuildAnimationSpecs();

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void PostLoad() override;
};
