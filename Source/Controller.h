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
	 * Represents exit state from the score window.
	 */
	enum Command
	{
		CMD_NONE = 0,
		CMD_QUIT,
		CMD_RESTART,
		CMD_CONTINUE
	};

	/**
	 * Game sound IDs.
	 */
	enum SoundID
	{
		SOUND_NONE = 0,
		SOUND_CLICK,
		SOUND_EXPLODE,
		SOUND_NOTIFY,
		SOUND_LEVEL_UP,
		SOUND_GAME_OVER,
		SOUND_MAX
	};

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

	/**
	 * Called by MainComponent at every framerate tick.
	 * TODO remove param score
	 */
	void Pump(int score);

	/**
	 * Called by MainComponent at the end of every round, when leaving the ScoreWindow.
	 * TODO 
	 */
	void Reset(Command cmd);

	/**
	 * Wakes up the AudioThread with a specific sound to be played once it is awake.
	 * 
	 * @param soundID	Sound to trigger in AudioThread::run().
	 */
	void QueueSound(SoundID soundID);

private:
	/**
	 * Class dedicated to playing sound effects on a dedicated thread.
	 */
	class AudioThread : public juce::Thread
	{
	public:
		/**
		 * Class constructor.
		 */
		AudioThread();

		/**
		 * Reimplemented from juce::Thread.
		 */
		void run() override;

		/**
		 * Wakes up the AudioThread with a specific sound to be played once it is awake.
		 *
		 * @param soundID	Sound to trigger in AudioThread::run().
		 */
		void QueueSound(SoundID soundID);

	private:
		/**
		 * Used to signal which sound is to be played once the AudioThread is running.
		 */
		std::atomic<SoundID> m_soundID = { SoundID::SOUND_NONE };
	};

	/**
	 * Object dedicated to playing sound effects on a dedicated thread.
	 */
	AudioThread m_audioThread;

	/**
	 * Configure and initialize the app properties file.
	 */
	void InitApplicationProperties();

	/**
	 * Configure and initialize the game's sound engine.
	 */
	void InitAudio();

	/**
	 * Free objects used for the game sounds.
	 */
	void ShutdownAudio();

	/**
	 * Triggers a sound to be played immediately by the m_audioMixer.
	 * This is called from within the AudioThread, after having been woken up via the MainComponent.
	 * 
	 * @param soundID	Sound to trigger in AudioThread::run().
	 */
	void PlaySound(SoundID soundID);

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

	/**
	 * Object that keeps tracks of a current audio device.
	 */
	juce::AudioDeviceManager m_audioDeviceManager;

	/**
	 * Object that streams audio from an audio source to an AudioIODevice.
	 */
	juce::AudioSourcePlayer m_audioPlayer;

	/**
	 * AudioSource object that mixes together the output of other AudioSources (see m_soundSources).
	 */
	juce::MixerAudioSource m_audioMixer;

	/**
	 * Shortcut type name for pointer to an AudioSource that obtains it's data from a file.
	 */
	typedef std::unique_ptr<juce::AudioFormatReaderSource> SoundSource;

	/**
	 * Map of sound sources. Keys are the soundIDs, values are AudioFormatReaderSource.
	 */
	std::map<SoundID, SoundSource> m_soundSources;

	/**
	 * TODO
	 */
	bool m_enoughScoreToLevelUp = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};
