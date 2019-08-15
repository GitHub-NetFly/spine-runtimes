
#include "SpineSkeletonComponent.h"
#include "spine/spine.h"

#include"SpineAtlasAsset.h"
#include "SpineSkeletonDataAsset.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "SpinePlugin.h"


#define LOCTEXT_NAMESPACE "Spine"

using namespace spine;

USpineSkeletonComponent::USpineSkeletonComponent ()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

	bUseComplexAsSimpleCollision = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);


	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterialRef(TEXT("/SpinePlugin/Custom/M_Spine_Lit_Masked"));
	NormalBlendMaterial = NormalMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Unit_Additive"));
	AdditiveBlendMaterial = AdditiveMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Unit_Additive"));
	MultiplyBlendMaterial = MultiplyMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ScreenMaterialRef(TEXT("/SpinePlugin/Custom/MI_Spine_Lit_Translucent"));
	ScreenBlendMaterial = ScreenMaterialRef.Object;

	TextureParameterName = FName(TEXT("SpriteTexture"));
	DepthOffset = 0.001f;
	Color = FLinearColor(1, 1, 1, 1);
	bCreateCollision = false;
}

void USpineSkeletonComponent::SetSkeletonAsset(USpineSkeletonDataAsset* InSkeletonAsset)
{
	if (IsValid(InSkeletonAsset)&& InSkeletonAsset!=SkeletonData)
	{
		SkeletonData = InSkeletonAsset;
		CheckState();
		OnSpineSkeletonUpdated.Broadcast();
	}
}

bool USpineSkeletonComponent::SetSkin(FString skinName)
{
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		TSharedPtr<Skin> skin = CurrentSpineSkeleton->getData()->findSkin(skinName);
		if (!skin)
		{
			return false;
		}
		CurrentSpineSkeleton->setSkin(skin);
		CurrentSpineSkeleton->setSlotsToSetupPose();
		OnSpineSkinUpdated.Broadcast(skinName);
		return true;
	}
	return false;
}

void USpineSkeletonComponent::GetSkins (TArray<FString> &Skins) const
{
	const_cast<ThisClass*>(this)->CheckState();

	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getData()->getSkins().Num(); i < n; i++) {
			Skins.Add(CurrentSpineSkeleton->getData()->getSkins()[i]->getName());
		}
	}
}

bool USpineSkeletonComponent::HasSkin ( FString skinName)const
{
	const_cast<ThisClass*>(this)->CheckState();

	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findSkin(skinName) .IsValid();
	}
	return false;
}

bool USpineSkeletonComponent::SetAttachment(const FString slotName, const FString attachmentName) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		if (!CurrentSpineSkeleton->getAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName)))
		{
			return false;
		}
		CurrentSpineSkeleton->setAttachment(TCHAR_TO_UTF8(*slotName), TCHAR_TO_UTF8(*attachmentName));
		return true;
	}
	return false;
}

bool USpineSkeletonComponent::GetBoneWorldTransform(const FString& BoneName, FTransform& OutTransform)const
{
	OutTransform = FTransform::Identity;

	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		spine::Bone* bone = CurrentSpineSkeleton->findBone(TCHAR_TO_UTF8(*BoneName));
		if (!bone)
		{

			return false;
		}
		if (!bone->isAppliedValid())
		{
			const_cast<ThisClass*>(this)->InternalTick_SkeletonPose(0);
		}

		// Need to fetch the renderer component to get world transform of actor plus
		// offset by renderer component and its parent component(s). If no renderer
		// component is found, this components owner's transform is used as a fallback
		FTransform baseTransform = GetComponentTransform();
		FVector position(bone->getWorldX(), 0, bone->getWorldY());
		FMatrix localTransform;
		localTransform.SetIdentity();
		localTransform.SetAxis(2, FVector(bone->getA(), 0, bone->getC()));
		localTransform.SetAxis(0, FVector(bone->getB(), 0, bone->getD()));
		localTransform.SetOrigin(FVector(bone->getWorldX(), 0, bone->getWorldY()));
		localTransform = localTransform * baseTransform.ToMatrixWithScale();

		FTransform result;
		result.SetFromMatrix(localTransform);

		OutTransform = result;
		return true;
	}
	return false;
}

//void USpineSkeletonComponent::SetBoneWorldPosition (const FString& BoneName, const FVector& position) 
//{
//	CheckState();
//	if (CurrentSpineSkeleton.IsValid())
//	{
//		Bone* bone = CurrentSpineSkeleton->findBone(TCHAR_TO_UTF8(*BoneName));
//		if (!bone) return;
//		if (!bone->isAppliedValid()) this->InternalTick_SkeletonPose(0, false);
//
//		// Need to fetch the renderer component to get world transform of actor plus
//		// offset by renderer component and its parent component(s). If no renderer
//		// component is found, this components owner's transform is used as a fallback
//		FTransform baseTransform;
//		AActor* owner = GetOwner();
//		if (owner) {
//			USpineSkeletonRendererComponent* rendererComponent = static_cast<USpineSkeletonRendererComponent*>(owner->GetComponentByClass(USpineSkeletonRendererComponent::StaticClass()));
//			if (rendererComponent) baseTransform = rendererComponent->GetComponentTransform();
//			else baseTransform = owner->GetActorTransform();
//		}
//
//		baseTransform = baseTransform.Inverse();
//		FVector localPosition = baseTransform.TransformPosition(position);
//		float localX = 0, localY = 0;
//		if (bone->getParent()) {
//			bone->getParent()->worldToLocal(localPosition.X, localPosition.Z, localX, localY);
//		} else {
//			bone->worldToLocal(localPosition.X, localPosition.Z, localX, localY);
//		}
//		bone->setX(localX);
//		bone->setY(localY);
//	}
//}

void USpineSkeletonComponent::UpdateWorldTransform () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->updateWorldTransform();
	}
}

void USpineSkeletonComponent::SetToSetupPose () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setToSetupPose();
	}
}

void USpineSkeletonComponent::SetBonesToSetupPose () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setBonesToSetupPose();
	}
		
		
}

void USpineSkeletonComponent::SetSlotsToSetupPose () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setSlotsToSetupPose();
	}
}

void USpineSkeletonComponent::SetScaleX (float scaleX) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setScaleX(scaleX);
	}
}

float USpineSkeletonComponent::GetScaleX () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getScaleX();
	}
	return 1;
}

void USpineSkeletonComponent::SetScaleY (float scaleY) {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->setScaleY(scaleY);
	}
}

float USpineSkeletonComponent::GetScaleY () {
	CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getScaleY();
	}
	return 1;
}

void USpineSkeletonComponent::GetBones (TArray<FName> &BoneNames) const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getBones().size(); i < n; i++) 
		{
			BoneNames.Add(CurrentSpineSkeleton->getBones()[i]->getData().getName());
		}
	}
}

bool USpineSkeletonComponent::HasBone ( FName BoneName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findBone(BoneName) != nullptr;
	}
	return false;
}

void USpineSkeletonComponent::GetSlots (TArray<FString> &Slots)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getSlots().Num(); i < n; i++) {
			Slots.Add(CurrentSpineSkeleton->getSlots()[i]->getData().getName());
		}
	}
}

bool USpineSkeletonComponent::HasSlot ( FString SlotName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findSlot(TCHAR_TO_UTF8(*SlotName)) != nullptr;
	}
	return false;
}

void USpineSkeletonComponent::GetAnimations(TArray<FString> &Animations)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		for (int32 i = 0, n = CurrentSpineSkeleton->getData()->getAnimations().Num(); i < n; i++) {
			Animations.Add(CurrentSpineSkeleton->getData()->getAnimations()[i]->getName().buffer());
		}
	}
}

bool USpineSkeletonComponent::HasAnimation(FString AnimationName)const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		return CurrentSpineSkeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName)).Get() != nullptr;
	}
	return false;
}

float USpineSkeletonComponent::GetAnimationDuration(FString AnimationName) const
{
	const_cast<ThisClass*>(this)->CheckState();
	if (CurrentSpineSkeleton.IsValid())
	{
		Animation *animation = CurrentSpineSkeleton->getData()->findAnimation(TCHAR_TO_UTF8(*AnimationName)).Get();
		if (animation == nullptr) return 0;
		else return animation->getDuration();
	}
	return 0;
}


void USpineSkeletonComponent::TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InternalTick_SkeletonPose(DeltaTime);

	InternalTick_Renderer();
}

void USpineSkeletonComponent::InternalTick_SkeletonPose(float DeltaTime)
{
	CheckState();

	if (CurrentSpineSkeleton.IsValid())
	{
		CurrentSpineSkeleton->updateWorldTransform();
	}
}

void USpineSkeletonComponent::CheckState()
{
	bool bNeedsUpdate = (lastSkeletonAsset != SkeletonData);

	if (!bNeedsUpdate)
	{
		// Are we doing a re-import? Then check if the underlying spine-cpp data
		if (IsValid(SkeletonData))
		{
			auto TestSkeletonData = SkeletonData->GetSpineSkeletonData();
			if (lastSkeletonData != TestSkeletonData)
			{
				bNeedsUpdate = true;
			}
		}

	}

	if (bForceRefresh)
	{
		bForceRefresh = false;
		bNeedsUpdate = true;
		UE_LOG(SpineLog, Display, TEXT("USpineSkeletonComponent::CheckState Force Refresh!!!!!!!!!!!!!!!"));
	}

	if (bNeedsUpdate)
	{
		DisposeState();

		if (SkeletonData)
		{
			TSharedPtr<spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
			lastSkeletonData = SpineSkeletonData;

			if (SpineSkeletonData.IsValid())
			{
				CurrentSpineSkeleton = MakeShared<spine::Skeleton>(SpineSkeletonData->AsShared());
			}
		}


		lastSkeletonAsset = SkeletonData;
	}
}

void USpineSkeletonComponent::DisposeState () 
{
	CurrentSpineSkeleton.Reset();
	lastSkeletonData.Reset();
}

void USpineSkeletonComponent::InternalTick_Renderer()
{
	if (IsValid(SkeletonData) && IsValid(SkeletonData->RelatedAtlasAsset) &&
		SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid() &&
		GetSkeleton().IsValid())
	{
		USpineAtlasAsset* AtlasAsset = SkeletonData->RelatedAtlasAsset;

		GetSkeleton()->getColor().set(Color.R, Color.G, Color.B, Color.A);

		pageToNormalBlendMaterial.Empty(6);
		pageToAdditiveBlendMaterial.Empty(6);
		pageToMultiplyBlendMaterial.Empty(6);
		pageToScreenBlendMaterial.Empty(6);

		if (atlasNormalBlendMaterials.Num() != AtlasAsset->GetAtlas()->getPages().Num())
		{
			atlasNormalBlendMaterials.Empty(6);
			atlasAdditiveBlendMaterials.Empty(6);
			atlasMultiplyBlendMaterials.Empty(6);
			atlasScreenBlendMaterials.Empty(6);

			for (const spine::AtlasPage& Page : AtlasAsset->GetAtlas()->getPages())
			{
				UTexture2D* Texture = Page.GetRendererObject().Get();
				ensure(IsValid(Texture));

				UMaterialInstanceDynamic* NormalMID = UMaterialInstanceDynamic::Create(NormalBlendMaterial, this);
				UMaterialInstanceDynamic* AdditiveMID = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, this);
				UMaterialInstanceDynamic* MultiplyMID = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, this);
				UMaterialInstanceDynamic* ScreenMID = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, this);

				NormalMID->SetTextureParameterValue(TextureParameterName, Texture);
				AdditiveMID->SetTextureParameterValue(TextureParameterName, Texture);
				MultiplyMID->SetTextureParameterValue(TextureParameterName, Texture);
				ScreenMID->SetTextureParameterValue(TextureParameterName, Texture);

				atlasNormalBlendMaterials.Add(NormalMID);
				atlasAdditiveBlendMaterials.Add(AdditiveMID);
				atlasMultiplyBlendMaterials.Add(MultiplyMID);
				atlasScreenBlendMaterials.Add(ScreenMID);

				pageToNormalBlendMaterial.Add(Page, NormalMID);
				pageToAdditiveBlendMaterial.Add(Page, AdditiveMID);
				pageToMultiplyBlendMaterial.Add(Page, MultiplyMID);
				pageToScreenBlendMaterial.Add(Page, ScreenMID);
			}
		}
		else
		{
			for (int32 i = 0; i < AtlasAsset->GetAtlas()->getPages().Num(); i++)
			{
				const spine::AtlasPage& PageRef = AtlasAsset->GetAtlas()->getPages()[i];

				UTexture2D* texture = PageRef.GetRendererObject().Get();
				UTexture* oldTexture = nullptr;

				UMaterialInstanceDynamic* CurrentNormalMID = atlasNormalBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentAdditiveMID = atlasAdditiveBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentMultiplyMID = atlasMultiplyBlendMaterials[i];
				UMaterialInstanceDynamic* CurrentScreenMID = atlasScreenBlendMaterials[i];

				if (!CurrentNormalMID || !CurrentNormalMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(NormalBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasNormalBlendMaterials[i] = material;
				}
				//pageToNormalBlendMaterial.Add(PageRef, atlasNormalBlendMaterials[i]);

				if (!CurrentAdditiveMID || !CurrentAdditiveMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(AdditiveBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasAdditiveBlendMaterials[i] = material;
				}

				if (!CurrentMultiplyMID || !CurrentMultiplyMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(MultiplyBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasMultiplyBlendMaterials[i] = material;
				}

				if (!CurrentScreenMID || !CurrentScreenMID->GetTextureParameterValue(TextureParameterName, oldTexture) || oldTexture != texture)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(ScreenBlendMaterial, this);
					material->SetTextureParameterValue(TextureParameterName, texture);
					atlasScreenBlendMaterials[i] = material;
				}

				pageToNormalBlendMaterial.Add(PageRef, atlasNormalBlendMaterials[i]);
				pageToAdditiveBlendMaterial.Add(PageRef, atlasAdditiveBlendMaterials[i]);
				pageToMultiplyBlendMaterial.Add(PageRef, atlasMultiplyBlendMaterials[i]);
				pageToScreenBlendMaterial.Add(PageRef, atlasScreenBlendMaterials[i]);

			}
		}
		UpdateMesh(GetSkeleton()->AsShared());
	}
	else
	{
		ClearAllMeshSections();
	}
}

void USpineSkeletonComponent::UpdateMesh(TSharedRef<class spine::Skeleton> SkeletonRef)
{
	TArray<FVector> vertices;
	TArray<int32> indices;
	TArray<FVector> normals;
	TArray<FVector2D> uvs;
	TArray<FColor> colors;
	TArray<FVector> darkColors;

	int idx = 0;
	int meshSection = 0;
	UMaterialInstanceDynamic* lastMaterial = nullptr;

	ClearAllMeshSections();

	// Early out if skeleton is invisible
	if (GetSkeleton()->getColor().a == 0) return;

	struct FStaticSpineVector
	{

		spine::Vector<float> worldVertices;

		FStaticSpineVector()
		{
			worldVertices.ensureCapacity(1024 * 2);
		}
	};

	static FStaticSpineVector StaticSpineVector;

	spine::SkeletonClipping clipper;



	float depthOffset = 0;
	unsigned short quadIndices[] = { 0, 1, 2, 0, 2, 3 };

	for (int32 i = 0; i < GetSkeleton()->getSlots().Num(); ++i) {
		Vector<float> &attachmentVertices = StaticSpineVector.worldVertices;
		unsigned short* attachmentIndices = nullptr;
		int numVertices;
		int numIndices;
		TSharedPtr<AtlasRegion>  attachmentAtlasRegion;
		spine::Color attachmentColor;
		attachmentColor.set(1, 1, 1, 1);
		float* attachmentUvs = nullptr;

		TSharedPtr<Slot> slot = GetSkeleton()->getDrawOrder()[i];
		Attachment* attachment = slot->getAttachment();

		if (slot->getColor().a == 0 || !slot->getBone().isActive()) {
			clipper.clipEnd(*slot);
			continue;
		}

		if (!attachment) continue;
		if (!attachment->getRTTI().isExactly(RegionAttachment::rtti) && !attachment->getRTTI().isExactly(MeshAttachment::rtti) && !attachment->getRTTI().isExactly(ClippingAttachment::rtti)) continue;

		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			RegionAttachment* regionAttachment = (RegionAttachment*)attachment;

			// Early out if region is invisible
			if (regionAttachment->getColor().a == 0) {
				clipper.clipEnd(*slot);
				continue;
			}

			attachmentColor.set(regionAttachment->getColor());
			attachmentAtlasRegion = regionAttachment->AttachmentAtlasRegion;
			regionAttachment->computeWorldVertices(slot->getBone(), attachmentVertices, 0, 2);
			attachmentIndices = quadIndices;
			attachmentUvs = regionAttachment->getUVs().GetData();
			numVertices = 4;
			numIndices = 6;
		}
		else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			MeshAttachment* mesh = (MeshAttachment*)attachment;

			// Early out if region is invisible
			if (mesh->getColor().a == 0) {
				clipper.clipEnd(*slot);
				continue;
			}

			attachmentColor.set(mesh->getColor());
			attachmentAtlasRegion = mesh->AttachmentAtlasRegion;
			mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), attachmentVertices, 0, 2);
			attachmentIndices = mesh->getTriangles().buffer();
			attachmentUvs = mesh->getUVs().buffer();
			numVertices = mesh->getWorldVerticesLength() >> 1;
			numIndices = mesh->getTriangles().size();
		}
		else /* clipping */ {
			ClippingAttachment* clip = (ClippingAttachment*)attachment;
			clipper.clipStart(*slot, clip);
			continue;
		}

		// if the user switches the atlas data while not having switched
		// to the correct skeleton data yet, we won't find any regions.
		// ignore regions for which we can't find a material
		UMaterialInstanceDynamic* material = nullptr;
		const auto& AtlasPage = attachmentAtlasRegion->Page;

		switch (slot->getData().getBlendMode()) {
		case BlendMode_Normal:
			if (!pageToNormalBlendMaterial.Contains(AtlasPage)) continue;
			material = pageToNormalBlendMaterial[AtlasPage];
			break;
		case BlendMode_Additive:
			if (!pageToAdditiveBlendMaterial.Contains(AtlasPage)) continue;
			material = pageToAdditiveBlendMaterial[AtlasPage];
			break;
		case BlendMode_Multiply:
			if (!pageToMultiplyBlendMaterial.Contains(AtlasPage)) continue;
			material = pageToMultiplyBlendMaterial[AtlasPage];
			break;
		case BlendMode_Screen:
			if (!pageToScreenBlendMaterial.Contains(AtlasPage)) continue;
			material = pageToScreenBlendMaterial[AtlasPage];
			break;
		default:
			if (!pageToNormalBlendMaterial.Contains(AtlasPage)) continue;
			material = pageToNormalBlendMaterial[AtlasPage];
		}

		if (clipper.isClipping()) {
			clipper.clipTriangles(attachmentVertices.buffer(), attachmentIndices, numIndices, attachmentUvs, 2);
			attachmentVertices = clipper.getClippedVertices();
			numVertices = clipper.getClippedVertices().size() >> 1;
			attachmentIndices = clipper.getClippedTriangles().buffer();
			numIndices = clipper.getClippedTriangles().size();
			attachmentUvs = clipper.getClippedUVs().buffer();
			if (clipper.getClippedTriangles().size() == 0) continue;
		}

		if (lastMaterial != material) {
			Flush(meshSection, vertices, indices, normals, uvs, colors, darkColors, lastMaterial);
			lastMaterial = material;
			idx = 0;
		}

		SetMaterial(meshSection, material);

		uint8 r = static_cast<uint8>(GetSkeleton()->getColor().r * slot->getColor().r * attachmentColor.r * 255);
		uint8 g = static_cast<uint8>(GetSkeleton()->getColor().g * slot->getColor().g * attachmentColor.g * 255);
		uint8 b = static_cast<uint8>(GetSkeleton()->getColor().b * slot->getColor().b * attachmentColor.b * 255);
		uint8 a = static_cast<uint8>(GetSkeleton()->getColor().a * slot->getColor().a * attachmentColor.a * 255);

		float dr = slot->hasDarkColor() ? slot->getDarkColor().r : 0.0f;
		float dg = slot->hasDarkColor() ? slot->getDarkColor().g : 0.0f;
		float db = slot->hasDarkColor() ? slot->getDarkColor().b : 0.0f;

		float* verticesPtr = attachmentVertices.buffer();
		for (int j = 0; j < numVertices << 1; j += 2) {
			colors.Add(FColor(r, g, b, a));
			darkColors.Add(FVector(dr, dg, db));
			vertices.Add(FVector(verticesPtr[j], depthOffset, verticesPtr[j + 1]));
			normals.Add(FVector(0, -1, 0));
			uvs.Add(FVector2D(attachmentUvs[j], attachmentUvs[j + 1]));
		}

		for (int j = 0; j < numIndices; j++) {
			indices.Add(idx + attachmentIndices[j]);
		}

		idx += numVertices;
		depthOffset += this->DepthOffset;

		clipper.clipEnd(*slot);
	}

	Flush(meshSection, vertices, indices, normals, uvs, colors, darkColors, lastMaterial);
	clipper.clipEnd();
}

void USpineSkeletonComponent::Flush(int &Idx, TArray<FVector> &Vertices, TArray<int32> &Indices, TArray<FVector> &Normals, TArray<FVector2D> &Uvs, TArray<FColor> &Colors, TArray<FVector> &Colors2, UMaterialInstanceDynamic* Material)
{
	if (Vertices.Num() == 0) return;
	SetMaterial(Idx, Material);

	CreateMeshSection(Idx, Vertices, Indices, Normals, Uvs, Colors, TArray<FProcMeshTangent>(), bCreateCollision);

	Vertices.SetNum(0);
	Indices.SetNum(0);
	Normals.SetNum(0);
	Uvs.SetNum(0);
	Colors.SetNum(0);
	Colors2.SetNum(0);
	Idx++;
}


void USpineSkeletonComponent::FinishDestroy () {
	DisposeState();
	Super::FinishDestroy();
}

FTransform USpineSkeletonComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace /*= RTS_World*/) const
{
	FTransform WorldSocketTransform;

	if (!GetBoneWorldTransform(InSocketName.ToString(), WorldSocketTransform))
	{
		WorldSocketTransform = GetComponentTransform();
	}

	switch (TransformSpace)
	{
		case RTS_World:
		{
			return WorldSocketTransform;
			break;
		}
		case RTS_Actor:
		{
			if (const AActor* Actor = GetOwner())
			{
				return WorldSocketTransform.GetRelativeTransform(Actor->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return WorldSocketTransform.GetRelativeTransform(GetComponentTransform());
		}
	}

	return WorldSocketTransform;
}

bool USpineSkeletonComponent::HasAnySockets() const
{
	if (GetSkeleton().IsValid() == false)
	{
		return false;
	}
	return GetSkeleton()->getBones().size() > 0;
}

void USpineSkeletonComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	//Super::QuerySupportedSockets(OutSockets);

	if (GetSkeleton().IsValid() == false)
	{
		return;
	}

	for (int32 i = 0; i < GetSkeleton()->getBones().size(); i++)
	{
		spine::Bone* Bone = GetSkeleton()->getBones()[i];
		FName SocketName = Bone->getData().getName();
		new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
	}
}

bool USpineSkeletonComponent::DoesSocketExist(FName InSocketName) const
{
	if (GetSkeleton().IsValid() == false)
	{
		return false;
	}

	spine::Bone* Bone = GetSkeleton()->findBone(TCHAR_TO_UTF8(*InSocketName.GetPlainNameString()));

	return Bone != nullptr;
}

bool USpineSkeletonComponent::ApplyReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup)
{
	CheckState();

	if (!CurrentSpineSkeleton)
	{
		return false;
	}

	if (!ensure(SkeletonData))
	{
		return false;
	}

	if (!ensure(SkeletonData->RelatedAtlasAsset))
	{
		return false;
	}

	check(CurrentSpineSkeleton.IsValid());
	check(SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid());

	TSharedPtr< spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
	check(SpineSkeletonData.IsValid());
	TSharedPtr< spine::Skin> FoundSkin = CurrentSpineSkeleton->getSkin(); /*SpineSkeletonData->findSkin(GetCurrentSkinName());*/

	if (!FoundSkin)
	{
		return false;
	}

	// 拷贝一份避免替换skin中的attachment时会影响原来的Skin ,同名确保了可以直接通过名字找回原来的默认版本的skin.
	TSharedPtr< spine::Skin> CopyedSkin = FoundSkin->GetClone(FoundSkin->getName());

	bool bAnyReplaced = false;

	for (const FReplaceSkinAttachmentDesc& OneReplace : ReplacementGroup.Replacements)
	{
		int32 SlotIndex = INDEX_NONE;

		TSharedPtr<spine::Attachment> NewAttachment = OneReplace.GenerateCopyAttachmentWithNewAtlasRegionIn(
			*CurrentSpineSkeleton,
			*FoundSkin,
			*SkeletonData->RelatedAtlasAsset->GetAtlas(),
			SlotIndex);

		if (!NewAttachment)
		{
			continue;
		}

		check(SlotIndex > INDEX_NONE);

		//override Attachment.
		CopyedSkin->setAttachment(SlotIndex, OneReplace.AttachmentName, NewAttachment);
		bAnyReplaced = true;
	}

	if (bAnyReplaced)
	{
		CurrentSpineSkeleton->setSkin(CopyedSkin);
		CurrentSpineSkeleton->setSlotsToSetupPose();
		// animation subclass need call SpineAnimState->apply(GetSkeleton()->AsShared().Get());
	}

	return bAnyReplaced;
}


bool USpineSkeletonComponent::CancelReplaceAttachment(const FReplaceAttachmentGroup& ReplacementGroup)
{
	CheckState();

	if (!CurrentSpineSkeleton)
	{
		return false;
	}

	if (!ensure(SkeletonData))
	{
		return false;
	}

	if (!ensure(SkeletonData->RelatedAtlasAsset))
	{
		return false;
	}

	check(CurrentSpineSkeleton.IsValid());
	check(SkeletonData->RelatedAtlasAsset->GetAtlas().IsValid());

	TSharedPtr< spine::SkeletonData> SpineSkeletonData = SkeletonData->GetSpineSkeletonData();
	check(SpineSkeletonData.IsValid());

	TSharedPtr< spine::Skin> CurrentSkin = CurrentSpineSkeleton->getSkin(); //因为拷贝的skin和原来的skin同名,所以可以通过名字找到原来的,

	TSharedPtr< spine::Skin> FoundOriginSkin = CurrentSkin ? SpineSkeletonData->findSkin(CurrentSkin->getName()) : nullptr;

	if (!FoundOriginSkin)
	{
		return false;
	}

	TSharedPtr< spine::Skin> CopyedSkin = CurrentSkin->GetClone(CurrentSkin->getName());

	bool bAnyReplaced = false;
	for (const FReplaceSkinAttachmentDesc& OneReplace : ReplacementGroup.Replacements)
	{
		int32 SlotIndex = INDEX_NONE;

		TSharedPtr<spine::Attachment> OriginAttachment=	OneReplace.FindOriginAttachment(
			*CurrentSpineSkeleton,
			*FoundOriginSkin,
			*SkeletonData->RelatedAtlasAsset->GetAtlas(),
			SlotIndex
		);

		if (!OriginAttachment)
		{
			continue;
		}

		CopyedSkin->setAttachment(SlotIndex, OneReplace.AttachmentName, OriginAttachment);
		bAnyReplaced = true;
	}

	if (bAnyReplaced)
	{
		CurrentSpineSkeleton->setSkin(CopyedSkin);
		CurrentSpineSkeleton->setSlotsToSetupPose();
		// animation subclass need call SpineAnimState->apply(GetSkeleton()->AsShared().Get());
	}
	return bAnyReplaced;
}

#undef LOCTEXT_NAMESPACE
