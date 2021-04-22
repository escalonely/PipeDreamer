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

#include <vector>
#include <map>
#include "TilePiece.h"

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
	 * Score base required to restore a used up bomb.
	 */
	static const int SCORE_BASE_FOR_FREE_BOMB;

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
	 * @return	The score, in number of tiles.
	 */
	int GetScoreBase() const;

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

	int m_scoreBase;

	int m_numBombs;

	/**
	 * Score base until an used up bomb will be restored.
	 * Once SCORE_BASE_FOR_FREE_BOMB is reached, this counter is set back to 0.
	 */
	int m_scoreUntilFreeBomb;
};
