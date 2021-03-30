/*
  ==============================================================================

    Board.cpp
    Created: 26 Mar 2021 7:48:05pm
    Author:  bernardoe

  ==============================================================================
*/

#include "Board.h"


Board::Board(int numCols, int numRows)
	:	m_numCols(numCols), 
		m_numRows(numRows)
{
	// Fill the board with (empty) tiles.
	for (int i = 0; i < numCols; i++)
	{
		for (int j = 0; j < numRows; j++)
		{
			TilePiece* tile;
			Coord coord(i, j);

			// TODO: Set random starting tile position
			if (i == 0 && j == 0)
			{
				m_oozingTile = new Pipe(TilePiece::TYPE_START_E);
				tile = m_oozingTile;
			}

			// The rest are just empty tiles.
			else
				tile = new TilePiece();
			
			m_tileMap[coord] = tile;
		}
	}
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

void Board::SetTileType(int col, int row, TilePiece::Type t)
{
	Coord c(col, row);
	if (m_tileMap[c]->GetType() != t)
	{
		delete m_tileMap[c];
		m_tileMap[c] = new Pipe(t);
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
				(neighbor->IsEmpty()))					// Neighbor not full of ooze yet
			{
				// Set the ooze entry point.
				if (neighbor->SetFlowEntry(inFlowDir))
				{
					// Now the ooze is flowing into the neighbor
					m_oozingTile = neighbor;

					// Ooze saved.
					ret = true;
				}
			}
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

	// TODO: Set random starting tile
	m_oozingTile = m_tileMap[Coord(0, 0)];
	m_oozingTile->SetType(TilePiece::TYPE_START_E);
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
			coord.second += 1;
			break;
		case Pipe::DIR_S:
			coord.second -= 1;
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