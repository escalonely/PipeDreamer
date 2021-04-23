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
 * Main application.
 */
class PipeDreamerApplication : public juce::JUCEApplication
{
public:
	PipeDreamerApplication() 
	{
		InitApplicationProperties();
	}

	const juce::String getApplicationName() override { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override { return true; }

	void InitApplicationProperties();

	void initialise(const juce::String& commandLine) override
	{
		(void)commandLine;

		// TODO: think about useful commandline options
		// i.e.: starting level.

		m_mainWindow.reset(new MainWindow(getApplicationName()));
	}

	void shutdown() override
	{
		// This deletes the window unique_ptr.
		m_mainWindow = nullptr;
	}

	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted(const juce::String& commandLine) override
	{
		(void)commandLine;
	}

	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	class MainWindow : public juce::DocumentWindow
	{
	public:
		MainWindow(juce::String name);

		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

private:
	std::unique_ptr<MainWindow> m_mainWindow;
};

/**
 * This macro generates the main() routine that launches the app.
 */
START_JUCE_APPLICATION(PipeDreamerApplication)
