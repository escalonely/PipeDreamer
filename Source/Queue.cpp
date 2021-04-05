/*
  ==============================================================================

    Queue.cpp
    Created: 26 Mar 2021 7:46:28pm
    Author:  bernardoe

  ==============================================================================
*/

#include "Queue.h"
#include "Randomizer.h"

Queue::Queue(int size)
{
	m_buff.reserve(size);
	Randomizer* rand = Randomizer::GetInstance();

	// Fill initial queue with random tiles, of any type between VERTICAL and CROSS.
	for (int i = 0; i < size; ++i)
	{
		TilePiece::Type t(static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_VERTICAL, TilePiece::TYPE_CROSS)));
		m_buff.push_back(dynamic_cast<Pipe*>(TilePiece::CreateTile(t)));
	}

	// This will move with every Pop.
	m_readPos = 0;
}

Queue::~Queue()
{
	for (int i = 0; i < m_buff.size(); ++i)
		delete m_buff[i];

}

void Queue::Reset()
{
	Randomizer* rand = Randomizer::GetInstance();
	for (int i = 0; i < m_buff.size(); ++i)
	{
		delete m_buff[i];

		TilePiece::Type t(static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_VERTICAL, TilePiece::TYPE_CROSS)));
		m_buff[i] = dynamic_cast<Pipe*>(TilePiece::CreateTile(t));
	}

	m_readPos = 0;
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
	Randomizer* rand = Randomizer::GetInstance();
	TilePiece::Type t(static_cast<TilePiece::Type>(rand->GetWithinRange(TilePiece::TYPE_VERTICAL, TilePiece::TYPE_CROSS)));

	delete m_buff[m_readPos];
	m_buff[m_readPos] = dynamic_cast<Pipe*>(TilePiece::CreateTile(t));

	// Move read position
	m_readPos = (m_readPos + 1) % m_buff.size();

	return currentType;
}