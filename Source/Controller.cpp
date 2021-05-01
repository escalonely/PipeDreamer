/*
===============================================================================

Copyright (C) 2021 Bernardo Escalona. All Rights Reserved.

  This file is part of Pipe Dreamer, found at:
  https://github.com/escalonely/PipeDreamer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

===============================================================================
*/


#include "Controller.h"


// ---- Helper types and constants ----

/**
 * Singleton initialization.
 */
Controller* Controller::m_singleton = nullptr;


// --- Controller class implementation ---

Controller::Controller()
{
	jassert(m_singleton == nullptr);
	m_singleton = this;

	// Initialize max score 
	InitApplicationProperties();
}

Controller::~Controller()
{
	m_singleton = nullptr;
}

Controller* Controller::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new Controller();
	}
	return m_singleton;
}

void Controller::InitApplicationProperties()
{
	// Locate or create the appropriate properties file.
	juce::PropertiesFile::Options options;
	options.applicationName = ProjectInfo::projectName;
	options.filenameSuffix = ".settings";
	options.osxLibrarySubFolder = "Application Support";
	options.folderName = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile(ProjectInfo::projectName).getFullPathName();
	options.storageFormat = juce::PropertiesFile::storeAsXML;
	m_appProperties.setStorageParameters(options);

	// If first time playing the game: initialize max score with a few
	// fun, dummy entries to entice the player to beat their score :)
	juce::StringPairArray allData = m_appProperties.getUserSettings()->getAllProperties();
	if (allData.size() == 0)
	{
		m_appProperties.getUserSettings()->setValue("31.10.1984@00:00:00;MADMX", 420);
		m_appProperties.getUserSettings()->setValue("01.07.1982@00:00:00;FLYNN", 1234);
		m_appProperties.getUserSettings()->setValue("18.04.2021@00:00:00;BERNS", 1750);

		allData = m_appProperties.getUserSettings()->getAllProperties();
	}

	// Generate list of cached score entries.
	juce::StringArray allkeys = allData.getAllKeys();
	for (int i = 0; i < allData.size(); i++)
	{
		juce::String name(allkeys[i]);
		int score = m_appProperties.getUserSettings()->getIntValue(allkeys[i]);

		m_scoreHash.push_back(std::pair<juce::String, int>(name, score));
	}

	// Sort the list by score, in descending order.
	std::sort(m_scoreHash.begin(), m_scoreHash.end(), HigherScoreThan);
}

std::vector<scoreEntry> Controller::GetAugmentedScoreHash(const scoreEntry& newEntry) const
{
	// Create a temporary copy of the scoreHash: we don't want to modify 
	// the actual hash yet. That can be done with SaveScoreEntry().
	std::vector<scoreEntry> listCopy(m_scoreHash);

	// Don't bother adding zero scores to the list.
	if (newEntry.second > 0)
	{
		listCopy.push_back(newEntry);
		std::sort(listCopy.begin(), listCopy.end(), HigherScoreThan);
	}

	return listCopy;
}

void Controller::SaveScoreEntry(const scoreEntry& newEntry)
{
	// Insert new entry into the cached list
	m_scoreHash.push_back(newEntry);
	std::sort(m_scoreHash.begin(), m_scoreHash.end(), HigherScoreThan);

	// ... and also into the properties file.
	m_appProperties.getUserSettings()->setValue(newEntry.first, newEntry.second);
}

bool Controller::HigherScoreThan(std::pair<juce::String, int> const &a, std::pair<juce::String, int> const &b)
{
	return a.second > b.second;
}