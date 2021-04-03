/*

*/

#include <JuceHeader.h>
#include "ScoreWindow.h"
#include "MainComponent.h"

ScoreWindow::ScoreWindow(int score)
	:	m_score(score),
		m_command(CMD_NONE)
{
	//setSize(300, 200);
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ScoreWindow::~ScoreWindow()
{
}

void ScoreWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);   // draw an outline around the component

	juce::String scoreText;
	scoreText << "Score: ";
	scoreText << m_score;
	scoreText << "\nClick to continue.";

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(scoreText, getLocalBounds(), juce::Justification::centred, true);
}

void ScoreWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void ScoreWindow::mouseDown(const juce::MouseEvent& event)
{
	//MainComponent* mc = dynamic_cast<MainComponent*>(getParentComponent());
	//if (mc != nullptr)
	//{

	//}

	m_command = CMD_CONTINUE;

	sendChangeMessage();
}

ScoreWindow::Command ScoreWindow::GetCommand() const
{
	return m_command;
}

