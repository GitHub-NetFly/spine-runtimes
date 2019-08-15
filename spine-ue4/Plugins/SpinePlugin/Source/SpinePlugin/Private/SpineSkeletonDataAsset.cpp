

#include "SpineSkeletonDataAsset.h"
#include "spine/spine.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include "Runtime/Core/Public/Misc/MessageDialog.h"

#include "UObject/DevObjectVersion.h"
#include "EditorFramework/AssetImportData.h"


#include "SpinePlugin.h"

#include"SpineAtlasAsset.h"

#if WITH_EDITOR
#include "Factories/Factory.h"
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "Spine"


const FGuid FSpineSkeletonDataAssetObjectVersion::GUID(0xc6e154d2, 0x17c2fbab, 0xf7800b72, 0xa1e2f5f4);

FDevVersionRegistration GRegisterSpineObjectVersion(FSpineSkeletonDataAssetObjectVersion::GUID,
	FSpineSkeletonDataAssetObjectVersion::LatestVersion, TEXT("Dev-SpineSkeletonDataAsset"));

using namespace spine;


static bool checkVersion(const char* version) {
	String tokens[3];
	int currToken = 0;

	while (*version && currToken < 3) {
		if (*version == '.') {
			version++;
			currToken++;
			continue;
		}

		char str[2];
		str[0] = *version;
		str[1] = 0;
		tokens[currToken].append(str);
		version++;
	}
	int versionNumber[3];
	for (int i = 0; i < 3; i++)
		versionNumber[i] = atoi(tokens[i].buffer());

	return versionNumber[0] >= 3 && versionNumber[1] >= 8 && versionNumber[2] >= 12;
}

static bool checkJson(const char* jsonData) {
	Json json(jsonData);
	Json* skeleton = Json::getItem(&json, "skeleton");
	if (!skeleton) return false;
	const char* version = Json::getString(skeleton, "spine", 0);
	if (!version) return false;

	return checkVersion(version);
}

struct BinaryInput {
	const unsigned char* cursor;
	const unsigned char* end;
};

static unsigned char readByte(BinaryInput *input) {
	return *input->cursor++;
}

static int readVarint(BinaryInput *input, bool optimizePositive) {
	unsigned char b = readByte(input);
	int value = b & 0x7F;
	if (b & 0x80) {
		b = readByte(input);
		value |= (b & 0x7F) << 7;
		if (b & 0x80) {
			b = readByte(input);
			value |= (b & 0x7F) << 14;
			if (b & 0x80) {
				b = readByte(input);
				value |= (b & 0x7F) << 21;
				if (b & 0x80) value |= (readByte(input) & 0x7F) << 28;
			}
		}
	}

	if (!optimizePositive) {
		value = (((unsigned int)value >> 1) ^ -(value & 1));
	}

	return value;
}

static char *readString(BinaryInput *input) {
	int length = readVarint(input, true);
	char *string;
	if (length == 0) {
		return NULL;
	}
	string = SpineExtension::alloc<char>(length, __FILE__, __LINE__);
	memcpy(string, input->cursor, length - 1);
	input->cursor += length - 1;
	string[length - 1] = '\0';
	return string;
}

static bool checkBinary(const char* binaryData, int length) {
	BinaryInput input;
	input.cursor = (const unsigned char*)binaryData;
	input.end = (const unsigned char*)binaryData + length;
	SpineExtension::free(readString(&input), __FILE__, __LINE__);
	char* version = readString(&input);
	bool result = checkVersion(version);
	SpineExtension::free(version, __FILE__, __LINE__);
	return result;
}


#if WITH_EDITOR

void USpineSkeletonDataAsset::SetRawData(TArray<uint8> &Data)
{
	this->rawData.Empty();
	this->rawData.Append(Data);

	LoadInfo();

	CachedSkeletonData = BuildSkeletonData();

	//	MarkPackageDirty();
}

void USpineSkeletonDataAsset::BindEngineCallback()
{
	check(GEditor);

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(this, &USpineSkeletonDataAsset::OnAtlasReimport);
	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.AddUObject(this, &USpineSkeletonDataAsset::OnAtlasPostImport);
}

void USpineSkeletonDataAsset::OnAtlasReimport(UObject* InObj)
{
	USpineAtlasAsset* TestAtlasAsset = Cast<USpineAtlasAsset>(InObj);

	if (IsValid(TestAtlasAsset) && TestAtlasAsset == RelatedAtlasAsset)
	{
		CachedSkeletonData = BuildSkeletonData();
	}
}

void USpineSkeletonDataAsset::OnAtlasPostImport(class UFactory* InFactory, UObject* InObj)
{
	USpineAtlasAsset* TestAtlasAsset = Cast<USpineAtlasAsset>(InObj);

	if (IsValid(TestAtlasAsset) && TestAtlasAsset == RelatedAtlasAsset)
	{
		CachedSkeletonData = BuildSkeletonData();
	}
}

void USpineSkeletonDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property == nullptr || PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(USpineSkeletonDataAsset, RelatedAtlasAsset))
	{
		CachedSkeletonData = BuildSkeletonData();
	}
}

#endif





void USpineSkeletonDataAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif

	Super::GetAssetRegistryTags(OutTags);
}






void USpineSkeletonDataAsset::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif

	Super::PostInitProperties();
}

void USpineSkeletonDataAsset::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FSpineSkeletonDataAssetObjectVersion::GUID); //如果是保存,则写入当前ID.

	Super::Serialize(Ar);

	const int32 AssetVersion = Ar.CustomVer(FSpineSkeletonDataAssetObjectVersion::GUID);

	int32 x = 55;
}



void USpineSkeletonDataAsset::PostLoad()
{
	Super::PostLoad();

	const int32 SpineAssetVersion = GetLinkerCustomVersion(FSpineSkeletonDataAssetObjectVersion::GUID);

	EnsureFullyLoaded(RelatedAtlasAsset);

	CachedSkeletonData = BuildSkeletonData();

#if WITH_EDITOR
	if (SpineAssetVersion < FSpineSkeletonDataAssetObjectVersion::Add_bIsValid_RawData_Flag)
	{
		LoadInfo();
	}



	if (GIsEditor && !RegisteredReimportCallback)
	{
		if (GEditor)
		{
			BindEngineCallback();
		}
		else
		{
			FCoreDelegates::OnPostEngineInit.AddUObject(this, &ThisClass::BindEngineCallback);
		}
		RegisteredReimportCallback = true;
	}
#endif

}








void USpineSkeletonDataAsset::EnsureFullyLoaded(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	bool bLoadInternalReferences = false;

	if (Object->HasAnyFlags(RF_NeedLoad))
	{
		FLinkerLoad* Linker = Object->GetLinker();
		if (ensure(Linker))
		{
			Linker->Preload(Object);
			bLoadInternalReferences = true;
			check(!Object->HasAnyFlags(RF_NeedLoad));
		}
	}

	bLoadInternalReferences = bLoadInternalReferences || Object->HasAnyFlags(RF_NeedPostLoad | RF_NeedPostLoadSubobjects);

	Object->ConditionalPostLoad();
	Object->ConditionalPostLoadSubobjects();

	if (bLoadInternalReferences)
	{
		// Collect a list of all things this element owns
		TArray<UObject*> ObjectReferences;
		FReferenceFinder(ObjectReferences, nullptr, false, true, false, true).FindReferences(Object);

		// Iterate over the list, and preload everything so it is valid for refreshing
		for (UObject* Reference : ObjectReferences)
		{
			if (Reference->IsA<UObject>())
			{
				EnsureFullyLoaded(Reference);
			}
			//	if (Reference->IsA<UMovieSceneSequence>() || Reference->IsA<UMovieScene>() || Reference->IsA<UMovieSceneTrack>() || Reference->IsA<UMovieSceneSection>())

		}
	}
}

#if WITH_EDITOR
class SP_API NullAttachmentLoader : public AttachmentLoader {
public:

	virtual TSharedPtr < RegionAttachment>  newRegionAttachment(Skin& skin, const FString& name, const FString& path) 
	{
		return MakeShared<RegionAttachment>(name);
	}

	virtual TSharedPtr < MeshAttachment>  newMeshAttachment(Skin& skin, const FString& name, const FString& path) {
		return MakeShared<MeshAttachment>(name);
	}

	virtual TSharedPtr < BoundingBoxAttachment>  newBoundingBoxAttachment(Skin& skin, const FString& name) {
		return MakeShared<BoundingBoxAttachment>(name);
	}

	virtual TSharedPtr < PathAttachment>  newPathAttachment(Skin& skin, const FString& name) {
		return MakeShared<PathAttachment>(name);
	}

	virtual TSharedPtr < PointAttachment>  newPointAttachment(Skin& skin, const FString& name) {
		return MakeShared<PointAttachment>(name);
	}

	virtual TSharedPtr < ClippingAttachment> newClippingAttachment(Skin& skin, const FString& name) {
		return MakeShared<ClippingAttachment>(name);
	}

	virtual void configureAttachment(TSharedPtr < Attachment> attachment) {

	}
};


void USpineSkeletonDataAsset::LoadInfo()
{
	Bones.Empty();
	Skins.Empty();
	Slots.Empty();
	Attachments.Empty();
	Animations.Empty();
	Events.Empty();

	bIsValidJsonRawData = false;
	bIsValidBinaryRawData = false;

	/*int32 dataLen = rawData.Num();
	if (dataLen == 0)
	{
		return;
	}*/

	NullAttachmentLoader loader;
	TSharedPtr<SkeletonData> skeletonData;

	FString ErrorString;

	if (AssetImportData->GetFirstFilename().Contains(TEXT(".json")))
	{
		SkeletonJson json(&loader);
		if (!checkJson((const char*)rawData.GetData()))
		{
			ErrorString = FString::Printf(TEXT("Asset:[%s] invalid json data,couldn't pass checkJson check"), *GetNameSafe(this));
		}
		else if (auto TempPtr = json.readSkeletonData((const char*)rawData.GetData()))
		{
			skeletonData = MakeShareable(TempPtr);
			bIsValidJsonRawData = true;
		}
		else
		{
			ErrorString = UTF8_TO_TCHAR(json.getError().buffer());
		}
	}
	else
	{
		SkeletonBinary binary(&loader);
		if (!checkBinary((const char*)rawData.GetData(), rawData.Num()))
		{
			ErrorString = FString::Printf(TEXT("Asset:[%s] invalid binary data,couldn't pass checkBinary check"),*GetNameSafe(this));
		}
		else if (auto TempPtr = binary.readSkeletonData((const unsigned char*)rawData.GetData(),rawData.Num()))
		{
			skeletonData = MakeShareable(TempPtr);
			bIsValidBinaryRawData = true;
		}
		else
		{
			ErrorString = UTF8_TO_TCHAR(binary.getError().buffer());
		}
	}

	if (skeletonData.IsValid())
	{
		for (int i = 0; i < skeletonData->getBones().size(); i++)
			Bones.Add(skeletonData->getBones()[i]->getName().ToString());

		for (int i = 0; i < skeletonData->getSkins().Num(); i++)
			Skins.Add(skeletonData->getSkins()[i]->getName());

		for (int i = 0; i < skeletonData->getSlots().size(); i++)
		{
			Slots.Add(skeletonData->getSlots()[i]->getName());
			Attachments.Add(skeletonData->getSlots()[i]->getAttachmentName());
		}

		for (int i = 0; i < skeletonData->getAnimations().Num(); i++)
			Animations.Add(UTF8_TO_TCHAR(skeletonData->getAnimations()[i]->getName().buffer()));

		for (int i = 0; i < skeletonData->getEvents().size(); i++)
			Events.Add(UTF8_TO_TCHAR(skeletonData->getEvents()[i]->getName().buffer()));

		TArray<FSpineAnimEventPair> PreSettingArray = MoveTemp(AnimEventSetupArray);
		for (const FString& EventName : Events)
		{
			FSpineAnimEventPair& AddEmptyPair = AnimEventSetupArray.AddDefaulted_GetRef();
			AddEmptyPair.EventName = EventName;

			if (FSpineAnimEventPair* FoundValidPairPtr = PreSettingArray.FindByPredicate([&EventName](const FSpineAnimEventPair& TestPair) { return TestPair.EventName == EventName && TestPair.RemapToGameplayEventTag.IsValid();  }))
			{
				AddEmptyPair.RemapToGameplayEventTag = FoundValidPairPtr->RemapToGameplayEventTag;
			}
		}
	}
	else
	{
		FMessageDialog::Debugf(FText::FromString(ErrorString));
		UE_LOG(SpineLog, Error, TEXT("%s"), *ErrorString);
	}
}
#endif




TSharedPtr<spine::SkeletonData> USpineSkeletonDataAsset::BuildSkeletonData()
{
	if (!IsValid(RelatedAtlasAsset))
	{
		return nullptr;
	}

	auto SpineAtlas = RelatedAtlasAsset->GetAtlas();
	check(SpineAtlas.IsValid());

	spine::SkeletonData* MySkeletonData = nullptr;

	int dataLen = rawData.Num();
	FString ErrorString;
	if (bIsValidJsonRawData&&ensure(checkJson((const char*)rawData.GetData())))
	{
		SkeletonJson json(SpineAtlas.Get());
		MySkeletonData = json.readSkeletonData((const char*)rawData.GetData());
		if (!MySkeletonData)
		{
			ErrorString = FString::Printf(TEXT("Asset:[%s] Couldn't build skeleton data: %s"), *GetNameSafe(this), UTF8_TO_TCHAR(json.getError().buffer()));
		}
	}
	else if (bIsValidBinaryRawData&&ensure(checkBinary((const char*)rawData.GetData(), (int)rawData.Num())))
	{
		SkeletonBinary binary(SpineAtlas.Get());
		MySkeletonData = binary.readSkeletonData((const unsigned char*)rawData.GetData(), (int)rawData.Num());
		if (!MySkeletonData)
		{
			ErrorString = FString::Printf(TEXT("Asset:[%s] Couldn't build skeleton data: %s"), *GetNameSafe(this), UTF8_TO_TCHAR(binary.getError().buffer()));
		}
	}
	else
	{
		ErrorString = FString::Printf(TEXT("Asset:[%s] Couldn't build skeleton data Unknow Reason"), *GetNameSafe(this));
	}

	if (!MySkeletonData)
	{
#if WITH_EDITOR
	//	FMessageDialog::Debugf(FText::FromString(ErrorString));
#endif
		UE_LOG(SpineLog, Error, TEXT("%s"), *ErrorString);
	}


	return MySkeletonData ? MakeShareable(MySkeletonData) : nullptr;
}

TSharedPtr<spine::SkeletonData> USpineSkeletonDataAsset::GetSpineSkeletonData()const
{
	return CachedSkeletonData;
}

TSharedPtr<spine::AnimationStateData> USpineSkeletonDataAsset::GetAnimationStateData() const
{
	TSharedPtr<spine::SkeletonData> SkeletonData = GetSpineSkeletonData();

	if (SkeletonData.IsValid()==false)
	{
		return nullptr;
	}

	TSharedRef<spine::AnimationStateData> ReturnAnimStateData = MakeShared<spine::AnimationStateData>(SkeletonData->AsShared());

	for (auto& data : MixData) 
	{
		if (!data.From.IsEmpty() && !data.To.IsEmpty())
		{
			const char* fromChar = TCHAR_TO_UTF8(*data.From);
			const char* toChar = TCHAR_TO_UTF8(*data.To);
			ReturnAnimStateData->setMix(fromChar, toChar, data.Mix);
		}
	}
	ReturnAnimStateData->setDefaultMix(DefaultMix);

	return   ReturnAnimStateData;
}




#undef LOCTEXT_NAMESPACE
