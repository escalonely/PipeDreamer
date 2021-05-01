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

#include <vector>
#include <map>
#include "TilePiece.h"


// ---- Class Definition ----

/**
 * Class which represents the grid where pipe tiles can be placed.
 */
class Board
{
public:
	/**
	 * Max number of bombs available per round, used to replace existing pipe tiles.
	 */
	static const int MAX_NUM_BOMBS;

	/**
	 * Score points required to restore a used up bomb.
	 */
	static const int SCORE_FOR_FREE_BOMB;

	/**
	 * Class constructor.
	 */
	Board(int numCols, int numRows);

	virtual ~Board();

	TilePiece::Type GetTileType(int col, int row) const;

	TilePiece* GetTile(int col, int row) const;

	void ReplaceTile(int col, int row, TilePiece::Type t);

	int GetNumRows() const;

	int GetNumCols() const;

	bool Pump(float amount);

	void Reset();

	void CreateRandomStart();

	TilePiece* FindNeighbor(TilePiece* p, Pipe::Direction d) /*const*/; // TODO: const deadlock

	/**
	 * Get the score gained so far in this round.
	 *
	 * @return	The score points on the board.
	 */
	int GetScoreValue() const;

	/**
	 * Get the number of bombs still available this round.
	 * Bombs are used to replace existing pipe tiles on the grid.
	 *
	 * @return	Number of bombs available.
	 */
	int GetNumBombs() const;

	/**
	 * Expend one of the available bombs, if available.
	 * The number of available bombs will be reduced by one.
	 *
	 * @return	True if at least one bomb was available prior to this call.
	 */
	bool PopBomb();

	/**
	 * Get the score gained so far, until one of the expended bombs is restored, in percent.
	 * Once this reaches 100, the number of available bombs will increase by one.
	 * After that, the score until the next restored bomb will be 0 again.
	 *
	 * @return	Score until next restored bomb, in percent.
	 */
	int GetPercentUntilFreeBomb();


private:
	TilePiece* m_oozingTile;

	int m_numCols;
	int m_numRows;

	typedef std::pair<int, int> Coord;
	std::map<Coord, TilePiece*> m_tileMap;

	int m_score;

	int m_numBombs;

	/**
	 * Score points until an used up bomb will be restored.
	 * Once SCORE_FOR_FREE_BOMB is reached, this counter is set back to 0.
	 */
	int m_scoreUntilFreeBomb;
};
