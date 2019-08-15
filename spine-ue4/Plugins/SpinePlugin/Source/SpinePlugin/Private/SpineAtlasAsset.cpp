

#include"SpineAtlasAsset.h"
#include "spine/spine.h"

#include "Misc/CString.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;

void USpineAtlasAsset::PostInitProperties() 
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif //WITH_EDITORONLY_DATA
	Super::PostInitProperties();
}

void USpineAtlasAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{

#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif //WITH_EDITORONLY_DATA

	Super::GetAssetRegistryTags(OutTags);
}


TSharedPtr<spine::Atlas> USpineAtlasAsset::BuildAtlas()
{
	auto AnsiChars = StringCast<ANSICHAR>(*rawData);

	TSharedRef<spine::Atlas> SpineAtlas = MakeShared<spine::Atlas>(AnsiChars.Get(), AnsiChars.Length(), "", nullptr);

	TArray<AtlasPage>& SpineAtlasPagesRef = SpineAtlas->getPages();

	TMap<FString, UTexture2D*> BeforeMap = MoveTemp(AtlasTexturePageMap);

	for (int32 i = 0, num = SpineAtlasPagesRef.Num(); i < num; i++)
	{
		AtlasPage& page = SpineAtlasPagesRef[i];

		if (auto FoundValue = BeforeMap.Find(page.name))
		{
			AtlasTexturePageMap.Add(page.name) = *FoundValue;
			page.SetRendererObject(*FoundValue);
		}
		else
		{
			AtlasTexturePageMap.Add(page.name) = nullptr;
		}
	}
	return SpineAtlas;
}

void USpineAtlasAsset::SetRawData(const FString &RawData)
{
	this->rawData = RawData;

	MyAtlas = BuildAtlas();
}


#if WITH_EDITOR
void USpineAtlasAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property&&PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USpineAtlasAsset, AtlasTexturePageMap))
	{
		TArray<AtlasPage>& SpineAtlasPagesRef = MyAtlas->getPages();

		for (int32 i = 0, num = SpineAtlasPagesRef.Num(); i < num; i++)
		{
			AtlasPage& page = SpineAtlasPagesRef[i];

			UTexture2D** TexturePtr = AtlasTexturePageMap.Find(page.name);
			if (ensureMsgf(TexturePtr,TEXT(" AtlasTexturePageMap shouldn't change key value.")))
			{
				page.SetRendererObject(*TexturePtr);
			}
			
		}
	}
}
#endif

void USpineAtlasAsset::PostLoad()
{
	Super::PostLoad();

	MyAtlas = BuildAtlas();

	if (atlasTexturePages_DEPRECATED.Num() > 0) //do upgrade
	{
		TArray<AtlasPage>& SpineAtlasPagesRef = MyAtlas->getPages();

		for (int32 i = 0, num = SpineAtlasPagesRef.Num(); i < num; i++)
		{
			AtlasPage& page = SpineAtlasPagesRef[i];

			if (atlasTexturePages_DEPRECATED.IsValidIndex(i))
			{
				AtlasTexturePageMap.Add(page.name) = atlasTexturePages_DEPRECATED[i];

				page.SetRendererObject(atlasTexturePages_DEPRECATED[i]);
			}
		}
		atlasTexturePages_DEPRECATED.Empty();
		
	}

}

#undef LOCTEXT_NAMESPACE

