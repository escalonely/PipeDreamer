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

#pragma once


static constexpr float MAX_OOZE_LEVEL = 100.0f;
static constexpr float MIN_OOZE_LEVEL = 0.0f;



/**
 * TODO
 */
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

	static TilePiece* CreateTile(Type t = TilePiece::TYPE_NONE);

	Type GetType() const;

	bool IsStart() const;

	virtual int GetScoreBase() const;

protected:
	void SetType(Type t);

	Type	m_type;
};


/**
 * TODO
 */
class Pipe : public TilePiece
{
public:
	/**
	 * TODO
	 */
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

	virtual float Pump(float amount);

	float GetOozeLevel() const;

	virtual bool IsFull() const;

	virtual bool IsEmpty() const;

	bool HasOpening(Pipe::Direction dir) const;

	static Pipe::Direction GetOppositeDirection(Pipe::Direction dir);

	virtual bool SetFlowEntry(Pipe::Direction dir);

	virtual Pipe::Direction GetFlowDirection() const;

	void Explode();

	int PopExplosion();

	int GetScoreBase() const override;

protected:
	/**
	 * TODO
	 */
	float m_oozeLevel;

	/**
	 * TODO
	 */
	int m_exploding;

	/**
	 * TODO
	 */
	Pipe::Direction m_flowDirection;
};

/**
 * TODO
 */
class Cross : public Pipe
{
public:
	/**
	 * TODO
	 */
	enum Way
	{
		WAY_NONE = 0,
		WAY_VERTICAL,
		WAY_HORIZONTAL
	};

	Cross();

	float Pump(float amount) override;

	float GetOozeLevel(Way w) const;

	bool IsFull() const override;

	bool IsEmpty() const override;

	bool SetFlowEntry(Pipe::Direction dir) override;

	int GetScoreBase() const override;

	Way GetBackgroundWay() const;

protected:
	/**
	 * TODO
	 */
	float m_horizOozeLevel;

	/**
	 * TODO
	 */
	float m_vertOozeLevel;

	bool m_horizWayFree;
	bool m_vertWayFree;

	/**
	 * Which way, horizontal or vertical, is drawn first and thus ends up 
	 * on the background. The second way, will then appear on the foreground. 
	 */
	Way m_backgroundWay;
};