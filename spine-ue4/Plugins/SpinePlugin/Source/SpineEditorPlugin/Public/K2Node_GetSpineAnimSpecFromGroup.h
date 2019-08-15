// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "SpineAnimationGroupDataAsset.h"
#include "K2Node_GetSpineAnimSpecFromGroup.generated.h"

/**
 * 
 */
class USpineSkeletonDataAsset;
class USpineAnimGroupAsset;

UCLASS(MinimalAPI)
class  UK2Node_GetSpineAnimSpecFromGroup : public UK2Node
{
	GENERATED_BODY()


private:
	UPROPERTY(EditAnywhere, meta = (AllowedClasses = "SpineAnimGroupAsset"))
	TSoftObjectPtr<USpineAnimGroupAsset> SpineAnimGroupAsset;

	UPROPERTY()
	FSpineAnimationSpec AnimationSpec;


	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

public:
	USpineAnimGroupAsset* GetAnimGroupAsset()const;

	void SetAnimGroupAsset(const FAssetData& InAssetData);

	const FSpineAnimationSpec& GetAnimationSpec()const 	{return AnimationSpec;}

	void SetAnimationSpec(const FSpineAnimationSpec& NewAnimationSpec);

	FText GetGroupAssetName()const;

	void EnsureFullyLoaded(UObject* Object);

public:
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual bool IsNodePure() const override { return true; }
	virtual void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;
	virtual class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	virtual void PreloadRequiredAssets() override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;

#if WITH_EDITOR
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
#endif
	
};
