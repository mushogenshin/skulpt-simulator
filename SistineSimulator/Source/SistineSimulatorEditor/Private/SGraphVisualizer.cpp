#include "SGraphVisualizer.h"
#include "Widgets/SCanvas.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"

const FMargin SSGraphVisualizer::Margin = FMargin(120.0f, 10.0f, 0.0f, 0.0f);

void SSGraphVisualizer::Construct(const FArguments& InArgs)
{
	GraphElementPtr = InArgs._GraphElement;

	ChildSlot
		.Padding(Margin)
		[
			SAssignNew(NodeCanvas, SCanvas)
		];

	RebuildGraph();
}

void SSGraphVisualizer::RebuildGraph()
{
	if (!GraphElementPtr.IsValid())
	{
		return;
	}

	NodeCanvas->ClearChildren();
	NodePositions.Empty();

	GraphElementPtr->UpdateAdjacencyList();
	const TArray<TArray<FGameplayTag>>& AdjacencyList = GraphElementPtr->GetAdjacencyList();
	if (AdjacencyList.IsEmpty())
	{
		NodeCanvas->AddSlot()
		          .Position(FVector2D(10, 10))
		          .Size(FVector2D(300, 20))
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Graph is empty or has not been processed.")))
		];
		return;
	}

	// --- Calculate Node Positions (arranged in a circle) ---
	const FVector2D Center(160.0f + Margin.Left, 160.0f + Margin.Top);
	const int32 NumNodes = AdjacencyList.Num();
	const float AngleStep = (2.0f * PI) / NumNodes;

	for (int32 i = 0; i < NumNodes; ++i)
	{
		constexpr float Radius = 150.0f;
		const FGameplayTag& NodeTag = AdjacencyList[i][0];
		const float Angle = i * AngleStep;
		const FVector2D NodePosition = Center + FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));

		NodePositions.Add(NodeTag, NodePosition);

		const FString NodeText = NodeTag.ToString();
		// A rough estimation for dynamic width. You may need to adjust the character width multiplier.
		constexpr float EstimatedCharWidth = 7.5f;
		constexpr float Padding = 10.0f;
		const float TextBlockWidth = (NodeText.Len() * EstimatedCharWidth) + Padding;
		constexpr float TextBlockHeight = 20.0f;

		// Add a text block for each node to the canvas
		NodeCanvas->AddSlot()
		          .Position(NodePosition - FVector2D(TextBlockWidth / 2.0f, TextBlockHeight) - FVector2D(
			          Margin.Left, Margin.Top))
		          // Center the text block then adjust its position to account for the margin, with some height offset for legibility
		          .Size(FVector2D(TextBlockWidth, TextBlockHeight))
		[
			SNew(STextBlock)
			.Text(FText::FromString(NodeText))
			.Justification(ETextJustify::Center)
		];
	}
}

int32 SSGraphVisualizer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                 const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                 const int32 LayerId, const FWidgetStyle& InWidgetStyle,
                                 const bool bParentEnabled) const
{
	// First, let the base class paint its children (the text blocks)
	const int32 NewLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
	                                                  InWidgetStyle, bParentEnabled);

	if (!GraphElementPtr.IsValid())
	{
		return NewLayerId;
	}

	const TArray<TArray<FGameplayTag>>& AdjacencyList = GraphElementPtr->GetAdjacencyList();
	TArray<FVector2D> LinePoints;

	// --- Draw Connection Lines ---
	for (const TArray<FGameplayTag>& NodeConnections : AdjacencyList)
	{
		if (NodeConnections.Num() < 2) continue;

		const FGameplayTag& StartNodeTag = NodeConnections[0];
		const FVector2D* StartPosPtr = NodePositions.Find(StartNodeTag);

		if (!StartPosPtr) continue;

		// Iterate neighbors (starting from index 1)
		for (int32 i = 1; i < NodeConnections.Num(); ++i)
		{
			const FGameplayTag& EndNodeTag = NodeConnections[i];
			const FVector2D* EndPosPtr = NodePositions.Find(EndNodeTag);

			if (!EndPosPtr) continue;

			// To avoid drawing lines twice (A->B and B->A), only draw if the start node's name is "less than" the end node's.
			if (StartNodeTag.ToString() < EndNodeTag.ToString())
			{
				LinePoints.Empty();
				LinePoints.Add(*StartPosPtr);
				LinePoints.Add(*EndPosPtr);

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					NewLayerId, // Draw on top of the base paint
					AllottedGeometry.ToPaintGeometry(),
					LinePoints,
					ESlateDrawEffect::None,
					FLinearColor::White,
					true, // bAntialias
					1.0f // Thickness
				);
			}
		}
	}

	return NewLayerId;
}
