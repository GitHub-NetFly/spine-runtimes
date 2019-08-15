

#pragma once
#include "CoreMinimal.h"
#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(SpineLog, Log, All);

class SPINEPLUGIN_API SpinePlugin : public IModuleInterface {

public:
	
	static inline SpinePlugin& Get() {
		return FModuleManager::LoadModuleChecked< SpinePlugin >( "SpinePlugin" );
	}
	
	static inline bool IsAvailable() {
		return FModuleManager::Get().IsModuleLoaded( "SpinePlugin" );
	}
};

