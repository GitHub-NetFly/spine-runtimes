// Fill out your copyright notice in the Description page of Project Settings.

#include "K2Node_GetSpineAnimSpecFromGroup.h"

#include "SpinePlugin.h"
#include "SpineSkeletonDataAsset.h"
#include "SpineAtlasAsset.h"
#include "SpineSkeletonComponent.h"
#include "SpineSkeletonAnimationComponent.h"


#include "SpineWidget.h"

#include "ToolMenu.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "BlueprintNodeSpawner.h"
#include "EditorCategoryUtils.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Framework/Application/SlateApplication.h"
#include "PropertyCustomizationHelpers.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "SGraphNode.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "GraphEditorSettings.h"
#include "Engine/Selection.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboBox.h"
#include "Editor.h"

#include "K2Node_CallFunction.h"
#include "SpineBPLibrary.h"
//#include "Compilation/MovieSceneCompiler.h"

#include "SpineUnrealTypes.h"

#define LOCTEXT_NAMESPACE "USpineSkeletonDataAsset"

#pragma optimize("",off)

static const FName OutputPinName(TEXT("Output"));
static const FName GroupAssetPinName(TEXT("GroupAsset"));

class FKCHandler_GetSpineAnimSpecFromGroup : public FNodeHandlingFunctor
{
public:
	FKCHandler_GetSpineAnimSpecFromGroup(FKismetCompilerContext& InCompilerContext)
		: FNodeHandlingFunctor(InCompilerContext)
	{
	}

	virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		

		UK2Node_GetSpineAnimSpecFromGroup* BindingNode = CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(Node);

		for (UEdGraphPin* Pin : BindingNode->GetAllPins())
		{
			if (Pin->Direction == EGPD_Output && Pin->LinkedTo.Num())
			{
				FBPTerminal* Term = RegisterLiteral(Context, Pin);

				FSpineAnimationSpec::StaticStruct()->ExportText(Term->Name, &BindingNode->GetAnimationSpec(), nullptr, nullptr, 0, nullptr);
				//FMovieSceneObjectBindingID::StaticStruct()->ExportText(Term->Name, &BindingNode->Binding, nullptr, nullptr, 0, nullptr);
			}
		}
	}
};


 void UK2Node_GetSpineAnimSpecFromGroup::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	static FName CopyFuncName = GET_FUNCTION_NAME_CHECKED(USpineBPLibrary, Copy_SpineAnimationSpec);
	static FName InParamName(TEXT("InSpec"));


	UK2Node_CallFunction* CallCopyNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

	UFunction* Function = USpineBPLibrary::StaticClass()->FindFunctionByName(CopyFuncName);
	CallCopyNode->SetFromFunction(Function);
	CallCopyNode->AllocateDefaultPins();


	UEdGraphPin* Copy_InPin = CallCopyNode->FindPinChecked(InParamName);
	UEdGraphPin* Copy_OutPin = CallCopyNode->GetReturnValuePin();
	UEdGraphPin* MyOutPin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);

	CompilerContext.MovePinLinksToIntermediate(*MyOutPin, *Copy_OutPin);
	Schema->TryCreateConnection(MyOutPin, Copy_InPin);

}

USpineAnimGroupAsset* UK2Node_GetSpineAnimSpecFromGroup::GetAnimGroupAsset() const
{
	return SpineAnimGroupAsset.LoadSynchronous();
}

void UK2Node_GetSpineAnimSpecFromGroup::SetAnimGroupAsset(const FAssetData& InAssetData)
{
	FSlateApplication::Get().DismissAllMenus();

	auto OldGroupAsset = SpineAnimGroupAsset;

	SpineAnimGroupAsset = Cast<USpineAnimGroupAsset>(InAssetData.GetAsset());
	if (OldGroupAsset!= SpineAnimGroupAsset)
	{
		AnimationSpec = FSpineAnimationSpec();

	}

	// Refresh the UI for the graph so the pin changes show up
	//GetGraph()->NotifyGraphChanged();

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}

void UK2Node_GetSpineAnimSpecFromGroup::SetAnimationSpec(const FSpineAnimationSpec& NewAnimationSpec)
{
	if (AnimationSpec!= NewAnimationSpec)
	{
		AnimationSpec = NewAnimationSpec;
		// Mark dirty
		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}
}

FText UK2Node_GetSpineAnimSpecFromGroup::GetGroupAssetName() const
{
	auto AnimGroupAsset = GetAnimGroupAsset();

	return AnimGroupAsset ? FText::FromName(AnimGroupAsset->GetFName()) : LOCTEXT("NoAnimGroupAsset", "No AnimGroupAsset");
}



void UK2Node_GetSpineAnimSpecFromGroup::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	if (!Context->bIsDebugging)
	{
		FToolMenuSection& Section = Menu->AddSection("K2NodeGetSequenceBinding", LOCTEXT("ThisNodeHeader", "This Node"));
		if (!Context->Pin)
		{
			USpineAnimGroupAsset* GroupAsset = GetAnimGroupAsset();

			Section.AddSubMenu(
				"SetSequence",
				LOCTEXT("SetSkeleton_Text", "Skeleton"),
				LOCTEXT("SetSkeleton_ToolTip", "Sets the GroupAsset to get animation from"),
				FNewToolMenuDelegate::CreateLambda([=](UToolMenu* SubMenu)
			{
				TArray<const UClass*> AllowedClasses({ USpineAnimGroupAsset::StaticClass() });

				TSharedRef<SWidget> MenuContent = PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
					FAssetData(GroupAsset),
					true /* bAllowClear */,
					AllowedClasses,
					PropertyCustomizationHelpers::GetNewAssetFactoriesForClasses(AllowedClasses),
					FOnShouldFilterAsset(),
					FOnAssetSelected::CreateUObject(const_cast<ThisClass*>(this), &UK2Node_GetSpineAnimSpecFromGroup::SetAnimGroupAsset),
					FSimpleDelegate());

				SubMenu->AddMenuEntry("Section", FToolMenuEntry::InitWidget("Widget", MenuContent, FText::GetEmpty(), false));

			}));
		}
	}
}

void UK2Node_GetSpineAnimSpecFromGroup::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	USpineAnimGroupAsset* GroupAsset = GetAnimGroupAsset();

	//这个阶段 获取到的对象,在加载蓝图时通过LoadSynchronous 载入的对象 依旧有RF_NeedLoad flag.
	// 所以需要进行一次EnsureFullyLoaded. 才能拿到载入完毕的对象.
	FSpineUtility::EnsureFullyLoaded(GroupAsset);

	if (!GroupAsset)
	{
		const FText MessageText = FText::FromString(FString::Printf(TEXT(" Couldn't Found SpineAnimGroupAsset")));
		MessageLog.Error(*MessageText.ToString(), this);
		return;
	}

	if (!AnimationSpec.RelatedSpineSkeletonDataAsset)
	{
		const FText MessageText = FText::FromString(FString::Printf(TEXT(" Not Select a valid AnimationSpec")));
		MessageLog.Error(*MessageText.ToString(), this);
		return;
	}

	auto FoundInSetPtr = GroupAsset->SpineSkeletonDataSet.Find(AnimationSpec.RelatedSpineSkeletonDataAsset);
	USpineSkeletonDataAsset* FoundSkeletonAsset = FoundInSetPtr ? *FoundInSetPtr : nullptr;

	if (!FoundSkeletonAsset)
	{
		const FText MessageText = FText::FromString(FString::Printf(TEXT(" Couldn't Found RelatedSpineSkeleton in SpineAnimGroupAsset")));
		MessageLog.Error(*MessageText.ToString(), this);
		return;
	}

	 if (FoundSkeletonAsset->Animations.Find(AnimationSpec.AnimationName)==INDEX_NONE)
	{
		const FText MessageText = FText::FromString(FString::Printf(TEXT(" Couldn't Found Anim[%s] in SpineSkeletonDataAsset[%s]"), *AnimationSpec.AnimationName, *GetNameSafe(FoundSkeletonAsset)));
		MessageLog.Error(*MessageText.ToString(), this);
		return;
	}
}

FText UK2Node_GetSpineAnimSpecFromGroup::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return (AnimationSpec.AnimationName.IsEmpty() || !IsValid(AnimationSpec.RelatedSpineSkeletonDataAsset))
		? LOCTEXT("NodeTitle", "Get Spine Anim Spec") :
		FText::Format(LOCTEXT("NodeTitle_Format", "Get Spine ({0})  Anim ({1})"), FText::FromString(AnimationSpec.RelatedSpineSkeletonDataAsset->GetName()), FText::FromString(AnimationSpec.AnimationName));
}


FText UK2Node_GetSpineAnimSpecFromGroup::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Get Spine Anim Spec");
}

FSlateIcon UK2Node_GetSpineAnimSpecFromGroup::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.GetSequenceBinding");
	return Icon;
}



class FNodeHandlingFunctor* UK2Node_GetSpineAnimSpecFromGroup::CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const
{
	return new FKCHandler_GetSpineAnimSpecFromGroup(CompilerContext);
}

void UK2Node_GetSpineAnimSpecFromGroup::PreloadRequiredAssets()
{
	PreloadObject(GetAnimGroupAsset());

	FSpineUtility::EnsureFullyLoaded(GetAnimGroupAsset());
	
	Super::PreloadRequiredAssets();
}

void UK2Node_GetSpineAnimSpecFromGroup::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

void UK2Node_GetSpineAnimSpecFromGroup::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, USpineAnimGroupAsset::StaticClass(), GroupAssetPinName);

	// Result pin
	UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, FSpineAnimationSpec::StaticStruct(), UEdGraphSchema_K2::PN_ReturnValue);
	//UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PN_ReturnValue);
	ResultPin->PinFriendlyName = LOCTEXT("SpineAnimationSpec", "AnimationSpec");

	Super::AllocateDefaultPins();
}

class SGraphNodeGetSpineAnim : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeGetSpineAnim) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UK2Node_GetSpineAnimSpecFromGroup* InNode)
	{
		bNeedsUpdate = false;
		GraphNode = InNode;
		Initialize();
		UpdateGraphNode();
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
	{
		UK2Node_GetSpineAnimSpecFromGroup* Node = CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode);
		USpineAnimGroupAsset* GroupAsset = Node->GetAnimGroupAsset();

		if (bNeedsUpdate || GroupAsset != LastGroupAsset.Get())
		{
			Initialize();
			UpdateGraphNode();

			bNeedsUpdate = false;
		}

		LastGroupAsset = GroupAsset;

		SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}

	virtual void CreateStandardPinWidget(UEdGraphPin* Pin) override
	{
		if (Pin->PinName == GroupAssetPinName)
		{
			CreateDetailsPickers();
		}
		else
		{
			SGraphNode::CreateStandardPinWidget(Pin);
		}
	}

	void OnAssetSelectedFromPicker(const FAssetData& AssetData)
	{
		CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->SetAnimGroupAsset(AssetData);
		Initialize();
		UpdateGraphNode();
	}

	FText GetAssetName() const
	{
		return CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->GetGroupAssetName();
	}

	TSharedRef<SWidget> GenerateAssetPicker()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		FAssetPickerConfig AssetPickerConfig;
		AssetPickerConfig.Filter.ClassNames.Add(USpineAnimGroupAsset::StaticClass()->GetFName());
		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SGraphNodeGetSpineAnim::OnAssetSelectedFromPicker);
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.bAllowDragging = false;

		return SNew(SBox)
			.HeightOverride(300)
			.WidthOverride(300)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
			]
			];
	}

	FReply UseSelectedAsset()
	{
		USpineAnimGroupAsset* CurrentSelectedAnimGroupAsset = Cast<USpineAnimGroupAsset>(GEditor->GetSelectedObjects()->GetTop(USpineAnimGroupAsset::StaticClass()));
		if (CurrentSelectedAnimGroupAsset)
		{
			CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->SetAnimGroupAsset(CurrentSelectedAnimGroupAsset);
			Initialize();
			UpdateGraphNode();
		}
		return FReply::Handled();
	}

	FReply BrowseToAsset()
	{
		USpineAnimGroupAsset* AnimGroupAsset = CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->GetAnimGroupAsset();
		if (AnimGroupAsset)
		{
			TArray<UObject*> Objects{ AnimGroupAsset };
			GEditor->SyncBrowserToObjects(Objects);
		}
		return FReply::Handled();
	}

	void CreateDetailsPickers()
	{
		LeftNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(Settings->GetInputPinPadding())
			[
				SNew(SHorizontalBox)

				// Asset Combo
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			.MaxWidth(200.0f)
			[
				SNew(SComboButton)
				.ButtonStyle(FEditorStyle::Get(), "PropertyEditor.AssetComboStyle")
			.ForegroundColor(this, &SGraphNodeGetSpineAnim::OnGetComboForeground)
			.ButtonColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetBackground)
			.ContentPadding(FMargin(2, 2, 2, 1))
			.MenuPlacement(MenuPlacement_BelowAnchor)
			.ButtonContent()
			[
				SNew(STextBlock)
				.ColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetComboForeground)
			.TextStyle(FEditorStyle::Get(), "PropertyEditor.AssetClass")
			.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(this, &SGraphNodeGetSpineAnim::GetAssetName)
			]
		.OnGetMenuContent(this, &SGraphNodeGetSpineAnim::GenerateAssetPicker)
			]

		// Use button
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.OnClicked(this, &SGraphNodeGetSpineAnim::UseSelectedAsset)
			.ButtonColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetBackground)
			.ContentPadding(1.f)
			.ToolTipText(LOCTEXT("GraphNodeGetSequenceBinding_Use_Tooltip", "Use asset browser selection"))
			[
				SNew(SImage)
				.ColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetForeground)
			.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Use")))
			]
			]

		// Browse button
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.OnClicked(this, &SGraphNodeGetSpineAnim::BrowseToAsset)
			.ButtonColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetBackground)
			.ContentPadding(0)
			.ToolTipText(LOCTEXT("GraphNodeGetSequenceBinding_Browse_Tooltip", "Browse"))
			[
				SNew(SImage)
				.ColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetForeground)
			.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Browse")))
			]
			]
			];

		LeftNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(Settings->GetInputPinPadding())
			[
				SNew(SBox)
				.MaxDesiredWidth(200.0f)
			.Padding(FMargin(2, 0))
			[
				SNew(SComboButton)
				.ButtonStyle(FEditorStyle::Get(), "PropertyEditor.AssetComboStyle")
			//	.ToolTipText(this, &SGraphNodeGetSpineAnim::GetToolTipText)
			.ForegroundColor(this, &SGraphNodeGetSpineAnim::OnGetComboForeground)
			.ButtonColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetWidgetBackground)
			.ContentPadding(FMargin(2, 2, 2, 1))
			.MenuPlacement(MenuPlacement_BelowAnchor)
			.ButtonContent()
			[
				GetCurrentItemWidget(
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "PropertyEditor.AssetClass")
					.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(this, &SGraphNodeGetSpineAnim::OnGetComboForeground)
				)
			]
		.OnGetMenuContent(this, &SGraphNodeGetSpineAnim::GetPickerMenu)
			]
			];
	}

private:

	TSharedRef<SWidget> GetPickerMenu()
	{
		// Close self only to enable use inside context menus
		FMenuBuilder MenuBuilder(true, nullptr, nullptr, true);

		Initialize();
		GetPickerMenu(MenuBuilder);

		// Hold onto the menu widget so we can destroy it manually
		TSharedRef<SWidget> MenuWidget = MenuBuilder.MakeWidget();
		DismissWidget = MenuWidget;
		return MenuWidget;

	}

	void GetPickerMenu(FMenuBuilder& MenuBuilder)
	{
		USpineAnimGroupAsset* AnimGroupAsset = CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->GetAnimGroupAsset();

		OnGetMenuContent(MenuBuilder, AnimGroupAsset ? AnimGroupAsset->AnimationSpecs : TArray<FSpineAnimationSpec>());
	}

	void Initialize()
	{
		UpdateCachedData();
	}

	void OnGetMenuContent(FMenuBuilder& MenuBuilder, const TArray<FSpineAnimationSpec>& SpineAnimationSpecs)
	{
		bool bHadAnyEntries = false;

		for (const FSpineAnimationSpec& SpineAnimationSpec : SpineAnimationSpecs)
		{
			bHadAnyEntries = true;

			FText DisplayText = FText::Format(LOCTEXT("GetMenuContent", "Spine({0}) Anim ({1})"), FText::FromString(GetNameSafe(SpineAnimationSpec.RelatedSpineSkeletonDataAsset)), FText::FromString(SpineAnimationSpec.AnimationName));

			MenuBuilder.AddMenuEntry(
				DisplayText,
				FText(),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateRaw(this, &SGraphNodeGetSpineAnim::SetSpineAnimationSpec, SpineAnimationSpec)
				)
			);

		}
	}

	void SetSpineAnimationSpec(FSpineAnimationSpec SpineAnimationSpec)
	{
		SetCurrentValue(SpineAnimationSpec);
		UpdateCachedData();


		TSharedPtr<SWidget> MenuWidget = DismissWidget.Pin();
		if (MenuWidget.IsValid())
		{
			FSlateApplication::Get().DismissMenuByWidget(MenuWidget.ToSharedRef());
		}
	}

	void UpdateCachedData()
	{
		const FSpineAnimationSpec& AnimationSpec = GetCurrentValue();

		auto CurrentGroupAsset = CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->GetAnimGroupAsset();

		if (!CurrentGroupAsset || AnimationSpec.AnimationName.IsEmpty() || !IsValid(AnimationSpec.RelatedSpineSkeletonDataAsset))
		{
			CurrentText = LOCTEXT("UnresolvedBinding", "Unresolved Binding");
		}
		else
		{
			FText DisplayText = FText::Format(LOCTEXT("CachedData", "Spine({0}) Anim ({1})"), FText::FromString(GetNameSafe(AnimationSpec.RelatedSpineSkeletonDataAsset)), FText::FromString(AnimationSpec.AnimationName));
			CurrentText = DisplayText;
		}
	}

	virtual void SetCurrentValue(const FSpineAnimationSpec& SpineAnimationSpec)
	{

		CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->SetAnimationSpec(SpineAnimationSpec);

		bNeedsUpdate = true;
	}
	virtual const FSpineAnimationSpec& GetCurrentValue() const
	{
		return CastChecked<UK2Node_GetSpineAnimSpecFromGroup>(GraphNode)->GetAnimationSpec();
	}


	FSlateColor OnGetComboForeground() const
	{
		return FSlateColor(FLinearColor(1.f, 1.f, 1.f, IsHovered() ? 1.f : 0.6f));
	}

	FSlateColor OnGetWidgetForeground() const
	{
		return FSlateColor(FLinearColor(1.f, 1.f, 1.f, IsHovered() ? 1.f : 0.15f));
	}

	FSlateColor OnGetWidgetBackground() const
	{
		return FSlateColor(FLinearColor(1.f, 1.f, 1.f, IsHovered() ? 0.8f : 0.4f));
	}

	FText GetCurrentText()const
	{
		return CurrentText;
	}

	TSharedRef<SWidget> GetCurrentItemWidget(TSharedRef<STextBlock> TextContent)
	{
		TextContent->SetText(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateRaw(this, &SGraphNodeGetSpineAnim::GetCurrentText)));

		return SNew(SHorizontalBox)

			//	+ SHorizontalBox::Slot()
			//	.AutoWidth()
			//	[
			//		SNew(SOverlay)

			//		+ SOverlay::Slot()
			//	[
			//		SNew(SImage)
			////		.Image_Raw(this, &FMovieSceneObjectBindingIDPicker::GetCurrentIconBrush)
			//	]

			//+ SOverlay::Slot()
			//	.VAlign(VAlign_Top)
			//	.HAlign(HAlign_Right)
			//	[
			//		SNew(SImage)
			//		.Visibility_Raw(this, &FMovieSceneObjectBindingIDPicker::GetSpawnableIconOverlayVisibility)
			//	.Image(FEditorStyle::GetBrush("Sequencer.SpawnableIconOverlay"))
			//	]
			//	]

			+SHorizontalBox::Slot()
			.Padding(4.f, 0, 0, 0)
			.VAlign(VAlign_Center)
			[
				TextContent
			];
	}
private:
	TWeakObjectPtr<USpineAnimGroupAsset> LastGroupAsset;
	bool bNeedsUpdate;

	FText CurrentText;

	/** Weak ptr to a widget used to dismiss menus to */
	TWeakPtr<SWidget> DismissWidget;
};


#if WITH_EDITOR
TSharedPtr<SGraphNode> UK2Node_GetSpineAnimSpecFromGroup::CreateVisualWidget()
{
	return SNew(SGraphNodeGetSpineAnim, this);
}
#endif

#undef LOCTEXT_NAMESPACE


#pragma optimize("",on)
