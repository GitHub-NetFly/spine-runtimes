

#include "SpineEditorPlugin.h"
#include "spine/spine.h"

#include "PropertyEditorModule.h"

#include "SpineAnimationSpecLayout.h"
#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"

#include "Factories/Factory.h"
#include "Editor.h"


IMPLEMENT_MODULE(FSpineEditorPlugin, SpineEditorPlugin)

void FSpineEditorPlugin::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	RegisterCustomPropertyTypeLayout("SpineAnimationSpec", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSpineAnimSpecLayout::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	//暂时注释掉,因为Spine Atlas更新后,旧的Spine Skeleton调用RebuildCachedSkeletonData()会导致问题.
	//应该只能在Spine Skeleton更新后,才可以调用RebuildCachedSkeletonData().

	/*if (GIsEditor)
	{
		if (GEditor)
		{
			BindAssetImportCallback();
		}
		else
		{
			FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSpineEditorPlugin::BindAssetImportCallback);
		}
	}*/
}

void FSpineEditorPlugin::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// Unregister all classes customized by name
		for (auto It = RegisteredClassNames.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomClassLayout(*It);
			}
		}

		RegisteredClassNames.Empty();

		// Unregister all structures
		for (auto It = RegisteredPropertyTypes.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomPropertyTypeLayout(*It);
			}
		}

		RegisteredPropertyTypes.Empty();

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	/*RemoveAssetImportCallback();
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);*/
}



void FSpineEditorPlugin::BindAssetImportCallback()
{
	check(GEditor);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddRaw(this, &FSpineEditorPlugin::OnAtlasReimport);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.AddRaw(this, &FSpineEditorPlugin::OnAtlasPostImport);
}

void FSpineEditorPlugin::RemoveAssetImportCallback()
{
	if (GEditor)
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.RemoveAll(this);
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.RemoveAll(this);
	}
}

void FSpineEditorPlugin::OnAtlasReimport(UObject* InObj)
{
	if (IsValid(InObj) && InObj->IsA<USpineAtlasAsset>())
	{
		USpineAtlasAsset* ChangedAtlasAsset = CastChecked<USpineAtlasAsset>(InObj);

		for (TObjectIterator<USpineSkeletonDataAsset> Itr; Itr; ++Itr)
		{
			USpineSkeletonDataAsset* SkeletonDataAsset = *Itr;
			if (IsValid(SkeletonDataAsset) && !SkeletonDataAsset->HasAnyFlags(RF_ClassDefaultObject))
			{
				if (SkeletonDataAsset->RelatedAtlasAsset == ChangedAtlasAsset)
				{
					SkeletonDataAsset->RebuildCachedSkeletonData();
				}
			}
		}
	}
}

void FSpineEditorPlugin::OnAtlasPostImport(class UFactory* InFactory, UObject* InObj)
{
	if (IsValid(InObj) && InObj->IsA<USpineAtlasAsset>())
	{
		USpineAtlasAsset* ChangedAtlasAsset = CastChecked<USpineAtlasAsset>(InObj);

		for (TObjectIterator<USpineSkeletonDataAsset> Itr; Itr; ++Itr)
		{
			USpineSkeletonDataAsset* SkeletonDataAsset = *Itr;
			if (IsValid(SkeletonDataAsset) && !SkeletonDataAsset->HasAnyFlags(RF_ClassDefaultObject))
			{
				if (SkeletonDataAsset->RelatedAtlasAsset == ChangedAtlasAsset)
				{
					SkeletonDataAsset->RebuildCachedSkeletonData();
				}
			}
		}
	}
}

void FSpineEditorPlugin::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);

	RegisteredClassNames.Add(ClassName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

void FSpineEditorPlugin::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);

	RegisteredPropertyTypes.Add(PropertyTypeName);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}
