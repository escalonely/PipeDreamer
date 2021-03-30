
#pragma once

#include <JuceHeader.h>
#include "Board.h"
#include "Queue.h"


class MainComponent  : public	juce::Component,
								juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void timerCallback() override;
	void mouseDown(const juce::MouseEvent& event) override;

	static juce::Colour GetTileTypeTestColor(TilePiece::Type t);
	void DrawTile(const TilePiece* tile, juce::Point<int> p, juce::Graphics& g);

private:
	Board* m_board;

	Queue* m_queue;

	float m_oozeAmount;

	juce::CriticalSection m_lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
