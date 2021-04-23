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


#include "MainComponent.h"
#include "TilePiece.h"
#include "Board.h"
#include "Queue.h"
#include "Randomizer.h"
#include "ScoreWindow.h"
//#include "Main.cpp"

static constexpr int TILESIZE = 70;
static constexpr int HALFTILE = TILESIZE / 2;
static constexpr int BOARD_VSTARTPOS = 80;
static constexpr int BOARD_HSTARTPOS = 175;


// ---- Helper types and constants ----

const int MainComponent::MIN_SCORE_TO_ADVANCE(2000);
const int MainComponent::SCORE_MULTIPLIER(100);


// ---- Class Implementation ----

MainComponent::MainComponent()
	:	m_blockInteraction(0),
		m_difficultyLevel(1),
		m_cumulativeScore(0),
		m_scoreWindow(nullptr)
{
	// Create GUI component wich will work as a clickable hyperlink to our github.
	m_hyperlink = std::make_unique<juce::HyperlinkButton>(	juce::String("https://github.com/escalonely/PipeDreamer"),
															juce::URL("https://github.com/escalonely/PipeDreamer"));
	m_hyperlink->setFont(juce::Font("consolas", 18.0f, juce::Font::plain), false /* do not resize */);
	m_hyperlink->setColour(juce::HyperlinkButton::textColourId, juce::Colours::grey);
	addAndMakeVisible(m_hyperlink.get());

	// TODO
	//PipeDreamerApplication* app = dynamic_cast<PipeDreamerApplication*>(juce::JUCEApplication::getInstance());
	//if (app != nullptr)
	//{
	//	app->InitApplicationProperties();
	//}

	setSize(900, 620);

	// Create board
	m_board = new Board(10, 7);

	// Create queue
	m_queue = new Queue(5);

	// Initialize randomizer and store pointer
	//  to ensure it is deleted on shutdown.
	m_randomizer = Randomizer::GetInstance();

	// Reset the countdown to the start of the round
	// (before ooze starts pumping out)
	m_countDown = GetCurrentCountdown();

	// GUI-refreh rate
	startTimer(60);
}

MainComponent::~MainComponent()
{
	delete m_board;
	delete m_queue;
	delete m_scoreWindow;
	delete m_randomizer;
}

void MainComponent::resized()
{
	// Position the hyperlink
	m_hyperlink->setBounds(getLocalBounds().getWidth() - 380, getLocalBounds().getHeight() - 50, 350, 40);
}

void MainComponent::timerCallback()
{
	// When it reaches 0, clicks are enabled again.
	if (m_blockInteraction > 0)
		m_blockInteraction--;

	// Countdown to start pumping ooze.
	if (m_countDown > 0)
		m_countDown--;

	else
	{
		// Pump more ooze into the board!
		bool ok = m_board->Pump(GetCurrentOozePerPump());
		if (!ok)
		{
			// Ooze spill! This round if over, show scoreboard.
			stopTimer();

			ScoreWindow::ScoreDetails details;
			details.score = m_board->GetScoreBase() * SCORE_MULTIPLIER;

			// Carryover is the score gained from all previous levels.
			details.carryover = m_cumulativeScore;

			// Add level-based bonus. This mechanic helps ensure that players
			// who make it further into the game end up with higher score than 
			// players who just manage a very long pipe on level 1.
			details.bonus = m_difficultyLevel * SCORE_MULTIPLIER;

			// Add score gained to the cumulative score.
			details.total = details.score + details.bonus + details.carryover;
			m_cumulativeScore = details.total;

			// If score is high enough, score window offers 
			// a button to continue to next level.
			details.level = m_difficultyLevel;
			details.advance = (details.score >= MIN_SCORE_TO_ADVANCE);

			// Show scoreboard overlay.
			m_scoreWindow = new ScoreWindow(details);
			m_scoreWindow->addChangeListener(this);
			addAndMakeVisible(m_scoreWindow);
			m_scoreWindow->setBounds(juce::Rectangle<int>(320, 180, 400, 320));
		}
	}

	this->repaint();
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
	const juce::ScopedLock lock(m_lock);

	if (m_blockInteraction == 0)
	{
		juce::Point<int> clickPos = event.getMouseDownPosition();

		for (int i = 0; i < m_board->GetNumCols(); i++)
		{
			for (int j = 0; j < m_board->GetNumRows(); j++)
			{
				juce::Rectangle<int> tileRect(	BOARD_HSTARTPOS + i * (TILESIZE - 1), 
												BOARD_VSTARTPOS + j * (TILESIZE - 1), 
												TILESIZE, TILESIZE);
				if (tileRect.contains(clickPos))
				{
					static constexpr int framesInteractionBlocked = 5;

					TilePiece* clickedTile = m_board->GetTile(i, j);
					bool replace(clickedTile->GetType() == TilePiece::TYPE_NONE);

					if (!replace)
					{
						Pipe* clickedPipe = dynamic_cast<Pipe*>(clickedTile);
						if ((clickedPipe != nullptr) &&
							(clickedPipe->IsEmpty()) &&		// Only empty tiles can be replaced.
							(!clickedPipe->IsStart()) &&	// Cannot replace starter tiles.
							(m_board->PopBomb()))			// Need bombs to replace existing pipe tiles.
						{
							replace = true;

							// Using bombs disables actions for a few more frames.
							m_blockInteraction += framesInteractionBlocked;
						}
					}

					if (replace)
					{
						// Grab the next piece in the queue, and...
						TilePiece::Type newType = m_queue->Pop();

						// ... place it on the board.
						m_board->ReplaceTile(i, j, newType);

						// To prevent accidental double-clicking disable actions for a few frames.
						m_blockInteraction += framesInteractionBlocked;
					}
				}
			}
		}
	}
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) 
{
	(void)source;

	//const juce::ScopedLock lock(m_lock); // TODO?

	if (m_scoreWindow != nullptr)
	{
		switch (m_scoreWindow->GetCommand())
		{
			case ScoreWindow::CMD_RESTART:
			case ScoreWindow::CMD_CONTINUE:
			{
				m_board->Reset();
				m_queue->Reset();
				m_blockInteraction = 0;

				// If re restart at lvl 1, clear total score
				if (m_scoreWindow->GetCommand() == ScoreWindow::CMD_RESTART)
				{
					m_difficultyLevel = 1;
					m_cumulativeScore = 0;
				}

				// Or advance to the next level
				else
					m_difficultyLevel += 1;

				// Countdown to ooze pumping.
				m_countDown = GetCurrentCountdown();

				// Restart GUI
				startTimer(60);
			}
			break;

			case ScoreWindow::CMD_QUIT:
			{
				juce::JUCEApplicationBase::quit();
			}
			break;
		}

		delete m_scoreWindow;
		m_scoreWindow = nullptr;
	}
}

juce::Colour MainComponent::GetCurrentTileColor() const
{
	static const juce::Colour colorsPerLevel[] = {
		juce::Colour(125, 125, 125),	// Level 1
		juce::Colours::cadetblue,		// Level 2
		juce::Colours::darkkhaki,		// Level 3
		juce::Colour(140, 180, 90),		// Level 4
		juce::Colours::darkslategrey,	// Level 5
		juce::Colours::hotpink,			// Level 6
		juce::Colour(27, 122, 165),		// Level 7
		juce::Colours::coral,			// Level 8
		juce::Colours::blueviolet,		// Level 9
		juce::Colours::darkorange,		// Level 10
		juce::Colours::mediumseagreen,	// Level 11
		juce::Colours::orangered,		// Level 12
	};

	// m_difficultyLevel starts at 1
	int arraySize = sizeof(colorsPerLevel) / sizeof(*colorsPerLevel);
	int level = m_difficultyLevel - 1;
	if (level >= arraySize)
		level = arraySize - 1;

	return colorsPerLevel[level];
}

float MainComponent::GetCurrentOozePerPump() const
{
	static const float oozePerLevel[] = { 
		1.0F, // Level 1
		1.2F, // Level 2
		1.4F, // Level 3
		1.5F, // Level 4
		1.6F, // Level 5
		1.8F, // Level 6
		2.0F, // Level 7
		2.2F, // Level 8
		2.5F, // Level 9
		3.0F, // Level 10
		3.5F, // Level 11
		5.0F  // Level 12
	};

	// m_difficultyLevel starts at 1
	int arraySize = sizeof(oozePerLevel) / sizeof(*oozePerLevel);
	int level = m_difficultyLevel - 1;
	if (level >= arraySize)
		level = arraySize - 1;

	return oozePerLevel[level];
}

int MainComponent::GetCurrentCountdown() const
{
	static const int countdownPerLevel[] = {
		320,	// Level 1
		280,	// Level 2
		240,	// Level 3
		220,	// Level 4
		200,	// Level 5
		180,	// Level 6
		160,	// Level 7
		140,	// Level 8
		120,	// Level 9
		100,	// Level 10
		80,		// Level 11
		60		// Level 12
	};

	// m_difficultyLevel starts at 1
	int arraySize = sizeof(countdownPerLevel) / sizeof(*countdownPerLevel);
	int level = m_difficultyLevel - 1;
	if (level >= arraySize)
		level = arraySize - 1;

	return countdownPerLevel[level];
}

void MainComponent::paint(juce::Graphics& g)
{
	const juce::ScopedLock lock(m_lock);

	// Background colour
	g.fillAll(juce::Colour(67, 67, 67));

	// Draw countdown to ooze.
	DrawOozeMeter(juce::Point<int>(74, 30), g);

	// Draw current level number and score
	DrawLevelAndScore(g);

	// Draw app info
	{
		juce::String infoText("Pipe Dreamer V");
		juce::String versionString(JUCE_STRINGIFY(JUCE_APP_VERSION));
		infoText << versionString;

		juce::Rectangle<int> textRect(BOARD_HSTARTPOS, getLocalBounds().getHeight() - 50, 250, 40);

		g.setFont(juce::Font("consolas", 18.0f, juce::Font::plain));
		g.setColour(juce::Colours::grey);
		g.drawText(infoText, textRect, juce::Justification::left, false);
		//g.drawRect(textRect, 1);

		//textRect = juce::Rectangle<int>(getLocalBounds().getWidth() - 380, getLocalBounds().getHeight() - 50, 350, 40);
		//g.drawRect(textRect, 1);
	}

	// Draw bombs
	DrawBombs(juce::Point<int>(538, 20), g);

	// Draw tile queue
	static constexpr int queueVStartPos = 450;
	static constexpr int queueHStartPos = 50;
	for (int i = 0; i < m_queue->GetSize(); i++)
	{
		// Pipe shape
		juce::Point<int> p(queueHStartPos, queueVStartPos - i * (TILESIZE - 0));
		DrawTile((m_queue->GetTile(i)), p, g);
		DrawCrossSecondWay((m_queue->GetTile(i)), p, g);
		DrawTileDecoration((m_queue->GetTile(i)), p, g);

		if (i == 0)
		{
			// Extra frame for the tile at the start of the queue.
			g.setColour(juce::Colours::red);
			g.drawRect(queueHStartPos, queueVStartPos - i * (TILESIZE - 0), TILESIZE, TILESIZE, 4);
		}
	}

	// Draw board.
	for (int i = 0; i < m_board->GetNumCols(); i++)
	{
		for (int j = 0; j < m_board->GetNumRows(); j++)
		{
			juce::Point<int> p(BOARD_HSTARTPOS + i * (TILESIZE - 1), BOARD_VSTARTPOS + j * (TILESIZE - 1));

			DrawTile(m_board->GetTile(i, j), p, g);
			DrawOoze(m_board->GetTile(i, j), p, g);
			DrawCrossSecondWay(m_board->GetTile(i, j), p, g);
			DrawTileDecoration(m_board->GetTile(i, j), p, g);
		}
	}
}

void MainComponent::DrawLevelAndScore(juce::Graphics& g)
{
	int playerScore = m_board->GetScoreBase() * SCORE_MULTIPLIER;

	g.setFont(juce::Font("consolas", 32.0f, juce::Font::plain));
	g.setColour(juce::Colours::grey);

	juce::Rectangle<int> textRect(BOARD_HSTARTPOS, 20, 92, HALFTILE);
	g.drawText("Level:", textRect, juce::Justification::left, false);
	//g.drawRect(textRect, 1.0f);

	textRect = juce::Rectangle<int>(BOARD_HSTARTPOS + 92 + 52, 20, 92, HALFTILE);
	g.drawText("Score:", textRect, juce::Justification::left, false);
	//g.drawRect(textRect, 1.0f);

	// If score this round is high enough to advance to next difficulty level, highlight the number.
	if (playerScore >= MIN_SCORE_TO_ADVANCE)
	{
		g.setColour(juce::Colours::yellow);
		g.setFont(juce::Font("consolas", 32.0f, juce::Font::bold));
	}
	textRect = juce::Rectangle<int>(BOARD_HSTARTPOS + 92 + 52 + 92, 20, 160, HALFTILE);
	g.drawText(juce::String(playerScore), textRect, juce::Justification::left, false);
	//g.drawRect(textRect, 1.0f);

	// Show difficulty level number in this level's tile color.
	g.setColour(GetCurrentTileColor());
	textRect = juce::Rectangle<int>(BOARD_HSTARTPOS + 92, 20, 52, HALFTILE);
	g.drawText(juce::String(m_difficultyLevel), textRect, juce::Justification::left, false);
	//g.drawRect(textRect, 1.0f);
}

void MainComponent::DrawTile(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	static constexpr float pipeThickness = 20.0f;

	if (tile->GetType() != TilePiece::TYPE_NONE)
	{
		Pipe* pipe = dynamic_cast<Pipe*>(tile);
		if (pipe != nullptr)
		{
			// Draw tile's Background color
			g.setColour(GetCurrentTileColor());
			g.fillRect(origin.getX(), origin.getY(), TILESIZE, TILESIZE);

			juce::Line<int> line;
			g.setColour(juce::Colours::black);

			// Ellipse rect used to have nice rounded corners in the elbow pipes.
			juce::Rectangle<float> elbowJoint(	origin.getX() + HALFTILE - (pipeThickness / 2),
												origin.getY() + HALFTILE - (pipeThickness / 2),
												pipeThickness, 
												pipeThickness);

			switch (pipe->GetType())
			{
			case TilePiece::TYPE_START_N:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(),
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_S:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_E:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE, 
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_W:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX(), 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_VERTICAL:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), pipeThickness);
				}
				break;

			case TilePiece::TYPE_HORIZONTAL:
				{
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE,
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
				}
				break;

			case TilePiece::TYPE_NW_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_NE_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					line = juce::Line<int>(	origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE, 
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_SE_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), pipeThickness);
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_SW_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), pipeThickness);
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE, 
											origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), pipeThickness);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_CROSS:
				{
					Cross* crossTile = dynamic_cast<Cross*>(pipe);
					if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
					{
						// Draw horizontal pipe first. 
						// Vertical pipe will be drawn in DrawCrossSecondWay()
						line = juce::Line<int>(	origin.getX(), 
												origin.getY() + HALFTILE, 
												origin.getX() + TILESIZE, 
												origin.getY() + HALFTILE);
					}
					else
					{
						// Draw vertica pipe first. 
						// Horizontal pipe will be drawn in DrawCrossSecondWay()
						line = juce::Line<int>(	origin.getX() + HALFTILE,
												origin.getY(), 
												origin.getX() + HALFTILE, 
												origin.getY() + TILESIZE);
					}

					g.drawLine(line.toFloat(), pipeThickness);
				}
				break;

			default:
				break;
			}
		}
	}
}

void MainComponent::DrawOoze(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	if (tile->GetType() != TilePiece::TYPE_NONE)
	{
		Pipe* pipe = dynamic_cast<Pipe*>(tile);
		if ((pipe != nullptr) &&
			(!pipe->IsEmpty()))
		{
			static constexpr float oozeThickness = 15.0f;
			bool overHalf(pipe->GetOozeLevel() >= 50.0f);
			bool underHalf(pipe->GetOozeLevel() < 50.0f);
			int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / MAX_OOZE_LEVEL);

			juce::Line<int> line;
			g.setColour(juce::Colours::limegreen);

			// Ellipse rect used to have nice rounded corners in the elbow pipes.
			juce::Rectangle<float> elbowJoint(	origin.getX() + HALFTILE - (oozeThickness / 2),
												origin.getY() + HALFTILE - (oozeThickness / 2),
												oozeThickness, 
												oozeThickness);


			switch (pipe->GetType())
			{
			case TilePiece::TYPE_START_N:
				{
					if (overHalf)
					{
						line = juce::Line<int>(	origin.getX() + HALFTILE, 
												origin.getY() + TILESIZE - fill,
												origin.getX() + HALFTILE,
												origin.getY() + HALFTILE);
						g.drawLine(line.toFloat(), oozeThickness);
						g.fillEllipse(elbowJoint);
					}
				}
				break;

			case TilePiece::TYPE_START_S:
				{
					if (overHalf)
					{
						line = juce::Line<int>(	origin.getX() + HALFTILE, 
												origin.getY() + HALFTILE,
												origin.getX() + HALFTILE,
												origin.getY() + fill);
						g.drawLine(line.toFloat(), oozeThickness);
						g.fillEllipse(elbowJoint);
					}
				}
				break;

			case TilePiece::TYPE_START_E:
				{
					if (overHalf)
					{
						line = juce::Line<int>(	origin.getX() + HALFTILE, 
												origin.getY() + HALFTILE, 
												origin.getX() + fill, 
												origin.getY() + HALFTILE);
						g.drawLine(line.toFloat(), oozeThickness);
						g.fillEllipse(elbowJoint);
					}
				}
				break;

			case TilePiece::TYPE_START_W:
				{
					if (overHalf)
					{
						line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
												origin.getY() + HALFTILE,
												origin.getX() + HALFTILE,
												origin.getY() + HALFTILE);
						g.drawLine(line.toFloat(), oozeThickness);
						g.fillEllipse(elbowJoint);
					}
				}
				break;

			case TilePiece::TYPE_VERTICAL:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_N)
						line = juce::Line<int>(	origin.getX() + HALFTILE, 
												origin.getY() + TILESIZE - fill, 
												origin.getX() + HALFTILE, 
												origin.getY() + TILESIZE);
					else if (pipe->GetFlowDirection() == Pipe::DIR_S)
						line = juce::Line<int>(	origin.getX() + HALFTILE, 
												origin.getY(), 
												origin.getX() + HALFTILE, 
												origin.getY() + fill);

					g.drawLine(line.toFloat(), oozeThickness);
				}
				break;

			case TilePiece::TYPE_HORIZONTAL:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_E)
						line = juce::Line<int>(	origin.getX(), 
												origin.getY() + HALFTILE, 
												origin.getX() + fill, 
												origin.getY() + HALFTILE);
					else if (pipe->GetFlowDirection() == Pipe::DIR_W)
						line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
												origin.getY() + HALFTILE, 
												origin.getX() + TILESIZE, 
												origin.getY() + HALFTILE);

					g.drawLine(line.toFloat(), oozeThickness);
				}
				break;

			case TilePiece::TYPE_NW_ELBOW:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_N)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
					else if (pipe->GetFlowDirection() == Pipe::DIR_W)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
				}
				break;

			case TilePiece::TYPE_NE_ELBOW:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_N)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
					else if (pipe->GetFlowDirection() == Pipe::DIR_E)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
				}
				break;

			case TilePiece::TYPE_SE_ELBOW:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_S)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
					else if (pipe->GetFlowDirection() == Pipe::DIR_E)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
				}
				break;

			case TilePiece::TYPE_SW_ELBOW:
				{
					if (pipe->GetFlowDirection() == Pipe::DIR_S)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
					else if (pipe->GetFlowDirection() == Pipe::DIR_W)
					{
						if (underHalf)
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), oozeThickness);
							g.fillEllipse(elbowJoint);
						}
					}
				}
				break;

			case TilePiece::TYPE_CROSS:
				{
					Cross* crossTile = dynamic_cast<Cross*>(pipe);
					if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
					{
						// Draw horizontally flowing ooze first.
						fill = static_cast<int>(TILESIZE * crossTile->GetOozeLevel(Cross::WAY_HORIZONTAL) / MAX_OOZE_LEVEL);
						if (fill > 0)
						{
							if (pipe->GetFlowDirection() == Pipe::DIR_E)
							{
								line = juce::Line<int>(	origin.getX(),
														origin.getY() + HALFTILE,
														origin.getX() + fill,
														origin.getY() + HALFTILE);
							}
							else
							{
								line = juce::Line<int>(	origin.getX() + TILESIZE - fill,
														origin.getY() + HALFTILE,
														origin.getX() + TILESIZE,
														origin.getY() + HALFTILE);
							}

							g.drawLine(line.toFloat(), oozeThickness);
						}
					}

					else
					{
						// Draw vertically flowing ooze first.
						fill = static_cast<int>(TILESIZE * crossTile->GetOozeLevel(Cross::WAY_VERTICAL) / MAX_OOZE_LEVEL);
						if (fill > 0)
						{
							if (pipe->GetFlowDirection() == Pipe::DIR_N)
							{
								line = juce::Line<int>(	origin.getX() + HALFTILE,
														origin.getY() + TILESIZE - fill,
														origin.getX() + HALFTILE,
														origin.getY() + TILESIZE);
							}
							else
							{
								line = juce::Line<int>(	origin.getX() + HALFTILE,
														origin.getY(),
														origin.getX() + HALFTILE,
														origin.getY() + fill);
							}

							g.drawLine(line.toFloat(), oozeThickness);
						}
					}
				}
				break;

			default:
				break;
			}
		}
	}
}

void MainComponent::DrawCrossSecondWay(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	if (tile->GetType() == TilePiece::TYPE_CROSS)
	{
		Cross* crossTile = dynamic_cast<Cross*>(tile);

		// TODO make class const
		static constexpr float pipeThickness = 20.0f;

		juce::Line<int> line;
		g.setColour(juce::Colours::black);

		// Draw pipe first
		if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
		{
			// Vertical pipe.
			line = juce::Line<int>(	origin.getX() + HALFTILE, 
									origin.getY(), 
									origin.getX() + HALFTILE, 
									origin.getY() + TILESIZE);
		}
		else
		{
			// Horizontal pipe. 
			line = juce::Line<int>(	origin.getX(), 
									origin.getY() + HALFTILE, 
									origin.getX() + TILESIZE, 
									origin.getY() + HALFTILE);
		}
		g.drawLine(line.toFloat(), pipeThickness);

		// Little lines along the pipe, which make the separation between horizontal and vertical 
		// components of the cross-pipe more visually obvious.
		g.setColour(GetCurrentTileColor());
		if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
		{
			// Vertical little lines
			line = juce::Line<int>(	origin.getX() + HALFTILE - static_cast<int>(pipeThickness / 2) - 1,
									origin.getY() + 1,
									origin.getX() + HALFTILE - static_cast<int>(pipeThickness / 2) - 1,
									origin.getY() + TILESIZE - 1);
			g.drawLine(line.toFloat(), 2.0f);
			line = juce::Line<int>(	origin.getX() + HALFTILE + static_cast<int>(pipeThickness / 2) + 1,
									origin.getY() + 1,
									origin.getX() + HALFTILE + static_cast<int>(pipeThickness / 2) + 1,
									origin.getY() + TILESIZE - 1);
			g.drawLine(line.toFloat(), 2.0f);
		}
		else
		{
			// Horizontal little lines
			line = juce::Line<int>(	origin.getX() + 1,
									origin.getY() + HALFTILE - static_cast<int>(pipeThickness / 2) - 1,
									origin.getX() + TILESIZE - 1,
									origin.getY() + HALFTILE - static_cast<int>(pipeThickness / 2) - 1);
			g.drawLine(line.toFloat(), 2.0f);
			line = juce::Line<int>(	origin.getX() + 1,
									origin.getY() + HALFTILE + static_cast<int>(pipeThickness / 2) + 1,
									origin.getX() + TILESIZE - 1,
									origin.getY() + HALFTILE + static_cast<int>(pipeThickness / 2) + 1);
			g.drawLine(line.toFloat(), 2.0f);
		}

		// Draw ooze
		g.setColour(juce::Colours::limegreen);
		int fill;
		static constexpr float oozeThickness = 15.0f;

		// Vertically flowing ooze
		if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
		{
			fill = static_cast<int>(TILESIZE * crossTile->GetOozeLevel(Cross::WAY_VERTICAL) / MAX_OOZE_LEVEL);
			if (fill > 0)
			{
				if (crossTile->GetFlowDirection() == Pipe::DIR_N)
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + TILESIZE - fill,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
				}
				else
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(),
											origin.getX() + HALFTILE,
											origin.getY() + fill);
				}

				g.drawLine(line.toFloat(), oozeThickness);
			}
		}

		// Hoizontally flowing ooze
		else
		{
			fill = static_cast<int>(TILESIZE * crossTile->GetOozeLevel(Cross::WAY_HORIZONTAL) / MAX_OOZE_LEVEL);
			if (fill > 0)
			{
				if (crossTile->GetFlowDirection() == Pipe::DIR_E)
				{
					line = juce::Line<int>(	origin.getX(),
											origin.getY() + HALFTILE,
											origin.getX() + fill,
											origin.getY() + HALFTILE);
				}
				else
				{
					line = juce::Line<int>(	origin.getX() + TILESIZE - fill,
											origin.getY() + HALFTILE,
											origin.getX() + TILESIZE,
											origin.getY() + HALFTILE);
				}

				g.drawLine(line.toFloat(), oozeThickness);
			}
		}
	}
}

void MainComponent::DrawTileDecoration(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	// Frame around tile
	g.setColour(juce::Colours::white);
	g.drawRect(origin.getX(), origin.getY(), TILESIZE, TILESIZE, 1);

	if (tile->GetType() != TilePiece::TYPE_NONE)
	{
		Pipe* pipe = dynamic_cast<Pipe*>(tile);
		if (pipe != nullptr)
		{
			// If this tile has an explosion on it, draw it.
			int exp = pipe->PopExplosion();
			if (exp > 0)
			{
				juce::Path starPath;
				starPath.addStar(juce::Point<float>(static_cast<float>(	origin.getX() + HALFTILE),
																		static_cast<float>(origin.getY() + HALFTILE)),
																		7,								// Number of peaks
																		static_cast<float>(exp * 4),	// Inner radius
																		static_cast<float>(exp * 8),	// Outer radius
																		static_cast<float>(exp * 2));	// Rotation angle
				g.setColour(juce::Colours::orangered);
				g.fillPath(starPath);
			}
		}
	}
}

void MainComponent::DrawOozeMeter(juce::Point<int> origin, juce::Graphics& g)
{
	// Empty vial (background)
	static constexpr int vialHeight = 118;
	juce::Rectangle<int> vialRect(origin.getX(), origin.getY(), 22, vialHeight);
	g.setColour(juce::Colours::black);
	g.fillRect(vialRect);

	// Ooze inside the vial.
	g.setColour(juce::Colours::limegreen);
	static constexpr int oozeMaxHeight = vialHeight - 6;
	int oozeHeight;
	if (m_countDown > 0)
	{
		// Starts at 0, goes to vialHeight		
		oozeHeight = static_cast<int>(oozeMaxHeight - ((m_countDown * oozeMaxHeight) / GetCurrentCountdown()));
	}
	else if (m_board->GetScoreBase() * SCORE_MULTIPLIER < MIN_SCORE_TO_ADVANCE)
	{
		// Starts at vialHeight, goes to 0.
		oozeHeight = static_cast<int>(((MIN_SCORE_TO_ADVANCE - (m_board->GetScoreBase() * SCORE_MULTIPLIER)) * oozeMaxHeight) / MIN_SCORE_TO_ADVANCE);
	}
	else
	{
		// Full yellow vial.
		g.setColour(juce::Colours::yellow);
		oozeHeight = oozeMaxHeight;
	}
	g.fillRect(juce::Rectangle<int>(origin.getX() + 3, origin.getY() + 3 + oozeMaxHeight - oozeHeight, 16, oozeHeight));

	// Vial outline and markings.
	g.setColour(juce::Colours::black);
	//static constexpr float halfVial = 100;
	g.drawLine(origin.getX() + 15.0f, 1.0f + origin.getY() + vialHeight * 0.25f,	origin.getX() + 22.0f, 1.0f + origin.getY() + vialHeight * 0.25f,	2.0f);
	g.drawLine(origin.getX() + 10.0f, 1.0f + origin.getY() + vialHeight * 0.5f,		origin.getX() + 22.0f, 1.0f + origin.getY() + vialHeight * 0.5f,	2.0f);
	g.drawLine(origin.getX() + 15.0f, 1.0f + origin.getY() + vialHeight * 0.75f,	origin.getX() + 22.0f, 1.0f + origin.getY() + vialHeight * 0.75f,	2.0f);
	g.setColour(juce::Colours::white);
	g.drawLine(origin.getX() + 15.0f, origin.getY() + vialHeight * 0.25f,	origin.getX() + 22.0f,	origin.getY() + vialHeight * 0.25f,	1.0f);
	g.drawLine(origin.getX() + 10.0f, origin.getY() + vialHeight * 0.5f,	origin.getX() + 22.0f,	origin.getY() + vialHeight * 0.5f,	1.0f);
	g.drawLine(origin.getX() + 15.0f, origin.getY() + vialHeight * 0.75f,	origin.getX() + 22.0f,	origin.getY() + vialHeight * 0.75f,	1.0f);
	g.drawRect(vialRect);
}

void MainComponent::DrawBombs(juce::Point<int> p, juce::Graphics& g)
{
	for (int i = 0; i < Board::MAX_NUM_BOMBS; i++)
	{
		// Draw the bombs that are still available
		if (i < m_board->GetNumBombs())
		{
			g.setColour(juce::Colours::red);
			g.fillEllipse(juce::Rectangle<int>(p.getX() + i * (TILESIZE - 1), p.getY(), HALFTILE, HALFTILE).toFloat());
		}

		// Draw the one used up bomb, which will soon become available again.
		else if (i == m_board->GetNumBombs())
		{
			g.setColour(juce::Colour(static_cast<juce::uint8>(67 + m_board->GetPercentUntilFreeBomb()), 67, 67));
			g.fillEllipse(juce::Rectangle<int>(p.getX() + i * (TILESIZE - 1), p.getY(), HALFTILE, HALFTILE).toFloat());
		}

		g.setColour(juce::Colours::white);
		g.drawEllipse(juce::Rectangle<int>(p.getX() + i * (TILESIZE - 1), p.getY(), HALFTILE, HALFTILE).toFloat(), 1.0f);
	}
}
