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

UENUM(BlueprintType)
enum class ESpineMaterialBlendType :uint8
{
	Normal,
	Additive,
	Multiply,
	Screen,

	UnKnown UMETA(Hidden),
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


USTRUCT(BlueprintType)
struct FUpdateMaterialParam
{
	GENERATED_BODY()
public:
	FUpdateMaterialParam()
	{
		ParamCurve = nullptr;
		bIsConstValue = false;
		bIsFloatValue = false;
		FloatValue = 0;
		ColorValue = FLinearColor::Black;
		BlendInTime = 0;
		BlendOutTime = 0;
		ParamName = NAME_None;
		BlendType = ESpineMaterialBlendType::UnKnown;
	}

	bool IsValid()const;


	UPROPERTY(BlueprintReadOnly)
	class UCurveBase* ParamCurve;

	UPROPERTY(BlueprintReadOnly)
	bool bIsConstValue;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFloatValue;

	UPROPERTY(BlueprintReadOnly)
	float FloatValue;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor ColorValue;

	UPROPERTY(BlueprintReadOnly)
	float BlendInTime;

	UPROPERTY(BlueprintReadOnly)
	float BlendOutTime;

	UPROPERTY(BlueprintReadOnly)
	FName ParamName;

	UPROPERTY(BlueprintReadOnly)
	ESpineMaterialBlendType BlendType;

	bool operator==(const FUpdateMaterialParam& Other) const
	{
		return FUpdateMaterialParam::StaticStruct()->CompareScriptStruct(this, &Other, 0);
	}

	bool operator!=(const FUpdateMaterialParam& Other) const
	{
		return !(*this == Other);
	}


	static FUpdateMaterialParam MakeCurveTypeMaterialParam(FName ParamName,
		class UCurveBase* ParamCurve, ESpineMaterialBlendType BlendType)
	{
		FUpdateMaterialParam Param;
		Param.ParamName = ParamName;
		Param.ParamCurve = ParamCurve;
		Param.BlendType = BlendType;
		Param.bIsConstValue = false;

		return Param;
	}

	static FUpdateMaterialParam MakeFloatTypeMaterialParam(FName ParamName,
		float FloatValue, ESpineMaterialBlendType BlendType)
	{
		FUpdateMaterialParam Param;
		Param.ParamName = ParamName;
		Param.bIsConstValue = true;
		Param.bIsFloatValue = true;
		Param.FloatValue = FloatValue;
		Param.BlendType = BlendType;

		return Param;
	}



	static FUpdateMaterialParam MakeColorTypeMaterialParam(FName ParamName,
		FLinearColor ColorValue, ESpineMaterialBlendType BlendType)
	{
		FUpdateMaterialParam Param;
		Param.ParamName = ParamName;
		Param.bIsConstValue = true;
		Param.bIsFloatValue = false;
		Param.ColorValue = ColorValue;
		Param.BlendType = BlendType;

		return Param;
	}


};

USTRUCT(BlueprintType)
struct FUpdateMaterialHandle
{
	GENERATED_BODY()
public:
	FUpdateMaterialHandle()
		:Handle(INDEX_NONE)
	{
	}


	FUpdateMaterialHandle(int32 InHandle) :Handle(InHandle)
	{
	}

	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	static FUpdateMaterialHandle GenerateNewHandle()
	{
		static int32 GHandleID = 0;

		FUpdateMaterialHandle NewHandle(GHandleID++);

		return NewHandle;
	}

	bool operator==(const FUpdateMaterialHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FUpdateMaterialHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	friend uint32 GetTypeHash(const FUpdateMaterialHandle& InHandle)
	{
		return InHandle.Handle;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%d"), Handle);
	}

	void Invalidate()
	{
		Handle = INDEX_NONE;
	}

private:
	UPROPERTY()
		int32 Handle;
};

DECLARE_DELEGATE_OneParam(FOnUpdateMaterialFinishedDelegate,bool);
DECLARE_DYNAMIC_DELEGATE_OneParam(FK2OnUpdateMaterialFinishedDelegate, bool, bWasCanceled);

struct FParamID
{
	ESpineMaterialBlendType BlendType;

	FName Name;

	friend uint32 GetTypeHash(const FParamID& InID) { return ::HashCombine((uint32)InID.BlendType, ::GetTypeHash(InID.Name)); }

	bool operator==(const FParamID& Other) const { return BlendType == Other.BlendType && Name == Other.Name; }

	bool operator!=(const FParamID& Other) const { return !operator==(Other); }
};

struct FParamCurrentData
{
	FParamCurrentData() = default;
	FParamCurrentData(float InValue) { bIsFloat = true; FloatValue = InValue; }

	FParamCurrentData(const FLinearColor& InValue) { bIsFloat = false; VectorValue = InValue; }

	bool bIsFloat = false;
	TOptional<float> FloatValue;
	TOptional<FLinearColor> VectorValue;


	FORCEINLINE FParamCurrentData operator*(float Scalar) const
	{
		return bIsFloat ?
			FParamCurrentData(FloatValue.Get(0)*Scalar) :
			FParamCurrentData(VectorValue.Get(FLinearColor::Black)*Scalar);
	}

	FORCEINLINE FParamCurrentData& operator*=(float Scalar)
	{
		FParamCurrentData Result = (*this)*Scalar;
		*this = Result;
		return *this;
	}

	FORCEINLINE FParamCurrentData operator/(float Scalar) const
	{
		return bIsFloat ?
			FParamCurrentData(FloatValue.Get(0)/Scalar) :
			FParamCurrentData(VectorValue.Get(FLinearColor::Black)/Scalar);
	}
	FORCEINLINE FParamCurrentData& operator/=(float Scalar)
	{
		FParamCurrentData Result = (*this)/Scalar;
		*this = Result;
		return *this;
	}

};

USTRUCT(BlueprintType)
struct FUpdateMaterialRuntime
{
	GENERATED_BODY()
public:

	UPROPERTY()
		FUpdateMaterialHandle Handle;

	UPROPERTY()
		TArray<FUpdateMaterialParam> Params;

	UPROPERTY()
		float Duration;

	UPROPERTY()
		float TotalBlendInTime;

	UPROPERTY()
		float TotalBlendOutTime;

	/*UPROPERTY()
		bool bSingleInstance;*/

	UPROPERTY()
	FName GroupName;

	UPROPERTY()
	bool bBlendingIn = false;

	UPROPERTY()
	bool bBlendingOut = false;

	UPROPERTY()
	float CurrentBlendInTime = 0;

	UPROPERTY()
	float CurrentBlendOutTime = 0;

	UPROPERTY()
	float TimeRemaining = 0;

public:


	FOnUpdateMaterialFinishedDelegate OnFinished;

	UPROPERTY()
		FK2OnUpdateMaterialFinishedDelegate K2OnFinished;

	bool operator==(const FUpdateMaterialRuntime& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FUpdateMaterialRuntime& Other) const
	{
		return Handle != Other.Handle;
	}

	TMap<FParamID, FParamCurrentData> UpdateAndEvaluateCurrentState(float DeltaTime);

	TMap<FParamID, FParamCurrentData> GetZeroedFinishState()const;

	bool IsLooping() const { return Duration < 0.f; }

	bool IsFinished() const { return TimeRemaining <= 0 && !IsLooping(); }
};




struct SPINEPLUGIN_API FSpineUtility
{
	static void EnsureFullyLoaded(UObject* Object);
};