#pragma once

#include "SlateCore.h"
#include "Slate/SMeshWidget.h"
#include <spine/spine.h>
#include "SpineAtlasAsset.h"

class USpineWidget;

class SSpineWidget: public SMeshWidget {

public:
	SLATE_BEGIN_ARGS(SSpineWidget): _MeshData(nullptr) { }
	SLATE_ARGUMENT(USlateVectorArtData*, MeshData)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	void SetData(USpineWidget* Widget);
	FSlateBrush* brush;
protected:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void UpdateMesh(int32 LayerId, FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, spine::Skeleton* Skeleton);

	void Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int &Idx, TArray<FVector> &Vertices, TArray<int32> &Indices, TArray<FVector2D> &Uvs, TArray<FColor> &Colors, TArray<FVector> &Colors2, UMaterialInstanceDynamic* Material);

	USpineWidget* widget;	
	FRenderData renderData;
	FVector boundsMin;
	FVector boundsSize;
};
