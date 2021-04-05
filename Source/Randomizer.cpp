/*
  ==============================================================================

    Randomizer.cpp
    Created: 4 Apr 2021 1:56:30pm
    Author:  bernardoe

  ==============================================================================
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
