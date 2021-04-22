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


#include "Main.h"
#include "MainComponent.h"

void PipeDreamApplication::InitApplicationProperties()
{
	// TODO
	juce::ApplicationProperties props;

	juce::PropertiesFile::Options options;
	options.applicationName = ProjectInfo::projectName;
	options.filenameSuffix = ".settings";
	options.osxLibrarySubFolder = "Application Support";
	options.folderName = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile(ProjectInfo::projectName).getFullPathName();
	options.storageFormat = juce::PropertiesFile::storeAsXML;
	props.setStorageParameters(options);

	//props.getUserSettings()->setValue("Flynn", 12345);
	//props.getUserSettings()->setValue("Bob", 2);
	//props.getUserSettings()->setValue("Madmax", 32132);

	juce::StringPairArray allData = props.getUserSettings()->getAllProperties();
	juce::StringArray allkeys = allData.getAllKeys();

	std::vector<std::pair<juce::String, int>> scorePairVector;
	for (int i = 0; i < allData.size(); i++)
	{
		juce::String name(allkeys[i]);
		int score = props.getUserSettings()->getIntValue(allkeys[i]);

		scorePairVector.push_back(std::pair<juce::String, int>(name, score));
	}

	// https://stackoverflow.com/questions/19842035/how-can-i-sort-a-stdmap-first-by-value-then-by-key
	auto cmp = [](std::pair<juce::String, int> const & a, std::pair<juce::String, int> const & b)
	{
		return a.second > b.second;
	};
	std::sort(scorePairVector.begin(), scorePairVector.end(), cmp);
}

PipeDreamApplication::MainWindow::MainWindow(juce::String name)
	: DocumentWindow(name,
		juce::Desktop::getInstance().getDefaultLookAndFeel()
		.findColour(juce::ResizableWindow::backgroundColourId),
		DocumentWindow::allButtons)
{
	setUsingNativeTitleBar(true);
	setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
	setFullScreen(true);
#else
	setResizable(false, false);
	centreWithSize(getWidth(), getHeight());
#endif

	setVisible(true);
}
