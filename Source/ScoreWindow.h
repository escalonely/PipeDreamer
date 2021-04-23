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


#pragma once

#include <JuceHeader.h>

/**
 * TODO
 */
class ScoreWindow  :	public juce::Component,
						public juce::ChangeBroadcaster
{
public:
	struct ScoreDetails
	{
		int score;
		int bonus;
		int carryover;
		int total;
		int level;
		bool advance;
	};

	enum Command
	{
		CMD_NONE = 0,
		CMD_QUIT,
		CMD_RESTART,
		CMD_CONTINUE
	};

    ScoreWindow(ScoreDetails details);
    ~ScoreWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void mouseDown(const juce::MouseEvent& event) override;

	Command GetCommand() const;

private:
	ScoreDetails m_details;

	Command m_command;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScoreWindow)
};
