

#include "SpineSkeletonImportFactory.h"

#include "SpineSkeletonDataAsset.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "PackageTools.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include <string>
#include <string.h>
#include <stdlib.h>

#include"SpineAtlasAsset.h"

#define LOCTEXT_NAMESPACE "Spine"

USpineSkeletonAssetFactory::USpineSkeletonAssetFactory (const FObjectInitializer& objectInitializer): Super(objectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = USpineSkeletonDataAsset::StaticClass();
	
	Formats.Add(TEXT("json;Spine skeleton file"));
	Formats.Add(TEXT("skel;Spine skeleton file"));
}

FText USpineSkeletonAssetFactory::GetToolTip () const {
	return LOCTEXT("USpineSkeletonAssetFactory", "Animations exported from Spine");
}

bool USpineSkeletonAssetFactory::FactoryCanImport (const FString& Filename) {
	return true;
}


bool LoadAtlas(const FString& Filename, const FString& TargetPath, USpineAtlasAsset* LoadedAtlasAsset)
{
	LoadedAtlasAsset = nullptr;

	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	FString skelFile = Filename.Replace(TEXT(".skel"), TEXT(".atlas")).Replace(TEXT(".json"), TEXT(".atlas"));
	if (!FPaths::FileExists(skelFile))
	{
		return false;
	}

	TArray<FString> fileNames;
	fileNames.Add(skelFile);
	TArray<UObject*>  ImportedObjects = AssetToolsModule.Get().ImportAssets(fileNames, TargetPath);

	if (ImportedObjects.Num() > 0)
	{
		LoadedAtlasAsset = CastChecked<USpineAtlasAsset>(ImportedObjects[0]);
		return true;
	}
	else
	{
		return false;
	}
}

UObject* USpineSkeletonAssetFactory::FactoryCreateFile(UClass * InClass, UObject * InParent, FName InName, EObjectFlags Flags, const FString & Filename, const TCHAR* Parms, FFeedbackContext * Warn, bool& bOutOperationCanceled)
{
	FString name(InName.ToString());
	name.Append("-data");

	USpineSkeletonDataAsset* asset = NewObject<USpineSkeletonDataAsset>(InParent, InClass, FName(*name), Flags);
	TArray<uint8> rawData;
	if (!FFileHelper::LoadFileToArray(rawData, *Filename, 0)) 
	{
		return nullptr;
	}
	asset->AssetImportData->UpdateFilenameOnly(Filename);
	asset->SetRawData(rawData);

	if (bIgnoreLoadOtherAsset == false)
	{
		const FString longPackagePath = FPackageName::GetLongPackagePath(asset->GetOutermost()->GetPathName());
		USpineAtlasAsset* LoadedAtlasAsset = nullptr;
		if (LoadAtlas(Filename, longPackagePath, LoadedAtlasAsset))
		{
			asset->RelatedAtlasAsset = LoadedAtlasAsset;
		}
	}

	return asset;
}

bool USpineSkeletonAssetFactory::CanReimport (UObject* Obj, TArray<FString>& OutFilenames)
{
	USpineSkeletonDataAsset* asset = Cast<USpineSkeletonDataAsset>(Obj);
	if (!asset||!asset->AssetImportData)
	{
		return false;
	}
	
	FString filename = asset->AssetImportData->GetFirstFilename();
	if (!filename.IsEmpty())
	{
		OutFilenames.Add(filename);
	}
	
	return true;
}

void USpineSkeletonAssetFactory::SetReimportPaths (UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	USpineSkeletonDataAsset* asset = Cast<USpineSkeletonDataAsset>(Obj);
	
	if (asset && ensure(NewReimportPaths.Num() == 1))
	{
		asset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type USpineSkeletonAssetFactory::Reimport(UObject* Obj)
{
	USpineSkeletonDataAsset* asset = Cast<USpineSkeletonDataAsset>(Obj);
	TArray<uint8> rawData;
	if (!FFileHelper::LoadFileToArray(rawData, *asset->AssetImportData->GetFirstFilename(), 0))
	{
		return EReimportResult::Failed;
	}
	asset->SetRawData(rawData);

	const FString longPackagePath = FPackageName::GetLongPackagePath(asset->GetOutermost()->GetPathName());

	if (bIgnoreLoadOtherAsset == false)
	{
		USpineAtlasAsset* LoadedAtlasAsset = nullptr;

		if (LoadAtlas(*asset->AssetImportData->GetFirstFilename(), longPackagePath, LoadedAtlasAsset))
		{
			asset->RelatedAtlasAsset = LoadedAtlasAsset;
		}
	}

	if (Obj->GetOuter())
	{
		Obj->GetOuter()->MarkPackageDirty();
	}
	else
	{
		Obj->MarkPackageDirty();
	}

	return EReimportResult::Succeeded;
}

#undef LOCTEXT_NAMESPACE
