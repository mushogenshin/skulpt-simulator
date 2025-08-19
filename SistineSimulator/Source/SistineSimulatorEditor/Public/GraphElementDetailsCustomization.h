#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"

class SSGraphVisualizer; 
class IDetailLayoutBuilder;

class FGraphElementDetailsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** Called when the 'Edges' property is changed */
	void OnEdgesChanged() const;

	/** Called when the refresh button is clicked */
	FReply OnRefreshButtonClicked() const;

	/** The visualizer widget instance */
	TSharedPtr<SSGraphVisualizer> GraphVisualizerWidget;
};
