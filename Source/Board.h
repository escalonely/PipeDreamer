/*
===============================================================================

Copyright (C) 2026 Bernardo Escalona. All Rights Reserved.

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
	 * Class constructor.
	 */
	Board(int numCols, int numRows);

	/**
	 * Class destructor. Deletes all entries in m_tileMap.
	 */
	virtual ~Board();

	/**
	 * Get the type of the tile at the given coordinates.
	 * 
	 * @param col	Column of desired tile.
	 * @param row	Row of desired tile.
	 * @return The type of the desired tile. Can be TYPE_NONE if the tile is empty.
	 */
	TilePiece::Type GetTileType(int col, int row) const;

	/**
	 * Get the tile at the desired location.
	 */
	TilePiece* GetTile(int col, int row) const;

	/**
	 * Replace the tile at the given coordinates with a new tile of the given type.
	 * The old tile will be deleted.
	 * 
	 * @param col	Column of tile to replace.
	 * @param row	Row of tile to replace.
	 * @param t		Type of new tile to place at the given coordinates.
	 */
	void ReplaceTile(int col, int row, TilePiece::Type t);

	/**
	 * Get the number of rows in the board.
	 */
	int GetNumRows() const;

	/**
	 * Get the number of columns in the board.
	 */
	int GetNumCols() const;

	/**
	 * Pump ooze into the pipes on the board. This method is called at every tick.
	 * Whichever pipe is m_oozingTile currently, will have its Pump() method called,
	 * and thus the amount of ooze inside it increased.
	 * 
	 * @param amount	Amount of ooze to insert. The higher the level, 
	 *					the more ooze amount will be pumped every tick.
	 * @return	True if the ooze is still contained within m_oozingTile or it's neighbor.
	 *			False if the ooze has now spilled, or reached the Stop tile (see IsVictory()).
	 */
	bool Pump(float amount);

	/**
	 * True if the ooze has reached the Stop tile, i.e. the player has won this round.
	 * Only meaningful once Pump() has returned false.
	 */
	bool IsVictory() const;

	/**
	 * Resets score, clears all tiles, and repositions starting tile
	 * to a random position.
	 */
	void Reset();

	/**
	 * Create a random Start tile on the board.
	 */
	void CreateRandomStart();

	/**
	 * Find the neighbor of the given tile in the given direction.
	 * 
	 * @param p		Tile whose neighbor is to be found.
	 * @param d		Direction in which to look for a neighbor.
	 * @return	The neighboring tile, or nullptr if no neighbor exists in that direction.
	 */
	TilePiece* FindNeighbor(TilePiece* p, Pipe::Direction d) const;

	/**
	 * Get the score gained so far in this round.
	 *
	 * @return	The score points on the board.
	 */
	int GetScoreValue() const;

	/**
	 * Get the pipe on the board, in which the ooze level is currently increasing.
	 */
	TilePiece* GetOozingTile() const;

private:
	/**
	 * Pipe piece in which the ooze level is currently increasing.
	 */
	TilePiece* m_oozingTile;

	/**
	 * True if the ooze has reached the Stop tile. Reset back to false at the start of every round.
	 */
	bool m_victory;

	int m_numCols;
	int m_numRows;

	typedef std::pair<int, int> Coord;
	std::map<Coord, TilePiece*> m_tileMap; /**< Map of coordinates and tile pieces. */

	int m_score;
};
