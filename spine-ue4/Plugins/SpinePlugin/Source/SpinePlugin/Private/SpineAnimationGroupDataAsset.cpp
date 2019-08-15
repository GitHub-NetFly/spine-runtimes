// Fill out your copyright notice in the Description page of Project Settings.

#include "SpineAnimationGroupDataAsset.h"

#include "SpineSkeletonDataAsset.h"

#if WITH_EDITOR
void USpineAnimGroupAsset::RebuildAnimationSpecs()
{
	AnimationSpecs.Empty();

	for (USpineSkeletonDataAsset* Skeleton : SpineSkeletonDataSet)
	{
		if (IsValid(Skeleton))
		{
			for (const FString& AnimName : Skeleton->Animations)
			{
				AnimationSpecs.Add({ Skeleton,AnimName });
			}
		}
	}
}

void USpineAnimGroupAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName SpineSkeletonDataSetPropertyName = GET_MEMBER_NAME_CHECKED(ThisClass, SpineSkeletonDataSet);

	if (PropertyChangedEvent.Property != nullptr && PropertyChangedEvent.Property->GetFName() == SpineSkeletonDataSetPropertyName)
	{
		RebuildAnimationSpecs();

		MarkPackageDirty();
	}
}
#endif // WITH_EDITOR

void USpineAnimGroupAsset::PostLoad()
{
	Super::PostLoad();

//#if WITH_EDITOR
//
//	void RebuildAnimationSpecs();
//
//#endif // WITH_EDITOR

}

bool FSpineAnimationSpec::IsValid() const
{
	return ::IsValid(RelatedSpineSkeletonDataAsset) && !AnimationName.IsEmpty();
}

FString FSpineAnimationSpec::GetDebugString() const
{
	return FString::Printf(TEXT("Asset:%s AnimName:%s"), *GetNameSafe(RelatedSpineSkeletonDataAsset), *AnimationName);
}
