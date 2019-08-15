

#include "SpinePlugin.h"
#include "spine/Extension.h"

DEFINE_LOG_CATEGORY(SpineLog);

class FSpinePlugin : public SpinePlugin {
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FSpinePlugin, SpinePlugin )

void FSpinePlugin::StartupModule() {
}

void FSpinePlugin::ShutdownModule() { }

class Ue4Extension : public spine::DefaultSpineExtension {
public:
	Ue4Extension() : spine::DefaultSpineExtension() { }

	virtual ~Ue4Extension() { }

	virtual void *_alloc(int32 size, const char *file, int line) {
		return FMemory::Malloc(size);
	}

	virtual void *_calloc(int32 size, const char *file, int line) {
		void * result = FMemory::Malloc(size);
		FMemory::Memset(result, 0, size);
		return result;
	}

	virtual void *_realloc(void *ptr, int32 size, const char *file, int line) {
		return FMemory::Realloc(ptr, size);
	}

	virtual void _free(void *mem, const char *file, int line) {
		FMemory::Free(mem);
	}
};

spine::SpineExtension* spine::getDefaultExtension() {
	return new Ue4Extension();
}



