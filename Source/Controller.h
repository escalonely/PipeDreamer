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

#pragma once

#include <JuceHeader.h>


// ---- Helper types and constants ----

typedef std::pair<juce::String, int> scoreEntry;


// ---- Class definition ----

/**
 * Controller class manages the game's various states and the player's score.
 */
class Controller
{
public:
	/**
	 * Class constructor.
	 */
	Controller();

	/**
	 * Class destructor.
	 */
	virtual ~Controller();

	/**
	 * Returns the one and only instance of Controller. If it doesn't exist yet, it is created.
	 *
	 * @return The Controller singleton object.
	 */
	static Controller* GetInstance();

	 /**
	  * Gets a list of score entries (date/name and score pairs) extracted from the app properties file.
	  * If not zero, the tempEntry will be inserted at the right position within the returned list. 
	  * It will not be written to the app properties file, however.
	  * 
	  * @param tempEntry	Placeholder entry. If non-zero, will be present within the returned list.
	  * @return	List of all score entries, sorted in descending order by score. 
	  */
	std::vector<scoreEntry> GetAugmentedScoreHash(const scoreEntry& tempEntry) const;

	/**
	 * Save a scoreEntry in the ApplicationProperties.
	 *
	 * @param newEntry	Player name / score pair which is to be added to the ApplicationProperties.
	 */
	void SaveScoreEntry(const scoreEntry& newEntry);

	/**
	 * Sorting function for score entries.
	 * 
	 * @param a	First entry.
	 * @param b	Second entry.
	 * @return True if a's score is higher than b's. 
	 */
	static bool HigherScoreThan(std::pair<juce::String, int> const &a, std::pair<juce::String, int> const &b);

private:
	/**
	 * Configure and initialize the app properties file.
	 */
	void InitApplicationProperties();

	/**
	 * The one and only instance of Controller.
	 */
	static Controller*	m_singleton;

	/**
	 * App properties file used to store player scores.
	 */
	juce::ApplicationProperties m_appProperties;

	/**
	 * Cached list of score entries, generated from the app properties file in InitApplicationProperties().
	 */
	std::vector<scoreEntry> m_scoreHash;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};
