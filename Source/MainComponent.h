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

#pragma once

#include <JuceHeader.h>

class Board;
class Queue;
class TilePiece;
class Randomizer;
class ScoreWindow;


/**
 * TODO
 */
class MainComponent  :	public juce::Component,
						public juce::Timer,
						public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

	void timerCallback() override;
	void mouseDown(const juce::MouseEvent& event) override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	juce::Colour GetCurrentTileColor() const;
	float GetCurrentOozePerPump() const;
	int GetCurrentCountdown() const;

	/**
	 * Draw a tile piece itself, of any kind.
	 *
	 * @param tile		The tile piece to draw.
	 * @param origin	The point on the MainComponent window where the top-left corner 
	 *					of the tile being drawn will be located.
	 * @param g			The graphics context used for drawing.
	 */
	void DrawTile(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g);

	/**
	 * Draw the ooze flowing through a pipe tile.
	 *
	 * @param tile		The tile piece to draw the ooze over.
	 * @param origin	The point on the MainComponent window where the top-left corner
	 *					of the tile being drawn will be located.
	 * @param g			The graphics context used for drawing.
	 */
	void DrawOoze(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g);

	/**
	 * The DrawTile() and DrawOoze() methods only worry about the first of the "ways" of TYPE_CROSS pipes, 
	 * either the vertical or the horizontal way. This method should be used afterwards to draw the 
	 * second, foreground way of TYPE_CROSS pipes. This method will not affect tiles of any other type.
	 *
	 * @param tile		The TYPE_CROSS tile piece to draw.
	 * @param origin	The point on the MainComponent window where the top-left corner
	 *					of the tile being drawn will be located.
	 * @param g			The graphics context used for drawing.
	 */
	void DrawCrossSecondWay(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g);

	/**
	 * Draw a tile piece's decoration elements, such as frame and explosion graphics.
	 *
	 * @param tile		The tile piece to draw the decorations for.
	 * @param origin	The point on the MainComponent window where the top-left corner
	 *					of the tile being drawn will be located.
	 * @param g			The graphics context used for drawing.
	 */
	void DrawTileDecoration(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g);

	/**
	 * Draw the bombs next to the board, used to replace pipe tiles.
	 *
	 * @param origin	The point on the MainComponent window where the first bomb shall be drawn.
	 * @param g			The graphics context used for drawing.
	 */
	void DrawBombs(juce::Point<int> origin, juce::Graphics& g);

private:
	Board* m_board;

	Queue* m_queue;

	ScoreWindow* m_scoreWindow;

	Randomizer* m_randomizer;

	int m_countDown;

	/**
	 * Level starts at 1, and as it increases, the amount of ooze pumped per frame also increases.
	 */
	int m_difficultyLevel;

	int m_cumulativeScore;

	int m_blockInteraction;

	juce::CriticalSection m_lock;

	/**
	 * Points gained in one round, necessary to advance to the next difficulty level.
	 */
	static const int MIN_SCORE_TO_ADVANCE;

	/**
	 * The Board and the TilePieces only know about their base score value. To obtain
	 * the actual player score, this is multiplied by the score multiplier.
	 */
	static const int SCORE_MULTIPLIER;

	/**
	 * Hyperlink to the download URL.
	 */
	std::unique_ptr<juce::HyperlinkButton> m_hyperlink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
