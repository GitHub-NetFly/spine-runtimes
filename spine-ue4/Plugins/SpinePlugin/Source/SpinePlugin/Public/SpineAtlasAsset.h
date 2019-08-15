
#pragma once


#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include <spine/Atlas.h>
#include "SpineAtlasAsset.generated.h"


namespace spine
{
	class Skeleton;
	class SkeletonData;
	class AnimationState;
	class AnimationStateData;
	class Atlas;
	class AtlasPage;
}

class USpineSkeletonComponent;
class USpineAtlasAsset;
class USpineSkeletonDataAsset;



UCLASS(BlueprintType, ClassGroup = (Spine))
class SPINEPLUGIN_API USpineAtlasAsset : public UObject 
{
	GENERATED_BODY()

public:
	TSharedPtr<class spine::Atlas> GetAtlas() { return MyAtlas; }

	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TMap<FString, UTexture2D*> AtlasTexturePageMap;

	void SetRawData(const FString &RawData);

private:
	UPROPERTY()
	TArray<UTexture2D*> atlasTexturePages_DEPRECATED;

public:

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Instanced, Category = ImportSettings)
	class  UAssetImportData* AssetImportData;
#endif //WITH_EDITORONLY_DATA

protected:

	UPROPERTY()
	FString rawData;

private:
	TSharedPtr<class spine::Atlas> BuildAtlas();

	TSharedPtr<class spine::Atlas> MyAtlas;

private:
#if WITH_EDITOR

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif
	

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//virtual void Serialize (FArchive& Ar) override;
};
