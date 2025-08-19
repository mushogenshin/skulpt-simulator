#include "SGraphVisualizer.h" 
#include "Widgets/SCanvas.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"

void SSGraphVisualizer::Construct(const FArguments& InArgs)
{
	GraphElementPtr = InArgs._GraphElement;

	ChildSlot
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

	UE_LOG(LogTemp, Warning, TEXT("SSGraphVisualizer::RebuildGraph: Rebuilding graph for element %s"), *GraphElementPtr->GetName());
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
	const FVector2D Center(160.0f, 160.0f);
	const int32 NumNodes = AdjacencyList.Num();
	const float AngleStep = (2.0f * PI) / NumNodes;

	for (int32 i = 0; i < NumNodes; ++i)
	{
		constexpr float Radius = 150.0f;
		const FGameplayTag& NodeTag = AdjacencyList[i][0];
		const float Angle = i * AngleStep;
		const FVector2D NodePosition = Center + FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));
		
		NodePositions.Add(NodeTag, NodePosition);

		// Add a text block for each node to the canvas
		NodeCanvas->AddSlot()
			.Position(NodePosition - FVector2D(60, 10)) // Center the text block
			.Size(FVector2D(120, 20))
			[
				SNew(STextBlock)
				.Text(FText::FromString(NodeTag.ToString()))
				.Justification(ETextJustify::Center)
			];
	}
}

int32 SSGraphVisualizer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// First, let the base class paint its children (the text blocks)
	const int32 NewLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

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
					1.0f  // Thickness
				);
			}
		}
	}

	return NewLayerId;
}