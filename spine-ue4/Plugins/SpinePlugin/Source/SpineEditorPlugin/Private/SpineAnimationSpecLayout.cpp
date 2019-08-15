// Fill out your copyright notice in the Description page of Project Settings.


#include "SpineAnimationSpecLayout.h"


#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SSearchBox.h"
#include "Framework/Commands/UIAction.h"

#include "SpineSkeletonDataAsset.h"
#include "SpineAnimationGroupDataAsset.h"


#define LOCTEXT_NAMESPACE "SpineAnimationSpecLayoutCustomizationLayout"

void FSpineAnimSpecLayout::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	this->StructPropertyHandle = InStructPropertyHandle;

	//if (StructPropertyHandle->HasMetaData(TEXT("RowType")))
	//{
	//	const FString& RowType = StructPropertyHandle->GetMetaData(TEXT("RowType"));
	////	RowTypeFilter = FName(*RowType);
	//}

	StructPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSpineAnimSpecLayout::OnReferencedDataAssetChanged));

	HeaderRow
		.NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget(FText::GetEmpty(), FText::GetEmpty(), false)
		];



	HeaderRow.AddCustomContextMenuAction(FUIAction(FExecuteAction::CreateSP(this, &FSpineAnimSpecLayout::OnSearchForReferences)), LOCTEXT("SearchForReferences", "Find Spine References"),
		LOCTEXT("SearchForReferencesTooltip", "Find assets that reference this "), FSlateIcon());

}

void FSpineAnimSpecLayout::CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	/** Get all the existing property handles */
	SpineSkeletonDataAssetPropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSpineAnimationSpec, RelatedSpineSkeletonDataAsset));
	AnimNamePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSpineAnimationSpec, AnimationName));

	if (SpineSkeletonDataAssetPropertyHandle->IsValidHandle() && AnimNamePropertyHandle->IsValidHandle())
	{
		/** Queue up a refresh of the selected item, not safe to do from here */
		StructCustomizationUtils.GetPropertyUtilities()->EnqueueDeferredAction(FSimpleDelegate::CreateSP(this, &FSpineAnimSpecLayout::OnReferencedDataAssetChanged));

		/** Setup Change callback */
		SpineSkeletonDataAssetPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSpineAnimSpecLayout::OnReferencedDataAssetChanged));

		/** Construct a asset picker widget with a custom filter */
		StructBuilder.AddCustomRow(LOCTEXT("SpineDataAsset", "SpineDataAsset"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SpineDataAsset", "SpineDataAsset"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MaxDesiredWidth(0.0f) // don't constrain the combo button width
			[
				SNew(SObjectPropertyEntryBox)
				.IsEnabled(InStructPropertyHandle, &IPropertyHandle::IsEditable)
				.PropertyHandle(SpineSkeletonDataAssetPropertyHandle)
				.AllowedClass(USpineSkeletonDataAsset::StaticClass())
			
		//	.OnShouldFilterAsset(this, &FSpineAnimSpecLayout::ShouldFilterAsset)
			];

		/** Construct a combo box widget to select from a list of valid options */
		StructBuilder.AddCustomRow(LOCTEXT("SpineAnimName", "SpineAnim"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SpineAnimName", "SpineAnim"))
			.Font(StructCustomizationUtils.GetRegularFont())
			]
		.ValueContent()
			.MaxDesiredWidth(0.0f) // don't constrain the combo button width
			[
				SAssignNew(RowNameComboButton, SComboButton)
				.IsEnabled(InStructPropertyHandle, &IPropertyHandle::IsEditable)
				.ToolTipText(this, &FSpineAnimSpecLayout::GetRowNameComboBoxContentText)
				.OnGetMenuContent(this, &FSpineAnimSpecLayout::GetListContent)
				.OnComboBoxOpened(this, &FSpineAnimSpecLayout::HandleMenuOpen)
				.ContentPadding(FMargin(2.0f, 2.0f))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FSpineAnimSpecLayout::GetRowNameComboBoxContentText)
				]
			];
	}
}

bool FSpineAnimSpecLayout::ShouldFilterAsset(const FAssetData& AssetData)
{
  
   //const USpineSkeletonDataAsset* SpineSkeletonData = Cast<USpineSkeletonDataAsset>(AssetData.GetAsset());

	return false;
}

TSharedPtr<FString> FSpineAnimSpecLayout::InitWidgetContent()
{
	TSharedPtr<FString> InitialValue = MakeShareable(new FString(LOCTEXT("None", "None").ToString()));;

	FString AnimName;
	const FPropertyAccess::Result RowResult = AnimNamePropertyHandle->GetValue(AnimName);
	Row_AnimNames.Empty();

	/** Get the properties we wish to work with */
	const USpineSkeletonDataAsset* SpineSkeleton = nullptr;
	SpineSkeletonDataAssetPropertyHandle->GetValue((UObject*&)SpineSkeleton);

	if (SpineSkeleton != nullptr)
	{
		/** Extract all the row names from the RowMap */


		for (const FString& Item_AnimName : SpineSkeleton->Animations)
		{
			/** Create a simple array of the row names */
			TSharedRef<FString> RowNameItem = MakeShareable(new FString(Item_AnimName));
			Row_AnimNames.Add(RowNameItem);

			/** Set the initial value to the currently selected item */
			if (AnimName== Item_AnimName)
			{
				InitialValue = RowNameItem;
			}
		}
	}

	/** Reset the initial value to ensure a valid entry is set */
	if (RowResult != FPropertyAccess::MultipleValues)
	{
		FString NewValue = FString(**InitialValue);
		AnimNamePropertyHandle->SetValue(NewValue);
	}

	return InitialValue;
}


TSharedRef<SWidget> FSpineAnimSpecLayout::GetListContent()
{
	SAssignNew(RowNameComboListView, SListView<TSharedPtr<FString> >)
		.ListItemsSource(&Row_AnimNames)
		.OnSelectionChanged(this, &FSpineAnimSpecLayout::OnSelectionChanged)
		.OnGenerateRow(this, &FSpineAnimSpecLayout::HandleRowNameComboBoxGenarateWidget)
		.SelectionMode(ESelectionMode::Single);

	// Ensure no filter is applied at the time the menu opens
	OnFilterTextChanged(FText::GetEmpty());

	if (CurrentSelectedItem.IsValid())
	{
		RowNameComboListView->SetSelection(CurrentSelectedItem);
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(SearchBox, SSearchBox)
			.OnTextChanged(this, &FSpineAnimSpecLayout::OnFilterTextChanged)
		]

	+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBox)
			.MaxDesiredHeight(600)
		[
			RowNameComboListView.ToSharedRef()
		]
		];
}

void FSpineAnimSpecLayout::OnReferencedDataAssetChanged()
{
	CurrentSelectedItem = InitWidgetContent();
	if (RowNameComboListView.IsValid())
	{
		RowNameComboListView->SetSelection(CurrentSelectedItem);
		RowNameComboListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> FSpineAnimSpecLayout::HandleRowNameComboBoxGenarateWidget(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(*InItem))
		];
}

FText FSpineAnimSpecLayout::GetRowNameComboBoxContentText() const
{
	FString AnimNameValue;
	const FPropertyAccess::Result RowResult = AnimNamePropertyHandle->GetValue(AnimNameValue);
	if (RowResult == FPropertyAccess::Success)
	{
		return FText::FromString(*AnimNameValue);
	}
	else if (RowResult == FPropertyAccess::Fail)
	{
		return LOCTEXT("None", "None");
	}
	else
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}
}

void FSpineAnimSpecLayout::OnSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		CurrentSelectedItem = SelectedItem;
	//	FName NewValue = FName(**SelectedItem);
		AnimNamePropertyHandle->SetValue(*CurrentSelectedItem);

		// Close the combo
		RowNameComboButton->SetIsOpen(false);
	}
}

void FSpineAnimSpecLayout::OnFilterTextChanged(const FText& InFilterText)
{
	FString CurrentFilterText = InFilterText.ToString();

	FString AnimName;
	const FPropertyAccess::Result RowResult = AnimNamePropertyHandle->GetValue(AnimName);
	Row_AnimNames.Empty();

	/** Get the properties we wish to work with */
	const USpineSkeletonDataAsset* SpineSkeleton = nullptr;
	SpineSkeletonDataAssetPropertyHandle->GetValue((UObject*&)SpineSkeleton);

	TArray<FString> AllRowNames;
	if (SpineSkeleton != nullptr)
	{
		for (const FString& Item_AnimName : SpineSkeleton->Animations)
		{
			AllRowNames.Add(Item_AnimName);
		}

		// Sort the names alphabetically.
		AllRowNames.Sort();
	}

	for (const FString& RowString : AllRowNames)
	{
		if (CurrentFilterText == TEXT("") || RowString.Contains(CurrentFilterText))
		{
			TSharedRef<FString> RowNameItem = MakeShareable(new FString(RowString));
			Row_AnimNames.Add(RowNameItem);
		}
	}

	RowNameComboListView->RequestListRefresh();
}

void FSpineAnimSpecLayout::HandleMenuOpen()
{
	FSlateApplication::Get().SetKeyboardFocus(SearchBox);
}

void FSpineAnimSpecLayout::OnSearchForReferences()
{
	if (CurrentSelectedItem.IsValid() && !CurrentSelectedItem->IsEmpty() && SpineSkeletonDataAssetPropertyHandle.IsValid() && SpineSkeletonDataAssetPropertyHandle->IsValidHandle())
	{
		UObject* SourceSpineSkeletonAsset;
		SpineSkeletonDataAssetPropertyHandle->GetValue(SourceSpineSkeletonAsset);
		FName RowName(**CurrentSelectedItem);

		TArray<FAssetIdentifier> AssetIdentifiers;
		AssetIdentifiers.Add(FAssetIdentifier(SourceSpineSkeletonAsset, RowName));

		FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers);
	}
}

#undef LOCTEXT_NAMESPACE
