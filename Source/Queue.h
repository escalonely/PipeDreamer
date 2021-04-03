/*
  ==============================================================================

    Queue.h
    Created: 26 Mar 2021 7:46:28pm
    Author:  bernardoe

  ==============================================================================
*/

#pragma once

#include "TilePiece.h"
#include <vector>
#include <random>

class Queue
{
public:
	Queue(int size);

	~Queue();

	void Reset();

	int GetSize() const;

	Pipe* GetTile(int pos) const;

	TilePiece::Type GetTileType(int pos) const;

	TilePiece::Type Pop();

protected:
	std::vector<Pipe*> m_buff;
	int m_readPos;

	std::mt19937 m_mt;
	std::uniform_int_distribution<int> m_dist;
};