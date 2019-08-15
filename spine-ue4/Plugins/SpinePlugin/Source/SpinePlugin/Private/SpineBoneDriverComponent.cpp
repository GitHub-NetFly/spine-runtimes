

#include "SpineBoneDriverComponent.h"
#include "SpineSkeletonComponent.h"
#include "GameFramework/Actor.h"
//
//USpineBoneDriverComponent::USpineBoneDriverComponent () {
//	PrimaryComponentTick.bCanEverTick = true;
//	bTickInEditor = true;
//	bAutoActivate = true;
//}
//
//void USpineBoneDriverComponent::BeginPlay () {
//	Super::BeginPlay();
//}
//
//void USpineBoneDriverComponent::BeforeUpdateWorldTransform(USpineSkeletonComponent* skeleton) {	
//	if (skeleton == lastBoundComponent) {		
//		if (UseComponentTransform) {
//			skeleton->SetBoneWorldPosition(BoneName, GetComponentLocation());
//		}
//		else {
//			AActor* owner = GetOwner();
//			if (owner) skeleton->SetBoneWorldPosition(BoneName, owner->GetActorLocation());
//		}
//	}
//}
//
//void USpineBoneDriverComponent::TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	if (Target) {
//		USpineSkeletonComponent* skeleton = static_cast<USpineSkeletonComponent*>(Target->GetComponentByClass(USpineSkeletonComponent::StaticClass()));
//		if (skeleton != lastBoundComponent) {
//			// if (lastBoundComponent) lastBoundComponent->BeforeUpdateWorldTransform.RemoveAll(this);
//			if (!skeleton->BeforeUpdateWorldTransform.GetAllObjects().Contains(this))
//				skeleton->BeforeUpdateWorldTransform.AddDynamic(this, &USpineBoneDriverComponent::BeforeUpdateWorldTransform);
//			lastBoundComponent = skeleton;
//		}		
//	}
//}
//
