/*
===============================================================================

Copyright (C) 2021 Bernardo Escalona. All Rights Reserved.

  This file is part of the Pipe Dream clone found at:
  https://github.com/escalonely/PipeDreamer

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


#include "Randomizer.h"
#include <assert.h>

/**
 * 
 */
Randomizer* Randomizer::m_singleton = nullptr;


Randomizer::Randomizer()
{
	assert(m_singleton == nullptr); // only one instnce allowed
	m_singleton = this;

	// Seed randomizer
	std::random_device rd;
	m_mt = std::mt19937(rd());

}

Randomizer::~Randomizer()
{
	m_singleton = nullptr;
}

Randomizer* Randomizer::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new Randomizer();
	}
	return m_singleton;
}

int Randomizer::GetWithinRange(int min, int max)
{
	Range r(min, max);
	if (m_distroMap.find(r) == m_distroMap.end())
	{
		Distro d = std::uniform_int_distribution<int>(min, max);
		m_distroMap.insert(std::pair<Range, Distro>(r, d));
	}

	int ret = (m_distroMap[r](m_mt));
	return ret;
}
