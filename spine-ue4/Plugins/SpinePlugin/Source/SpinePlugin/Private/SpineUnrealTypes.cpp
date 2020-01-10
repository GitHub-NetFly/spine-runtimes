// Fill out your copyright notice in the Description page of Project Settings.


#include "SpineUnrealTypes.h"

#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"
#include "spine/spine.h"

#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveVector.h"

#include "UObject/LinkerLoad.h"

TSharedPtr<spine::Attachment> FReplaceSkinAttachmentDesc::GenerateCopyAttachmentWithNewAtlasRegionIn(
	spine::Skeleton& InSkeleton, 
	spine::Skin& InSkin, 
	spine::Atlas& InAtlas, 
	int32& OutSlotIndex) const
{
	using namespace spine;


	if (!SkeletonAsset.IsValid())
	{
		return nullptr;
	}

	check(SkeletonAsset->GetSpineSkeletonData().IsValid());

	if (SkeletonAsset->GetSpineSkeletonData()!= InSkeleton.getData())
	{
		return nullptr;
	}

	int32 SlotIndex = InSkeleton.findSlotIndex(SlotName);
	if (SlotIndex == INDEX_NONE)
	{
		return nullptr;
	}

	TSharedPtr<spine::Attachment> FoundAttachment = InSkin.getAttachment(SlotIndex, AttachmentName);

	if (!FoundAttachment)
	{
		return nullptr;
	}

	TSharedPtr<spine::AtlasRegion> CurrentRegion;

	if (FoundAttachment->getRTTI().isExactly(RegionAttachment::rtti))
	{
		CurrentRegion = StaticCastSharedPtr<RegionAttachment>(FoundAttachment)->AttachmentAtlasRegion;
	}
	else if (FoundAttachment->getRTTI().isExactly(MeshAttachment::rtti))
	{
		CurrentRegion = StaticCastSharedPtr<MeshAttachment>(FoundAttachment)->AttachmentAtlasRegion;
	}
	else
	{
		return nullptr;
	}

	check(CurrentRegion);

	const FString& CurrentRegionName = CurrentRegion->RegionName;
	check(!CurrentRegionName.IsEmpty());

	const FString& UsedRegionName = CustomRegionName.IsEmpty() ? CurrentRegionName : CustomRegionName;

	TSharedPtr<spine::AtlasRegion> UsedRegion = InAtlas.findRegionInSpecificPage(UsedRegionName, CustomPageName);

	if (!UsedRegion)
	{
		return nullptr;
	}

	TSharedPtr<spine::Attachment> ResultAttachment;
	if (FoundAttachment->getRTTI().isExactly(RegionAttachment::rtti))
	{
		ResultAttachment=StaticCastSharedPtr<RegionAttachment>(FoundAttachment)->GetCloneWithNewAtlasRegion(UsedRegion->AsShared());
	}
	else if (FoundAttachment->getRTTI().isExactly(MeshAttachment::rtti))
	{
		ResultAttachment=StaticCastSharedPtr<MeshAttachment>(FoundAttachment)->GetCloneWithNewAtlasRegion(UsedRegion->AsShared());
	}
	else
	{
		checkNoEntry();
	}

	OutSlotIndex = SlotIndex;
	return ResultAttachment;
}

TSharedPtr<spine::Attachment> FReplaceSkinAttachmentDesc::FindOriginAttachment(spine::Skeleton& InSkeleton, spine::Skin& InSkin, spine::Atlas& InAtlas, int32& OutSlotIndex) const
{
	using namespace spine;


	if (!SkeletonAsset.IsValid())
	{
		return nullptr ;
	}

	check(SkeletonAsset->GetSpineSkeletonData().IsValid());

	if (SkeletonAsset->GetSpineSkeletonData() != InSkeleton.getData())
	{
		return nullptr;
	}

	int32 SlotIndex = InSkeleton.findSlotIndex(SlotName);
	if (SlotIndex == INDEX_NONE)
	{
		return nullptr;
	}

	TSharedPtr<spine::Attachment> FoundAttachment = InSkin.getAttachment(SlotIndex, AttachmentName);
	OutSlotIndex = SlotIndex;
	return FoundAttachment;
}

TMap<FParamID, FParamCurrentData> FUpdateMaterialRuntime::UpdateAndEvaluateCurrentState(float DeltaTime)
{
	TMap<FParamID, FParamCurrentData> ResultMap;

	if (TimeRemaining > 0)
	{
		TimeRemaining -= DeltaTime;
		TimeRemaining = FMath::Max(0.f, TimeRemaining);
	}

	if (bBlendingIn)
	{
		CurrentBlendInTime += DeltaTime;
	}

	if (bBlendingOut)
	{
		CurrentBlendOutTime += DeltaTime;
	}

	bool bIsProcessFinish = false;
	if (TimeRemaining == 0)
	{
		// finished!
		bIsProcessFinish = true;
	}
	else if (TimeRemaining < 0)
	{
		// indefinite
	}
	else if (TimeRemaining < TotalBlendOutTime)
	{
		// start blending out
		bBlendingOut = true;
		CurrentBlendOutTime = TotalBlendOutTime - TimeRemaining;
	}

	if (bBlendingIn)
	{
		if (CurrentBlendInTime > TotalBlendInTime)
		{
			// done blending in!
			bBlendingIn = false;
		}
	}
	if (bBlendingOut)
	{
		if (CurrentBlendOutTime > TotalBlendOutTime)
		{
			// done!!
			CurrentBlendOutTime = TotalBlendOutTime;
			bIsProcessFinish = true;
		}
	}

	// Do not update oscillation further if finished
	if (bIsProcessFinish == false)
	{
		// calculate blend weight. calculating separately and taking the minimum handles overlapping blends nicely.
		float const BlendInWeight = (bBlendingIn) ? (CurrentBlendInTime / TotalBlendInTime) : 1.f;
		float const BlendOutWeight = (bBlendingOut) ? (1.f - CurrentBlendOutTime / TotalBlendOutTime) : 1.f;
		float const CurrentBlendWeight = FMath::Min(BlendInWeight, BlendOutWeight);

		float const UsedScale = CurrentBlendWeight;

		if (UsedScale > 0.f)
		{
			const float TimePassed = this->Duration - this->TimeRemaining;

			for (const FUpdateMaterialParam&Param : Params)
			{
				FParamCurrentData& CurrentData = ResultMap.FindOrAdd(FParamID{ Param.BlendType ,Param.ParamName });

				if (IsValid(Param.ParamCurve))
				{
					float Percentage = FMath::Clamp(TimePassed / Duration, 0.f, 1.f);
					if (UCurveFloat* FloatCurve = Cast<UCurveFloat>(Param.ParamCurve))
					{
						CurrentData = FParamCurrentData(FloatCurve->GetFloatValue(Percentage));
					}
					else if (UCurveLinearColor* LinearColorCurve = Cast<UCurveLinearColor>(Param.ParamCurve))
					{
						CurrentData = FParamCurrentData(LinearColorCurve->GetLinearColorValue(Percentage));
					}
					else if (UCurveVector* VectorCurve = Cast<UCurveVector>(Param.ParamCurve))
					{
						CurrentData = FParamCurrentData(VectorCurve->GetVectorValue(Percentage));
					}
					else
					{
						checkNoEntry();
					}
				}
				else
				{
					check(Param.bIsConstValue);

					CurrentData = Param.bIsFloatValue ?
						FParamCurrentData(Param.FloatValue) :
						FParamCurrentData(Param.ColorValue);
				}
				CurrentData *= UsedScale;
			}
		}
	}
	return ResultMap;
}

TMap<FParamID, FParamCurrentData> FUpdateMaterialRuntime::GetZeroedFinishState() const
{
	TMap<FParamID, FParamCurrentData> ReturnValue;

	for (const FUpdateMaterialParam&Param : Params)
	{
		FParamCurrentData& CurrentData = ReturnValue.FindOrAdd(FParamID{ Param.BlendType ,Param.ParamName });

		if (UCurveFloat* FloatCurve = Cast<UCurveFloat>(Param.ParamCurve))
		{
			CurrentData = FParamCurrentData(0.f);
		}
		else if (UCurveLinearColor* LinearColorCurve = Cast<UCurveLinearColor>(Param.ParamCurve))
		{
			CurrentData = FParamCurrentData(FVector::ZeroVector);
		}
		else if (UCurveVector* VectorCurve = Cast<UCurveVector>(Param.ParamCurve))
		{
			CurrentData = FParamCurrentData(FVector::ZeroVector);
		}
		else if (Param.ParamCurve == nullptr)
		{
			check(Param.bIsConstValue);
			if (Param.bIsFloatValue)
			{
				CurrentData = FParamCurrentData(0.f);
			}
			else
			{
				CurrentData = FParamCurrentData(FVector::ZeroVector);
			}
		}
		else
		{
			checkNoEntry();
		}
	}

	return ReturnValue;
}

bool FUpdateMaterialParam::IsValid() const
{
	if (ParamCurve)
	{
		if (ParamCurve->IsA<UCurveFloat>() || ParamCurve->IsA<UCurveLinearColor>() || ParamCurve->IsA<UCurveVector>())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return bIsConstValue;
	}
}

void FSpineUtility::EnsureFullyLoaded(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	bool bLoadInternalReferences = false;

	if (Object->HasAnyFlags(RF_NeedLoad))
	{
		FLinkerLoad* Linker = Object->GetLinker();
		if (ensure(Linker))
		{
			Linker->Preload(Object);
			bLoadInternalReferences = true;
			check(!Object->HasAnyFlags(RF_NeedLoad));
		}
	}

	bLoadInternalReferences = bLoadInternalReferences || Object->HasAnyFlags(RF_NeedPostLoad | RF_NeedPostLoadSubobjects);

	Object->ConditionalPostLoad();
	Object->ConditionalPostLoadSubobjects();

	if (bLoadInternalReferences)
	{
		// Collect a list of all things this element owns
		TArray<UObject*> ObjectReferences;
		FReferenceFinder(ObjectReferences, nullptr, false, true, false, true).FindReferences(Object);

		// Iterate over the list, and preload everything so it is valid for refreshing
		for (UObject* Reference : ObjectReferences)
		{
			if (Reference->IsA<UObject>())
			{
				FSpineUtility::EnsureFullyLoaded(Reference);
			}
			//	if (Reference->IsA<UMovieSceneSequence>() || Reference->IsA<UMovieScene>() || Reference->IsA<UMovieSceneTrack>() || Reference->IsA<UMovieSceneSection>())

		}
	}
}
