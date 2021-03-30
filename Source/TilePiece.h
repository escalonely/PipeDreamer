/*
  ==============================================================================

    TilePiece.h
    Created: 26 Mar 2021 7:45:11pm
    Author:  bernardoe

  ==============================================================================
*/

#pragma once

//#include <vector>

class TilePiece
{
public:
	enum Type
	{
		TYPE_NONE = 0,
		TYPE_START_N,
		TYPE_START_S,
		TYPE_START_E,
		TYPE_START_W,
		TYPE_VERTICAL,
		TYPE_HORIZONTAL,
		TYPE_NW_ELBOW,
		TYPE_NE_ELBOW,
		TYPE_SE_ELBOW,
		TYPE_SW_ELBOW,
		TYPE_CROSS,
		TYPE_MAX
	};

	TilePiece();

	TilePiece(Type t);

	virtual ~TilePiece();

	Type GetType() const;

	void SetType(Type t);

protected:
	Type	m_type;
};


/**
 *
 */
class Pipe : public TilePiece
{
public:
	enum Direction
	{
		DIR_NONE = 0,
		DIR_N,
		DIR_S,
		DIR_E,
		DIR_W,
	};

	Pipe(TilePiece::Type t);

	virtual ~Pipe();

	float Pump(float amount);

	float GetOozeLevel() const;

	bool IsFull() const;

	bool IsEmpty() const;

	bool HasOpening(Pipe::Direction dir) const;

	static Pipe::Direction GetOppositeDirection(Pipe::Direction dir);

	bool SetFlowEntry(Pipe::Direction dir);

	Pipe::Direction GetFlowDirection() const;

	void SetFlowDirection(Pipe::Direction dir);

protected:
	/**
	 * TODO
	 */
	float m_oozeLevel;

	/**
	 * TODO
	 */
	Pipe::Direction m_flowDirection;
};

