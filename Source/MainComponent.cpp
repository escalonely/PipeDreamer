/*
===============================================================================

Copyright (C) 2021 Bernardo Escalona. All Rights Reserved.

  This file is part of Pipe Dreamer, found at:
  https://github.com/escalonely/PipeDreamer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

===============================================================================
*/


#include "MainComponent.h"
#include "TilePiece.h"
#include "Board.h"
#include "Queue.h"
#include "Randomizer.h"
#include "ScoreWindow.h"


static constexpr int BOARD_VSTARTPOS = 80; // TODO: class constants
static constexpr int BOARD_HSTARTPOS = 175;


// ---- Helper types and constants ----

const int MainComponent::TILESIZE(70);
const int MainComponent::HALFTILE(TILESIZE / 2);
const float MainComponent::PIPE_THICKNESS(20.0f);
const float MainComponent::OOZE_THICKNESS(15.0f);
const int MainComponent::GUI_REFRESH_RATE(60);
const juce::Rectangle<int> MainComponent::m_fastForwardButtonRect(50, 550, 70, 45);

const int MainComponent::MIN_SCORE_TO_ADVANCE(200); // TODO: move to Controller


// ---- Class Implementation ----

MainComponent::MainComponent()
	:	m_blockInteraction(0),
		m_difficultyLevel(1),
		m_cumulativeScore(0),
		m_scoreWindow(nullptr),
		m_fastForward(false)
{
	// Create GUI component wich will work as a clickable hyperlink to our github.
	m_hyperlink = std::make_unique<juce::HyperlinkButton>(	juce::String("https://github.com/escalonely/PipeDreamer"),
															juce::URL("https://github.com/escalonely/PipeDreamer"));
	m_hyperlink->setFont(juce::Font("consolas", 18.0f, juce::Font::plain), false /* do not resize */);
	m_hyperlink->setColour(juce::HyperlinkButton::textColourId, juce::Colours::grey);
	addAndMakeVisible(m_hyperlink.get());

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
	startTimer(GUI_REFRESH_RATE);
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
	const juce::ScopedLock lock(m_lock);

	// When it reaches 0, clicks are enabled again.
	if (m_blockInteraction > 0)
		m_blockInteraction--;

	// Countdown to start pumping ooze.
	if (m_countDown > 0)
	{
		// If fast-forward button is currently toggled on, decrease countdown faster.
		if (m_fastForward)
			m_countDown -= 5;
		else
			m_countDown -= 1;
	}

	else
	{
		// Pump more ooze into the board!
		bool ok = m_board->Pump(GetCurrentOozePerPump());

		// TODO remove param
		Controller::GetInstance()->Pump(m_board->GetScoreValue());

		// Ooze spill! This round is over, show scoreboard.
		if (!ok)
		{
			// Stop refreshing GUI.
			stopTimer();

			ScoreWindow::ScoreDetails details;
			details.score = m_board->GetScoreValue();

			// Carryover is the score gained from all previous levels.
			details.carryover = m_cumulativeScore;

			// Add level-based bonus. This mechanic helps ensure that players
			// who make it further into the game end up with higher score than 
			// players who just manage a very long pipe on level 1.
			details.bonus = 0;
			if (m_difficultyLevel > 1)
				details.bonus = m_difficultyLevel * m_difficultyLevel * 15;

			// Add score gained to the cumulative score.
			details.total = details.score + details.bonus + details.carryover;
			m_cumulativeScore = details.total;

			// If score is high enough, score window offers 
			// a button to continue to next level.
			details.level = m_difficultyLevel;
			details.advance = (details.score >= MIN_SCORE_TO_ADVANCE);

			// Sound effect to trigger, depends on whether the player advanced to next level.
			Controller::SoundID soundID(Controller::SOUND_GAME_OVER);

			juce::Point<int> windowOrigin(0, 0);
			if (details.advance)
			{
				// Position the small AdvanceWindow in the middle of the window.
				windowOrigin = juce::Point<int>(320, 170);

				soundID = Controller::SOUND_LEVEL_UP;
			}

			// Trigger sound effect.
			Controller::GetInstance()->QueueSound(soundID);

			// Show scoreboard overlay.
			m_scoreWindow = ScoreWindow::CreateScoreWindow(details);
			m_scoreWindow->addChangeListener(this);
			addAndMakeVisible(m_scoreWindow);
			m_scoreWindow->setTopLeftPosition(windowOrigin);
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

		// If user clicked on the fast-forward button, toggle fast-forward state.
		// This increases the ooze amount in GetCurrentOozePerPump().
		if (m_fastForwardButtonRect.contains(clickPos))
			m_fastForward = !m_fastForward;

		else
		{
			for (int i = 0; i < m_board->GetNumCols(); i++)
			{
				for (int j = 0; j < m_board->GetNumRows(); j++)
				{
					juce::Rectangle<int> tileRect(BOARD_HSTARTPOS + i * (TILESIZE - 1),
						BOARD_VSTARTPOS + j * (TILESIZE - 1),
						TILESIZE, TILESIZE);
					if (tileRect.contains(clickPos))
					{
						// Default sound effect for placing pipes on the grid.
						Controller::SoundID soundID(Controller::SOUND_CLICK);

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

								// Sound effect should be explosive instead.
								soundID = Controller::SOUND_EXPLODE;
							}
						}

						if (replace)
						{
							// Trigger approproate sound effect.
							Controller::GetInstance()->QueueSound(soundID);

							// Grab the next piece in the queue, and...
							TilePiece::Type newType = m_queue->Pop();

							// ... place it on the board.
							m_board->ReplaceTile(i, j, newType);

							// To prevent accidental double-clicking disable actions for a few frames.
							static constexpr int framesInteractionBlocked = 5;
							m_blockInteraction += framesInteractionBlocked;
						}
					}
				}
			}
		}
	}
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) 
{
	(void)source;

	const juce::ScopedLock lock(m_lock);

	if (m_scoreWindow != nullptr)
	{
		switch (m_scoreWindow->GetCommand())
		{
			case Controller::CMD_RESTART:
			case Controller::CMD_CONTINUE:
				{
					Controller::GetInstance()->Reset(m_scoreWindow->GetCommand());

					// TODO move below to Controller
					m_board->Reset();
					m_queue->Reset();
					m_blockInteraction = 0;
					m_fastForward = false;

					// If re restart at lvl 1, clear total score
					if (m_scoreWindow->GetCommand() == Controller::CMD_RESTART)
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
					startTimer(GUI_REFRESH_RATE);
				}
				break;

			case Controller::CMD_QUIT:
				{
					juce::JUCEApplicationBase::quit();
				}
				break;
				
			default:
				break;
		}

		delete m_scoreWindow;
		m_scoreWindow = nullptr;
	}
}

juce::Colour MainComponent::GetTileColourForLevel(int difficultyLevel)
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

	// difficultyLevel starts at 1
	int arraySize = sizeof(colorsPerLevel) / sizeof(*colorsPerLevel);
	difficultyLevel--;
	if (difficultyLevel >= arraySize)
		difficultyLevel = arraySize - 1;

	return colorsPerLevel[difficultyLevel];
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

	// If fast-forward button is currently toggled on, increase ooze per pump.
	if (m_fastForward)
		return oozePerLevel[level] * 10.0f;

	return oozePerLevel[level];
}

int MainComponent::GetCurrentCountdown() const
{
	static const int countdownPerLevel[] = {
		320,	// Level 1
		290,	// Level 2
		260,	// Level 3
		230,	// Level 4
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

	// Draw button to accelerate game speed.
	DrawFastForwardButton(g);

	// Draw tile queue
	static constexpr int queueVStartPos = 450;
	static constexpr int queueHStartPos = 50;
	for (int i = 0; i < m_queue->GetSize(); i++)
	{
		// Pipe shape
		juce::Point<int> p(queueHStartPos, queueVStartPos - i * (TILESIZE - 1));
		DrawTile((m_queue->GetTile(i)), p, g);
		DrawCrossSecondWay((m_queue->GetTile(i)), p, g);
		DrawTileDecoration((m_queue->GetTile(i)), p, g);

		if (i == 0)
		{
			// Extra frame for the tile at the start of the queue.
			g.setColour(juce::Colours::grey);
			g.drawRect(queueHStartPos - 4, queueVStartPos - 4, TILESIZE + 8, TILESIZE + 8, 2);
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
	int playerScore = m_board->GetScoreValue();

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
	g.setColour(GetTileColourForLevel(m_difficultyLevel));
	textRect = juce::Rectangle<int>(BOARD_HSTARTPOS + 92, 20, 52, HALFTILE);
	g.drawText(juce::String(m_difficultyLevel), textRect, juce::Justification::left, false);
	//g.drawRect(textRect, 1.0f);
}

void MainComponent::DrawTile(TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	if (tile->GetType() != TilePiece::TYPE_NONE)
	{
		Pipe* pipe = dynamic_cast<Pipe*>(tile);
		if (pipe != nullptr)
		{
			// Draw tile's Background color
			g.setColour(GetTileColourForLevel(m_difficultyLevel));
			g.fillRect(origin.getX(), origin.getY(), TILESIZE, TILESIZE);

			juce::Line<int> line;
			static const float pipeHalfThickness = PIPE_THICKNESS / 2.0f;
			g.setColour(juce::Colours::black);

			// Ellipse rect used to have nice rounded corners in the elbow pipes.
			juce::Rectangle<float> elbowJoint(	origin.getX() + HALFTILE - pipeHalfThickness,
												origin.getY() + HALFTILE - pipeHalfThickness,
												PIPE_THICKNESS, 
												PIPE_THICKNESS);

			switch (pipe->GetType())
			{
			case TilePiece::TYPE_START_N:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(),
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_S:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_E:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE, 
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_START_W:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX(), 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_VERTICAL:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
				}
				break;

			case TilePiece::TYPE_HORIZONTAL:
				{
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE,
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
				}
				break;

			case TilePiece::TYPE_NW_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_NE_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY(), 
											origin.getX() + HALFTILE,
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					line = juce::Line<int>(	origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE, 
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_SE_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + TILESIZE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					g.fillEllipse(elbowJoint);
				}
				break;

			case TilePiece::TYPE_SW_ELBOW:
				{
					line = juce::Line<int>(	origin.getX() + HALFTILE,
											origin.getY() + HALFTILE,
											origin.getX() + HALFTILE,
											origin.getY() + TILESIZE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
					line = juce::Line<int>(	origin.getX(), 
											origin.getY() + HALFTILE, 
											origin.getX() + HALFTILE, 
											origin.getY() + HALFTILE);
					g.drawLine(line.toFloat(), PIPE_THICKNESS);
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

					g.drawLine(line.toFloat(), PIPE_THICKNESS);
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
			static const float oozealfThickness = OOZE_THICKNESS / 2.0f;
			bool overHalf(pipe->GetOozeLevel() >= 50.0f);
			bool underHalf(pipe->GetOozeLevel() < 50.0f);
			int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / MAX_OOZE_LEVEL);

			juce::Line<int> line;
			g.setColour(juce::Colours::limegreen);

			// Ellipse rect used to have nice rounded corners in the elbow pipes.
			juce::Rectangle<float> elbowJoint(	origin.getX() + HALFTILE - oozealfThickness,
												origin.getY() + HALFTILE - oozealfThickness,
												OOZE_THICKNESS, 
												OOZE_THICKNESS);


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
						g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
						g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
						g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
						g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

					g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

					g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE - fill, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY(), 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + TILESIZE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + fill, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX(), 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + fill);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
						}
						else
						{
							line = juce::Line<int>(	origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
							line = juce::Line<int>(	origin.getX() + TILESIZE - fill, 
													origin.getY() + HALFTILE, 
													origin.getX() + HALFTILE, 
													origin.getY() + HALFTILE);
							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

							g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

		juce::Line<int> line;
		static const float pipeHalfThickness = PIPE_THICKNESS / 2.0f;
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
		g.drawLine(line.toFloat(), PIPE_THICKNESS);

		// Little lines along the pipe, which make the separation between horizontal and vertical 
		// components of the cross-pipe more visually obvious.
		g.setColour(GetTileColourForLevel(m_difficultyLevel));
		if (crossTile->GetBackgroundWay() == Cross::WAY_HORIZONTAL)
		{
			// Vertical little lines
			line = juce::Line<int>(	static_cast<int>(origin.getX() + HALFTILE - pipeHalfThickness - 1),
									origin.getY() + 1,
									static_cast<int>(origin.getX() + HALFTILE - pipeHalfThickness - 1),
									origin.getY() + TILESIZE - 1);
			g.drawLine(line.toFloat(), 2.0f);
			line = juce::Line<int>( static_cast<int>(origin.getX() + HALFTILE + pipeHalfThickness + 1),
									origin.getY() + 1,
									static_cast<int>(origin.getX() + HALFTILE + pipeHalfThickness + 1),
									origin.getY() + TILESIZE - 1);
			g.drawLine(line.toFloat(), 2.0f);
		}
		else
		{
			// Horizontal little lines
			line = juce::Line<int>(	origin.getX() + 1,
									origin.getY() + HALFTILE - static_cast<int>(pipeHalfThickness) - 1,
									origin.getX() + TILESIZE - 1,
									origin.getY() + HALFTILE - static_cast<int>(pipeHalfThickness) - 1);
			g.drawLine(line.toFloat(), 2.0f);
			line = juce::Line<int>(	origin.getX() + 1,
									origin.getY() + HALFTILE + static_cast<int>(pipeHalfThickness) + 1,
									origin.getX() + TILESIZE - 1,
									origin.getY() + HALFTILE + static_cast<int>(pipeHalfThickness) + 1);
			g.drawLine(line.toFloat(), 2.0f);
		}

		// Draw ooze
		g.setColour(juce::Colours::limegreen);
		int fill;

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

				g.drawLine(line.toFloat(), OOZE_THICKNESS);
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

				g.drawLine(line.toFloat(), OOZE_THICKNESS);
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
	else if (m_board->GetScoreValue() < MIN_SCORE_TO_ADVANCE)
	{
		// Starts at vialHeight, goes to 0.
		oozeHeight = static_cast<int>(((MIN_SCORE_TO_ADVANCE - (m_board->GetScoreValue())) * oozeMaxHeight) / MIN_SCORE_TO_ADVANCE);
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

void MainComponent::DrawFastForwardButton(juce::Graphics& g)
{
	float radius = 13;
	juce::Point<float> origin(m_fastForwardButtonRect.getX() + 24.0f, m_fastForwardButtonRect.getY() + 23.0f);

	juce::Path ffwdPath;
	ffwdPath.addPolygon(juce::Point<float>(static_cast<float>(origin.getX()), static_cast<float>(origin.getY())), 3, radius, -0.52f);
	ffwdPath.addPolygon(juce::Point<float>(origin.getX() + 19.0f, static_cast<float>(origin.getY())), 3, radius, -0.52f);

	float thickness = 1.5f;
	juce::Colour iconColour(juce::Colours::grey);
	juce::Colour frameColour(juce::Colour(27, 27, 27));
	if (m_fastForward)
	{
		thickness = 2.5f;
		iconColour = juce::Colours::red;
		frameColour = juce::Colours::black;
	}

	// Draw ff icon (two little triangles)
	g.setColour(iconColour);
	g.fillPath(ffwdPath);
	g.setColour(juce::Colours::white);
	g.strokePath(ffwdPath, juce::PathStrokeType(thickness, juce::PathStrokeType::curved));

	// Frame around button
	g.setColour(frameColour);
	g.drawRect(m_fastForwardButtonRect.toFloat(), thickness);
}
