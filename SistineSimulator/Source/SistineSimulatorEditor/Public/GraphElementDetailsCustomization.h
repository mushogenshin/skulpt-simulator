#pragma once

#include "IDetailCustomization.h"

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

	/** The visualizer widget instance */
	TSharedPtr<SSGraphVisualizer> GraphVisualizerWidget;
};

