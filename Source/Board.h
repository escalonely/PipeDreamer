/*
  ==============================================================================

    Board.h
    Created: 26 Mar 2021 7:48:05pm
    Author:  bernardoe

  ==============================================================================
*/

#pragma once

#include <vector>
#include <map>
#include "TilePiece.h"

class Board
{
public:
	Board(int numCols, int numRows);

	virtual ~Board();

	TilePiece::Type GetTileType(int col, int row) const;

	TilePiece* GetTile(int col, int row) const;

	void SetTileType(int col, int row, TilePiece::Type t);

	int GetNumRows() const;

	int GetNumCols() const;

	bool Pump(float amount);

	void Reset();

	TilePiece* FindNeighbor(TilePiece* p, Pipe::Direction d) /*const*/; // TODO: const deadlock

protected:
	//std::vector< std::vector<TilePiece*>> m_tiles;

	TilePiece* m_oozingTile;

	int m_numCols;
	int m_numRows;

	typedef std::pair<int, int> Coord;
	std::map<Coord, TilePiece*> m_tileMap;
};
