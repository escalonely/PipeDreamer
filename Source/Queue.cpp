/*
  ==============================================================================

    Queue.cpp
    Created: 26 Mar 2021 7:46:28pm
    Author:  bernardoe

  ==============================================================================
*/

#include "Queue.h"

Queue::Queue(int size)
{
	m_buff.reserve(size);

	// Seed randomizer
	std::random_device rd;
	m_mt = std::mt19937(rd());
	m_dist = std::uniform_int_distribution<int>(TilePiece::TYPE_VERTICAL, TilePiece::TYPE_CROSS);

	// Fill initial random queue
	for (int i = 0; i < size; ++i)
	{
		TilePiece::Type t(static_cast<TilePiece::Type>(m_dist(m_mt)));
		m_buff.push_back(new Pipe(t));
	}

	// This will move with every Pop.
	m_readPos = 0;
}

Queue::~Queue()
{
	for (int i = 0; i < m_buff.size(); ++i)
		delete m_buff[i];

}

int Queue::GetSize() const
{
	return static_cast<int>(m_buff.size());
}

Pipe* Queue::GetTile(int pos) const
{
	int idx = (m_readPos + pos) % m_buff.size();

	return m_buff[idx];
}

TilePiece::Type Queue::GetTileType(int pos) const
{
	int idx = (m_readPos + pos) % m_buff.size();

	return m_buff[idx]->GetType();
}

TilePiece::Type Queue::Pop()
{
	TilePiece::Type currentType(m_buff[m_readPos]->GetType());

	// Randomize type of m_buff[m_readPos]
	TilePiece::Type randomType(static_cast<TilePiece::Type>(m_dist(m_mt)));
	m_buff[m_readPos]->SetType(randomType);

	// Move read position
	m_readPos = (m_readPos + 1) % m_buff.size();

	return currentType;
}