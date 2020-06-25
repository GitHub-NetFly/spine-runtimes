// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SpineAnimationGroupDataAsset.h"

#include "SpineSkeletonDataAsset.h"
#include "SpineUnrealTypes.h"

#include "SpineBPLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SPINEPLUGIN_API USpineBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	static FString ToString(const FSpineAnimationSpec& SpineAnimSpec);


	UFUNCTION(BlueprintPure, meta = ( BlueprintInternalUseOnly = "true"))
		static FSpineAnimationSpec Copy_SpineAnimationSpec(FSpineAnimationSpec InSpec) { return InSpec; }


	UFUNCTION(BlueprintCallable)
		static bool FetchSkinAttachmentDescFromDatable(UDataTable* InTable, TArray<FReplaceSkinAttachmentDesc>& OutAttachmentDescs);
		
	UFUNCTION(BlueprintPure)
	static FUpdateMaterialParam MakeCurveTypeMaterialParam(FName ParamName,
		class UCurveBase* ParamCurve, ESpineMaterialBlendType BlendType);

	UFUNCTION(BlueprintPure)
	static FUpdateMaterialParam MakeFloatTypeMaterialParam(FName ParamName,
		float FloatValue, ESpineMaterialBlendType BlendType);

	UFUNCTION(BlueprintPure)
	static FUpdateMaterialParam MakeColorTypeMaterialParam(FName ParamName,
		FLinearColor ColorValue, ESpineMaterialBlendType BlendType);
};

