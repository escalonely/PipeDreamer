/*
  ==============================================================================

    ScoreWindow.h
    Created: 31 Mar 2021 9:55:50pm
    Author:  bernardoe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * TODO
 */
class ScoreWindow  :	public juce::Component,
						public juce::ChangeBroadcaster
{
public:
	enum Command
	{
		CMD_NONE = 0,
		CMD_QUIT,
		CMD_CONTINUE
	};

    ScoreWindow(int score);
    ~ScoreWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void mouseDown(const juce::MouseEvent& event) override;

	Command GetCommand() const;

private:
	int m_score;

	Command m_command;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScoreWindow)
};
