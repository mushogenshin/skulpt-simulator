#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ArtGraph/ArtGraph.h" // Required for UGraphElement

class SSGraphVisualizer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSGraphVisualizer) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UGraphElement>, GraphElement)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Rebuilds the visual representation from the graph data
	void RebuildGraph();

protected:
	// Override OnPaint to draw the connection lines
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:

	// The UGraphElement data asset to visualize
	TWeakObjectPtr<UGraphElement> GraphElementPtr;

	// A map to store the calculated screen position for each graph node (by tag)
	TMap<FGameplayTag, FVector2D> NodePositions;

	// The canvas panel used to position the node widgets
	TSharedPtr<class SCanvas> NodeCanvas;

	// Constant for margin
	static const FMargin Margin;
};
