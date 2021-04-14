/*
===============================================================================

Copyright (C) 2021 Bernardo Escalona. All Rights Reserved.

  This file is part of the Pipe Dream clone found at:
  https://github.com/escalonely/PipeDream

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include <JuceHeader.h>
#include "ScoreWindow.h"
#include "MainComponent.h"

ScoreWindow::ScoreWindow(ScoreDetails details)
	:	m_details(details),
		m_command(CMD_NONE)
{
}

ScoreWindow::~ScoreWindow()
{
}

void ScoreWindow::paint (juce::Graphics& g)
{
	// Background color
    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

	// Frame
	g.setColour(juce::Colours::grey);
	juce::Rectangle<int> rect(	getLocalBounds().getX() + 5, getLocalBounds().getY() + 5, 
								getLocalBounds().getWidth() - 10, getLocalBounds().getHeight() - 10);
	g.drawRect(rect, 2);

	// Display score
	juce::String scoreStr = juce::String::formatted("Score: %d \nTotal Score: %d000", m_details.score, m_details.cmlScore);
	g.setFont(juce::Font("consolas", 32.0f, juce::Font::plain));
	rect = juce::Rectangle<int>(getLocalBounds().getX() + 5, getLocalBounds().getY() + 5, 
								getLocalBounds().getWidth() - 10, getLocalBounds().getHeight() / 2);
    g.drawFittedText(scoreStr, rect, juce::Justification::centred, true);
	g.drawRect(rect);

	// TODO: buttons
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

