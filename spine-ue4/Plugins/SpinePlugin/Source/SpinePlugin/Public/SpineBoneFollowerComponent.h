﻿
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
//#include "SpineBoneFollowerComponent.generated.h"
//
//
//UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
//class SPINEPLUGIN_API USpineBoneFollowerComponent : public USceneComponent {
//	GENERATED_BODY()
//
//public:
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	AActor* Target = 0;
//	
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	FString BoneName;
//
//	//Updates just this component when set to true. Updates owning actor otherwise.
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	bool UseComponentTransform = false;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	bool UsePosition = true;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	bool UseRotation = true;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	bool UseScale = true;
//	
//	USpineBoneFollowerComponent ();
//	
//	virtual void BeginPlay () override;
//		
//	virtual void TickComponent (float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//};
