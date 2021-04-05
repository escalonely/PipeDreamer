/*
  ==============================================================================

    Board.cpp
    Created: 26 Mar 2021 7:48:05pm
    Author:  bernardoe

  ==============================================================================
*/

#include "Board.h"
#include "Randomizer.h"
//#include <vector>



Board::Board(int numCols, int numRows)
	:	m_numCols(numCols), 
		m_numRows(numRows)
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
	int score(0);

	for (int i = 0; i < GetNumCols(); i++)
	{
		for (int j = 0; j < GetNumRows(); j++)
		{
			Coord c(i, j);

			switch (m_tileMap.at(c)->GetType())
			{
			case TilePiece::TYPE_VERTICAL:
			case TilePiece::TYPE_HORIZONTAL:
			case TilePiece::TYPE_NW_ELBOW:
			case TilePiece::TYPE_NE_ELBOW:
			case TilePiece::TYPE_SE_ELBOW:
			case TilePiece::TYPE_SW_ELBOW:
				{
					Pipe* pipe = dynamic_cast<Pipe*>(m_tileMap.at(c));
					if (pipe->IsFull())
						score += 100;
				}
				break;
			case TilePiece::TYPE_CROSS:
				{
					// Cross tile awards normal points if only one way is full.
					Cross* cross = dynamic_cast<Cross*>(m_tileMap.at(c));
					if ((cross->GetOozeLevel(Cross::WAY_HORIZONTAL) >= MAX_OOZE_LEVEL) && 
						(cross->GetOozeLevel(Cross::WAY_VERTICAL) < MAX_OOZE_LEVEL))
						score += 100;
					else if ((cross->GetOozeLevel(Cross::WAY_HORIZONTAL) < MAX_OOZE_LEVEL) &&
						(cross->GetOozeLevel(Cross::WAY_VERTICAL) >= MAX_OOZE_LEVEL))
						score += 100;

					// If both ways are full, give bonus points!
					else if ((cross->GetOozeLevel(Cross::WAY_HORIZONTAL) >= MAX_OOZE_LEVEL) &&
						(cross->GetOozeLevel(Cross::WAY_VERTICAL) >= MAX_OOZE_LEVEL))
						score += 300;
				}
				break;
			default:
				break;
			}
		}
	}

	return score;
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
