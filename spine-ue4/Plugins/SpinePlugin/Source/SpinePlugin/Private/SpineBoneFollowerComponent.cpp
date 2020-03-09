

#include "SpineBoneFollowerComponent.h"

//
//USpineBoneFollowerComponent::USpineBoneFollowerComponent () {	
//	PrimaryComponentTick.bCanEverTick = true;
//	bTickInEditor = true;
//	bAutoActivate = true;
//}
//
//void USpineBoneFollowerComponent::BeginPlay () {
//	Super::BeginPlay();
//}
//
//void USpineBoneFollowerComponent::TickComponent ( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) {
//	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
//
//	if (Target) {
//		USpineSkeletonComponent* skeleton = static_cast<USpineSkeletonComponent*>(Target->GetComponentByClass(USpineSkeletonComponent::StaticClass()));
//		if (skeleton) {
//			FTransform transform = skeleton->GetBoneWorldTransform(BoneName);
//			if (UseComponentTransform) {
//				if (UsePosition) SetWorldLocation(transform.GetLocation());
//				if (UseRotation) SetWorldRotation(transform.GetRotation());
//				if (UseScale) SetWorldScale3D(transform.GetScale3D());
//			}
//			else {
//				AActor* owner = GetOwner();
//				if (owner) {
//					if (UsePosition) owner->SetActorLocation(transform.GetLocation());
//					if (UseRotation) owner->SetActorRotation(transform.GetRotation());
//					if (UseScale) owner->SetActorScale3D(transform.GetScale3D());
//				}
//			}
//		}
//	}
//}
//
