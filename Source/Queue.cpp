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