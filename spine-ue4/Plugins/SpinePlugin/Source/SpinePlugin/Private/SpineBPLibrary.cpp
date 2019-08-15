// Fill out your copyright notice in the Description page of Project Settings.


#include "SpineBPLibrary.h"
#include "Engine/DataTable.h"

FString USpineBPLibrary::ToString(const FSpineAnimationSpec& SpineAnimSpec)
{
	return SpineAnimSpec.GetDebugString();
}

bool USpineBPLibrary::FetchSkinAttachmentDescFromDatable(UDataTable* InTable, TArray<FReplaceSkinAttachmentDesc>& OutAttachmentDescs)
{
	if (!IsValid(InTable))
	{
		return false;
	}

	if (InTable->GetRowStruct() != FReplaceSkinAttachmentDesc::StaticStruct())
	{
		return false;
	}

	bool bFoundAny = false;
	InTable->ForeachRow<FReplaceSkinAttachmentDesc>(TEXT("FetchSkinAttachmentDescFromDatable"),
		[&OutAttachmentDescs, &bFoundAny]
	(const FName& Key, const FReplaceSkinAttachmentDesc& Value)
	{
		OutAttachmentDescs.Add(Value);
		bFoundAny = true;
	});

	return bFoundAny;
}
