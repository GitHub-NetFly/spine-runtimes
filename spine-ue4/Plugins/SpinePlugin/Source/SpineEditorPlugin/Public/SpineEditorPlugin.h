

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorDelegates.h"

class FSpineEditorPlugin : public IModuleInterface {

public:
	/*static inline ISpineEditorPlugin& Get() {
		return FModuleManager::LoadModuleChecked< ISpineEditorPlugin >( "SpineEditorPlugin" );
	}

	static inline bool IsAvailable() {
		return FModuleManager::Get().IsModuleLoaded( "SpineEditorPlugin" );
	}*/

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	void BindAssetImportCallback();

	void RemoveAssetImportCallback();

	void OnAtlasReimport(UObject* InObj);

	void OnAtlasPostImport(class UFactory* InFactory, UObject* InObj);
	

	void RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate);

	void RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate);

private:
	/** List of registered class that we must unregister when the module shuts down */
	TSet< FName > RegisteredClassNames;
	TSet< FName > RegisteredPropertyTypes;
};

