#include "GraphElementDetailsCustomization.h"
#include "SGraphVisualizer.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "ArtGraph/ArtGraph.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"

TSharedRef<IDetailCustomization> FGraphElementDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FGraphElementDetailsCustomization);
}

void FGraphElementDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Get the UGraphElement being inspected
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	TWeakObjectPtr<UGraphElement> GraphElement = nullptr;
	if (Objects.Num() > 0)
	{
		GraphElement = Cast<UGraphElement>(Objects[0].Get());
	}

	// Get a handle to the 'Edges' property and bind to its change delegates
	if (const TSharedPtr<IPropertyHandle> EdgesPropertyHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UGraphElement, Edges)); EdgesPropertyHandle.IsValid())
	{
		const FSimpleDelegate OnEdgesChangedDelegate = FSimpleDelegate::CreateSP(
			this, &FGraphElementDetailsCustomization::OnEdgesChanged);

		// This delegate fires when a property *within* an array element changes.
		EdgesPropertyHandle->SetOnChildPropertyValueChanged(OnEdgesChangedDelegate);

		// This delegate fires when elements are added or removed from the array.
		if (const TSharedPtr<IPropertyHandleArray> ArrayHandle = EdgesPropertyHandle->AsArray())
		{
			ArrayHandle->SetOnNumElementsChanged(OnEdgesChangedDelegate);
		}
	}

	// Add a new category for the visual graph
	IDetailCategoryBuilder& GraphCategory = DetailBuilder.EditCategory("Graph Visualization", FText::GetEmpty(),
	                                                                     ECategoryPriority::Important);

	// Add a refresh button
	GraphCategory.AddCustomRow(FText::FromString(TEXT("Manual Refresh")))
	.ValueContent()
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("Refresh Visualization")))
		.OnClicked(this, &FGraphElementDetailsCustomization::OnRefreshButtonClicked)
	];

	// Add the custom SGraphVisualizer widget
	GraphCategory.AddCustomRow(FText::FromString(TEXT("Graph Preview")))
	             .WholeRowContent()
	[
		SNew(SBox)
		.HeightOverride(350) // Give the visualizer a fixed height
		[
			SAssignNew(GraphVisualizerWidget, SSGraphVisualizer)
			.GraphElement(GraphElement)
		]
	];
}

FReply FGraphElementDetailsCustomization::OnRefreshButtonClicked() const
{
	if (GraphVisualizerWidget.IsValid())
	{
		GraphVisualizerWidget->RebuildGraph();
	}
	return FReply::Handled();
}

void FGraphElementDetailsCustomization::OnEdgesChanged() const
{
	if (GraphVisualizerWidget.IsValid())
	{
		GraphVisualizerWidget->RebuildGraph();
	}
}
