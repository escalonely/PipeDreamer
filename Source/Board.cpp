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


#include "Board.h"
#include "Randomizer.h"



Board::Board(int numCols, int numRows)
	:	m_numCols(numCols), 
		m_numRows(numRows),
		m_score(0)
{
	// Fill the board with (empty) tiles.
	for (int i = 0; i < numCols; i++)
	{
		for (int j = 0; j < numRows; j++)
		{
			TilePiece* tile = TilePiece::CreateTile();

			m_tileMap[Coord(i, j)] = tile;
		}
	}

	CreateRandomStart();
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

TilePiece::Type Board::GetTileType(int col, int row) const
{
	return m_tileMap.at(Coord(col, row))->GetType();
}

TilePiece* Board::GetTile(int col, int row) const
{
	return m_tileMap.at(Coord(col, row));
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

void Board::Reset()
{
	m_score = 0;

	// Clear all tiles
	for (int i = 0; i < GetNumCols(); i++)
	{
		for (int j = 0; j < GetNumRows(); j++)
		{
			delete m_tileMap[Coord(i, j)];
			m_tileMap[Coord(i, j)] = new TilePiece();
		}
	}

	// Set random starting tile
	CreateRandomStart();
}

TilePiece* Board::FindNeighbor(TilePiece* tile, Pipe::Direction dir) /*const*/
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
		ret = m_tileMap[coord];
	}

	return ret;
}

int Board::GetScore() const
{
	return m_score;
}

void Board::CreateRandomStart()
{
	// Determine a random position on the board.
	Randomizer* rand = Randomizer::GetInstance();
	int startPosInt = rand->GetWithinRange(0, (m_numCols * m_numRows));
	Coord startCoord(startPosInt % m_numCols, static_cast<int>(startPosInt / m_numCols));

	TilePiece::Type starterType;
	bool search(true);
	while (search)
	{
		// Get a random starter tile
		starterType = static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_START_N, TilePiece::TYPE_START_W));

		// Keep trying with random starter tiles until we find one which is not facing the wall.
		search = (((startCoord.first <= 1) && (starterType == TilePiece::TYPE_START_W)) ||
			((startCoord.first >= (m_numCols - 2)) && (starterType == TilePiece::TYPE_START_E)) ||
			((startCoord.second <= 1) && (starterType == TilePiece::TYPE_START_N)) ||
			((startCoord.second >= (m_numRows - 2)) && (starterType == TilePiece::TYPE_START_S)));
	}

	// Set starter tile.
	ReplaceTile(startCoord.first, startCoord.second, starterType);
	m_oozingTile = m_tileMap[startCoord];
}
