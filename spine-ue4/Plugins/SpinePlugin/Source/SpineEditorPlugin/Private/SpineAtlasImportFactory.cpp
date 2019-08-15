

#include "SpineAtlasImportFactory.h"

#include "SpineAtlasAsset.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "PackageTools.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"


#include "spine/spine.h"

#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;

USpineAtlasAssetFactory::USpineAtlasAssetFactory (const FObjectInitializer& objectInitializer): Super(objectInitializer) {
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = USpineAtlasAsset::StaticClass();
	
	Formats.Add(TEXT("atlas;Spine Atlas file"));
}

FText USpineAtlasAssetFactory::GetToolTip () const {
	return LOCTEXT("SpineAtlasAssetFactory", "Animations exported from Spine");
}

bool USpineAtlasAssetFactory::FactoryCanImport (const FString& Filename) {
	return true;
}

UObject* USpineAtlasAssetFactory::FactoryCreateFile (UClass * InClass, UObject * InParent, FName InName, EObjectFlags Flags, const FString & Filename, const TCHAR* Parms, FFeedbackContext * Warn, bool& bOutOperationCanceled) {
	FString rawString;
	if (!FFileHelper::LoadFileToString(rawString, *Filename)) {
		return nullptr;
	}
	
	FString currentSourcePath, filenameNoExtension, unusedExtension;
	const FString longPackagePath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetPathName());
	FPaths::Split(UFactory::GetCurrentFilename(), currentSourcePath, filenameNoExtension, unusedExtension);
	FString name(InName.ToString());
	name.Append("-atlas");
	
	USpineAtlasAsset* asset = NewObject<USpineAtlasAsset>(InParent, InClass, FName(*name), Flags);
	asset->SetRawData(rawString);
	
	asset->AssetImportData->UpdateFilenameOnly(Filename);

	if (bIgnoreLoadOtherAsset==false)
	{
		LoadAtlas(asset, currentSourcePath, longPackagePath);
	}
	

	return asset;
}

bool USpineAtlasAssetFactory::CanReimport (UObject* Obj, TArray<FString>& OutFilenames) {
	USpineAtlasAsset* asset = Cast<USpineAtlasAsset>(Obj);
	if (!asset) return false;
	
	FString filename = asset->AssetImportData->GetFirstFilename();
	if (!filename.IsEmpty())
		OutFilenames.Add(filename);
	
	return true;
}

void USpineAtlasAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	USpineAtlasAsset* asset = Cast<USpineAtlasAsset>(Obj);

	if (asset && ensure(NewReimportPaths.Num() == 1))
	{
		asset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type USpineAtlasAssetFactory::Reimport (UObject* Obj) 
{
	USpineAtlasAsset* asset = Cast<USpineAtlasAsset>(Obj);
	FString rawString;
	if (!FFileHelper::LoadFileToString(rawString, *asset->AssetImportData->GetFirstFilename()))
	{
		return EReimportResult::Failed;
	}
	asset->SetRawData(rawString);
	
	FString currentSourcePath, filenameNoExtension, unusedExtension;
	const FString longPackagePath = FPackageName::GetLongPackagePath(asset->GetOutermost()->GetPathName());
	FPaths::Split(UFactory::GetCurrentFilename(), currentSourcePath, filenameNoExtension, unusedExtension);
	

	if (bIgnoreLoadOtherAsset == false)
	{
		LoadAtlas(asset, currentSourcePath, longPackagePath);
	}
	
	
	if (Obj->GetOuter()) Obj->GetOuter()->MarkPackageDirty();
	else Obj->MarkPackageDirty();
	
	return EReimportResult::Succeeded;
}

UTexture2D* resolveTexture (USpineAtlasAsset* Asset, const FString& PageFileName, const FString& TargetSubPath) {
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	
	TArray<FString> fileNames;
	fileNames.Add(PageFileName);
		
	TArray<UObject*> importedAsset = AssetToolsModule.Get().ImportAssets(fileNames, TargetSubPath);
	UTexture2D* texture = (importedAsset.Num() > 0) ? Cast<UTexture2D>(importedAsset[0]) : nullptr;
	
	return texture;
}

void USpineAtlasAssetFactory::LoadAtlas (USpineAtlasAsset* Asset, const FString& CurrentSourcePath, const FString& LongPackagePath)
{
	/*auto atlas = Asset->GetAtlas();
	Asset->atlasTexturePages.Empty();

	const FString targetTexturePath = LongPackagePath / TEXT("Textures");

	TArray<AtlasPage>& pages = atlas->getPages();
	for (int32 i = 0, n = pages.Num(); i < n; i++)
	{
		AtlasPage& page = pages[i];
		const FString sourceTextureFilename = FPaths::Combine(*CurrentSourcePath, page.name);
		UTexture2D* texture = resolveTexture(Asset, sourceTextureFilename, targetTexturePath);
		Asset->atlasTexturePages.Add(texture);
	}*/
}

#undef LOCTEXT_NAMESPACE
