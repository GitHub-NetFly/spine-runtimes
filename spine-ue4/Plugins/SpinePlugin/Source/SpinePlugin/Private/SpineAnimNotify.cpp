// Fill out your copyright notice in the Description page of Project Settings.

#include "SpineAnimNotify.h"
#include "SpineSkeletonDataAsset.h"
#include "SpineSkeletonAnimationComponent.h"

USpineAnimNotifyPayload* USpineAnimNotifyPayload::CreatePayload(USpineSkeletonDataAsset* SkeletonData, const FString& AnimName, UTrackEntry* TrackEntry)
{
	USpineAnimNotifyPayload* Payload = NewObject<USpineAnimNotifyPayload>();
	Payload->AnimName = AnimName;
	Payload->SkeletonData = SkeletonData;
	Payload->TrackEntry = TrackEntry;
	check(IsValid(TrackEntry));
	check(TrackEntry->IsValidAnimation());
	return Payload;

}
