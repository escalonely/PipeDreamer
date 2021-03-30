/*
  ==============================================================================

    TilePiece.cpp
    Created: 26 Mar 2021 7:45:11pm
    Author:  bernardoe

  ==============================================================================
*/

#include "TilePiece.h"

static constexpr float MAX_LEVEL = 100.0f;
static constexpr float MIN_LEVEL = 0.0f;

TilePiece::TilePiece()
	: m_type(TYPE_NONE)
{

}

TilePiece::TilePiece(Type t)
	: m_type(t)
{

}

TilePiece::~TilePiece()
{

}

TilePiece::Type TilePiece::GetType() const
{
	return m_type;
}

void TilePiece::SetType(Type t)
{
	m_type = t;
}


/****************************************************************
* Pipe
*****************************************************************/


Pipe::Pipe(TilePiece::Type t)
	: TilePiece(t),
	m_oozeLevel(0.0f)
{
	switch (t)
	{
	case TYPE_START_N:
		m_flowDirection = DIR_N;
		break;
	case TYPE_START_S:
		m_flowDirection = DIR_S;
		break;
	case TYPE_START_E:
		m_flowDirection = DIR_E;
		break;
	case TYPE_START_W:
		m_flowDirection = DIR_W;
		break;
	default:
		m_flowDirection = DIR_NONE;
		break;
	}
}

Pipe::~Pipe()
{

}

float Pipe::Pump(float amount)
{
	m_oozeLevel += amount;

	// TODO remove
	//if (m_oozeLevel > MAX_LEVEL)
	//	m_oozeLevel = 0;

	return m_oozeLevel;
}

float Pipe::GetOozeLevel() const
{
	return m_oozeLevel;
}

bool Pipe::IsFull() const
{
	return (m_oozeLevel >= MAX_LEVEL);
}

bool Pipe::IsEmpty() const
{
	return (m_oozeLevel == MIN_LEVEL);
}

bool Pipe::HasOpening(Pipe::Direction dir) const
{
	switch (m_type)
	{
	case TYPE_START_N:
		return (dir == DIR_N);
	case TYPE_START_S:
		return (dir == DIR_S);
	case TYPE_START_E:
		return (dir == DIR_E);
	case TYPE_START_W:
		return (dir == DIR_W);
	case TYPE_VERTICAL:
		return ((dir == DIR_N) || (dir == DIR_S));
	case TYPE_HORIZONTAL:
		return ((dir == DIR_E) || (dir == DIR_W));
	case TYPE_NW_ELBOW:
		return ((dir == DIR_N) || (dir == DIR_W));
	case TYPE_NE_ELBOW:
		return ((dir == DIR_N) || (dir == DIR_E));
	case TYPE_SE_ELBOW:
		return ((dir == DIR_S) || (dir == DIR_E));
	case TYPE_SW_ELBOW:
		return ((dir == DIR_S) || (dir == DIR_W));
	case TYPE_CROSS:
		return true;
	default:
		break;
	}

	return false;
}

Pipe::Direction Pipe::GetOppositeDirection(Pipe::Direction dir)
{
	switch (dir)
	{
		case DIR_N: 
			return DIR_S;
		case DIR_S: 
			return DIR_N;
		case DIR_E:
			return DIR_W;
		case DIR_W:
			return DIR_E;
	}

	return DIR_NONE;
}

bool Pipe::SetFlowEntry(Pipe::Direction dir)
{
	bool ret(false);

	switch (m_type)
	{
	case TYPE_VERTICAL:
		if (dir == DIR_N)
		{
			m_flowDirection = DIR_S;
			ret = true;

		}
		else if (dir == DIR_S)
		{
			m_flowDirection = DIR_N;
			ret = true;
		}
		break;

	case TYPE_HORIZONTAL:
		if (dir == DIR_E)
		{
			m_flowDirection = DIR_W;
			ret = true;

		}
		else if (dir == DIR_W)
		{
			m_flowDirection = DIR_E;
			ret = true;
		}
		break;

	case TYPE_NW_ELBOW:
		if (dir == DIR_N)
		{
			m_flowDirection = DIR_W;
			ret = true;

		}
		else if (dir == DIR_W)
		{
			m_flowDirection = DIR_N;
			ret = true;
		}
		break;

	case TYPE_NE_ELBOW:
		if (dir == DIR_N)
		{
			m_flowDirection = DIR_E;
			ret = true;

		}
		else if (dir == DIR_E)
		{
			m_flowDirection = DIR_N;
			ret = true;
		}
		break;

	case TYPE_SE_ELBOW:
		if (dir == DIR_S)
		{
			m_flowDirection = DIR_E;
			ret = true;

		}
		else if (dir == DIR_E)
		{
			m_flowDirection = DIR_S;
			ret = true;
		}
		break;

	case TYPE_SW_ELBOW:
		if (dir == DIR_S)
		{
			m_flowDirection = DIR_W;
			ret = true;

		}
		else if (dir == DIR_W)
		{
			m_flowDirection = DIR_S;
			ret = true;
		}
		break;

	case TYPE_CROSS:
		// TODO
		{
			m_flowDirection = Pipe::GetOppositeDirection(dir);
			ret = true;
		}
		break;

	default:
		break;
	}

	return ret;
}

Pipe::Direction Pipe::GetFlowDirection() const
{
	return m_flowDirection;
}

void Pipe::SetFlowDirection(Pipe::Direction dir)
{
	m_flowDirection = dir;
}
