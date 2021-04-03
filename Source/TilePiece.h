/*
  ==============================================================================

    TilePiece.h
    Created: 26 Mar 2021 7:45:11pm
    Author:  bernardoe

  ==============================================================================
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

	virtual float Pump(float amount) override;

	float GetOozeLevel(Way w) const;

	virtual bool IsFull() const override;

	virtual bool IsEmpty() const override;

	virtual bool SetFlowEntry(Pipe::Direction dir) override;


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
};