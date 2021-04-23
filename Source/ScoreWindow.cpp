/*
===============================================================================

Copyright (C) 2021 Bernardo Escalona. All Rights Reserved.

  This file is part of the Pipe Dream clone found at:
  https://github.com/escalonely/PipeDreamer

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
	int width = getLocalBounds().getWidth();
	int buttonHeight = getLocalBounds().getHeight() / 5;

	juce::String messageText("You Lose! :(\n");
	juce::String buttonText("Restart Game");
	if (m_details.advance)
	{
		messageText = juce::String("You Leveled Up!\n");
		buttonText = juce::String("Continue to Next Level");
	}

	// Background color
	g.fillAll(juce::Colour(27, 27, 27));

	// Frame
	g.setColour(juce::Colours::grey);
	juce::Rectangle<int> rect(5, 5, width - 10, getLocalBounds().getHeight() - 10);
	g.drawRect(rect, 2);

	// Display score
	messageText << juce::String::formatted("Score: %d\nLevel Bonus: %d\nCarry Over: %d\nTotal: %d", 
											m_details.score, m_details.bonus, m_details.carryover, m_details.total);
	g.setFont(juce::Font("consolas", 32.0f, juce::Font::plain));
	rect = juce::Rectangle<int>(5, 5, width - 10, buttonHeight * 3);
    g.drawFittedText(messageText, rect, juce::Justification::centred, true);

	// Continue button
	rect = juce::Rectangle<int>(10, (buttonHeight * 3) + 5, width - 20, buttonHeight - 10);
	g.drawFittedText(buttonText, rect, juce::Justification::centred, true);
	g.drawRect(rect);

	// Quit button
	rect = juce::Rectangle<int>(10, (buttonHeight * 4) - 0, width - 20, buttonHeight - 10);
    g.drawFittedText("Quit", rect, juce::Justification::centred, true);
	g.drawRect(rect);
}

void ScoreWindow::resized()
{
}

void ScoreWindow::mouseDown(const juce::MouseEvent& event)
{
	int width = getLocalBounds().getWidth();
	int buttonHeight = getLocalBounds().getHeight() / 5;

	juce::Point<int> clickPos = event.getMouseDownPosition();
	juce::Rectangle<int> continueRect(10, (buttonHeight * 3) + 5, width - 20, buttonHeight - 10);
	juce::Rectangle<int> quitRect(10, (buttonHeight * 4) - 0, width - 20, buttonHeight - 10);

	if (continueRect.contains(clickPos))
	{
		if (m_details.advance)
			m_command = CMD_CONTINUE;
		else
			m_command = CMD_RESTART;
	}

	else if (quitRect.contains(clickPos))
		m_command = CMD_QUIT;

	// If a button was clicked, send a callback.
	if (m_command != CMD_NONE)
		sendChangeMessage();
}

ScoreWindow::Command ScoreWindow::GetCommand() const
{
	return m_command;
}

