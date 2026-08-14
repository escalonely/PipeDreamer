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


#include "Board.h"
#include "Randomizer.h"


// ---- Helper types and constants ----

const int Board::MAX_NUM_BOMBS(5);
const int Board::SCORE_FOR_FREE_BOMB(50);


// ---- Class Implementation ----

Board::Board(int numCols, int numRows)
	:	m_numCols(numCols), 
		m_numRows(numRows)
{
	Reset();
}

Board::~Board()
{
	// Clear all tiles
	for (int i = 0; i < GetNumCols(); i++)
	{
		for (int j = 0; j < GetNumRows(); j++)
		{
			delete m_tileMap[Coord(i, j)];
		}
	}
}

void Board::Reset()
{
	m_score = 0;
	m_scoreUntilFreeBomb = 0;
	m_numBombs = MAX_NUM_BOMBS;

	// Fill the board with (empty) tiles.
	for (int i = 0; i < GetNumCols(); i++)
	{
		for (int j = 0; j < GetNumRows(); j++)
		{
			if (m_tileMap.count(Coord(i, j)) > 0)
				delete m_tileMap[Coord(i, j)];

			m_tileMap[Coord(i, j)] = TilePiece::CreateTile();
		}
	}

	// Set random starting tile
	CreateRandomStartAndStop();
}

TilePiece::Type Board::GetTileType(int col, int row) const
{
	return m_tileMap.at(Coord(col, row))->GetType();
}

TilePiece* Board::GetTile(int col, int row) const
{
	return m_tileMap.at(Coord(col, row));
}

TilePiece* Board::GetOozingTile() const
{
	return m_oozingTile;
}

void Board::ReplaceTile(int col, int row, TilePiece::Type t)
{
	Coord c(col, row);

	// If the old tile wasn't empty (was a pipe), we mark the replacement tile
	// so that an explosion graphic can be drawn over it.
	bool explode = (m_tileMap[c]->GetType() != TilePiece::TYPE_NONE);

	delete m_tileMap[c];
	m_tileMap[c] = TilePiece::CreateTile(t);

	if (explode)
	{
		Pipe* pipe = dynamic_cast<Pipe*>(m_tileMap[c]);
		if (pipe != nullptr)
			pipe->Explode();
	}
}

int Board::GetNumCols() const
{
	return m_numCols;
}

int Board::GetNumRows() const
{
	return m_numRows;
}

bool Board::Pump(float amount)
{
	bool ret(false);

	Pipe* oozingPipe = dynamic_cast<Pipe*>(m_oozingTile);
	if (oozingPipe != nullptr)
	{
		oozingPipe->Pump(amount);
		if (oozingPipe->IsFull())
		{
			m_score += oozingPipe->GetScoreValue();

			if (oozingPipe->IsStop())
			{
				// Ooze flowing into a Stop tile. You won!
				// TODO: implement victory
				return false;
			}

			// Once this score reaches SCORE_FOR_FREE_BOMB, the number of available 
			// bombs will increase by one. After that, the score until the next restored
			// bomb will be 0 again.
			m_scoreUntilFreeBomb += oozingPipe->GetScoreValue();
			if (m_scoreUntilFreeBomb >= SCORE_FOR_FREE_BOMB)
			{
				if (m_numBombs < MAX_NUM_BOMBS)
					m_numBombs++;

				m_scoreUntilFreeBomb = 0;
			}

			Pipe::Direction outFlowDir = oozingPipe->GetFlowDirection();
			Pipe::Direction inFlowDir = Pipe::GetOppositeDirection(outFlowDir);

			// Returns null if ooze flowing out of bounds, 
			// and cast will fail if ooze is spilling on empty tile.
			Pipe* neighbor = dynamic_cast<Pipe*>(FindNeighbor(oozingPipe, outFlowDir));
			if ((neighbor != nullptr) &&
				(neighbor->HasOpening(inFlowDir)) &&	// Neighbor has an opening in the right spot.
				(neighbor->SetFlowEntry(inFlowDir)))	// Able to set the ooze entry point.
			{
				// Now the ooze is flowing into the neighbor
				m_oozingTile = neighbor;

				// Ooze saved.
				ret = true;
			}

			// Spill!
			else
				ret = false;
		}
		else
		{
			// This pipe not full yet -> nothing more to do.
			ret = true;
		}
	}

	return ret;
}

TilePiece* Board::FindNeighbor(TilePiece* tile, Pipe::Direction dir) const
{
	TilePiece* ret(nullptr);

	// Get the coordinates of the passed pipe.
	Coord coord;
	for (auto it = m_tileMap.begin(); it != m_tileMap.end(); ++it)
	{
		if (it->second == tile)
		{
			coord = it->first;
			break;
		}
	}

	// Advance coord in the desired direction
	switch (dir)
	{
		case Pipe::DIR_N:
			coord.second -= 1;
			break;
		case Pipe::DIR_S:
			coord.second += 1;
			break;
		case Pipe::DIR_E:
			coord.first += 1;
			break;
		case Pipe::DIR_W:
			coord.first -= 1;
			break;
		default:
			break;
	}

	// Check bounds
	if ((coord.first >= 0) && (coord.first < m_numCols) &&
		(coord.second >= 0) && (coord.second < m_numRows))
	{
		ret = m_tileMap.at(coord);
	}

	return ret;
}

int Board::GetScoreValue() const
{
	return m_score;
}

void Board::CreateRandomStartAndStop()
{
	// Determine a random position on the board.
	Randomizer* rand = Randomizer::GetInstance();

	Coord startCoord, stopCoord;
	TilePiece::Type startTileType(TilePiece::TYPE_NONE);
	TilePiece::Type stopTileType(TilePiece::TYPE_NONE);

	bool search(true);
	while (search)
	{
		// Get a random start tile
		int startPosInt = rand->GetWithinRange(0, (m_numCols * m_numRows) - 1);
		startCoord = Coord(startPosInt % m_numCols, static_cast<int>(startPosInt / m_numCols));
		startTileType = static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_START_N, TilePiece::TYPE_START_W));

		// Get a random stop tile
		int stopPosInt = rand->GetWithinRange(0, (m_numCols * m_numRows) - 1);
		stopCoord = Coord(stopPosInt % m_numCols, static_cast<int>(stopPosInt / m_numCols));
		stopTileType = static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_STOP_N, TilePiece::TYPE_STOP_W));

		// True if the start tile would ooze directly into the wall.
		bool startFacesWall = (((startCoord.first <= 1) && (startTileType == TilePiece::TYPE_START_W)) ||
			((startCoord.first >= (m_numCols - 2)) && (startTileType == TilePiece::TYPE_START_E)) ||
			((startCoord.second <= 1) && (startTileType == TilePiece::TYPE_START_N)) ||
			((startCoord.second >= (m_numRows - 2)) && (startTileType == TilePiece::TYPE_START_S)));

		// True if the stop tile is directly facing the wall.
		bool stopFacesWall = (((stopCoord.first <= 1) && (stopTileType == TilePiece::TYPE_STOP_W)) ||
			((stopCoord.first >= (m_numCols - 2)) && (stopTileType == TilePiece::TYPE_STOP_E)) ||
			((stopCoord.second <= 1) && (stopTileType == TilePiece::TYPE_STOP_N)) ||
			((stopCoord.second >= (m_numRows - 2)) && (stopTileType == TilePiece::TYPE_STOP_S)));

		// True if start and stop are neighboring tiles facing each other, so the ooze would
		// flow directly out of the start tile into the stop tile (instant win :D).
		int colDelta = stopCoord.first - startCoord.first;
		int rowDelta = stopCoord.second - startCoord.second;
		bool startFacesStop =
			((startTileType == TilePiece::TYPE_START_N) && (colDelta == 0) && (rowDelta == -1) && (stopTileType == TilePiece::TYPE_STOP_S)) ||
			((startTileType == TilePiece::TYPE_START_S) && (colDelta == 0) && (rowDelta == 1) && (stopTileType == TilePiece::TYPE_STOP_N)) ||
			((startTileType == TilePiece::TYPE_START_E) && (colDelta == 1) && (rowDelta == 0) && (stopTileType == TilePiece::TYPE_STOP_W)) ||
			((startTileType == TilePiece::TYPE_START_W) && (colDelta == -1) && (rowDelta == 0) && (stopTileType == TilePiece::TYPE_STOP_E));

		// Keep trying with random start/stop tiles until neither faces the wall and they don't face each other.
		search = (startFacesWall || stopFacesWall || startFacesStop);
	}

	// Set start and stop tiles.
	ReplaceTile(startCoord.first, startCoord.second, startTileType);
	ReplaceTile(stopCoord.first, stopCoord.second, stopTileType);

	m_oozingTile = m_tileMap[startCoord];
}

int Board::GetNumBombs() const
{
	return m_numBombs;
}

bool Board::PopBomb()
{
	if (m_numBombs > 0)
	{
		m_numBombs--;
		return true;
	}

	return false;
}

int Board::GetPercentUntilFreeBomb()
{
	return static_cast<int>((m_scoreUntilFreeBomb * 100) / SCORE_FOR_FREE_BOMB);
}