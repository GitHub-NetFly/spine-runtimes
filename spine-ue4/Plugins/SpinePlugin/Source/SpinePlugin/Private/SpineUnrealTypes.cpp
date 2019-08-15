// Fill out your copyright notice in the Description page of Project Settings.


#include "SpineUnrealTypes.h"

#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"
#include "spine/spine.h"

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