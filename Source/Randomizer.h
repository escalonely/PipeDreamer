/*
  ==============================================================================

    Randomizer.h
    Created: 4 Apr 2021 1:56:30pm
    Author:  bernardoe

  ==============================================================================
*/

#pragma once

#include <random>
#include <map>

/**
 * TODO
 */
class Randomizer
{
public:
	Randomizer();

	~Randomizer();

	static Randomizer* GetInstance();

	int GetWithinRange(int min, int max);

protected:
	static Randomizer* m_singleton;

	typedef std::pair<int, int> Range;
	typedef std::uniform_int_distribution<int> Distro;

	std::map<Range, Distro> m_distroMap;

	std::mt19937 m_mt;
};