
#pragma once

#include <JuceHeader.h>

class Board;
class Queue;
class TilePiece;
class ScoreWindow;


/**
 * TODO
 */
class MainComponent  :	public juce::Component,
						public juce::Timer,
						public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void timerCallback() override;
	void mouseDown(const juce::MouseEvent& event) override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	juce::Colour GetCurrentTileColor() const;
	float GetCurrentOozePerPump() const;
	void DrawTile(TilePiece* tile, juce::Point<int> p, juce::Graphics& g);
	void DrawOoze(TilePiece* tile, juce::Point<int> p, juce::Graphics& g);

private:
	Board* m_board;

	Queue* m_queue;

	ScoreWindow* m_scoreWindow;

	/**
	 * Level starts at 1, and as it increases, the amount of ooze pumped per frame also increases.
	 */
	int m_difficultyLevel;

	int m_cummulativeScore;

	int m_blockInteraction;

	juce::CriticalSection m_lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
