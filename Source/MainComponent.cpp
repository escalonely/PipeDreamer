#include "MainComponent.h"

static constexpr int TILESIZE = 70;
static constexpr int BOARD_VSTARTPOS = 80;
static constexpr int BOARD_HSTARTPOS = 175;

MainComponent::MainComponent()
{
    setSize (900, 600);

	// Create board
	m_board = new Board(10, 7);

	// Create queue
	m_queue = new Queue(5);

	// Amount of ooze to pump every timer callback.
	m_oozeAmount = 0.5f;

	startTimer(60);
}

MainComponent::~MainComponent()
{
	delete m_board;
	delete m_queue;
}

void MainComponent::paint (juce::Graphics& g)
{
	const juce::ScopedLock lock(m_lock);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    //g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);

	static constexpr int queueVStartPos = 450;
	static constexpr int queueHStartPos = 50;

	for (int i = 0; i < m_queue->GetSize(); i++)
	{
		//// Tile's Background color
		//g.setColour(GetTileTypeTestColor(m_queue->GetTileType(i)));
		//g.fillRect(queueHStartPos, queueVStartPos - i * (TILESIZE - 0), TILESIZE, TILESIZE);

		// Pipe shape
		juce::Point<int> p(queueHStartPos, queueVStartPos - i * (TILESIZE - 0));
		DrawTile((m_queue->GetTile(i)), p, g);

		//// Frame
		//g.setColour(juce::Colours::white);
		//g.drawRect(queueHStartPos, queueVStartPos - i * (TILESIZE - 0), TILESIZE, TILESIZE, (i == 0 ? 4 : 1));
	}

	for (int i = 0; i < m_board->GetNumCols(); i++)
	{
		for (int j = 0; j < m_board->GetNumRows(); j++)
		{
			if (m_board->GetTileType(i, j) != TilePiece::TYPE_NONE)
			{
				//// Tile's Background color
				//g.setColour(GetTileTypeTestColor(m_board->GetTileType(i, j)));
				//g.fillRect(BOARD_HSTARTPOS + i * (TILESIZE - 1), BOARD_VSTARTPOS + j * (TILESIZE - 1), TILESIZE, TILESIZE);
			}

			// Pipe shape
			juce::Point<int> p(BOARD_HSTARTPOS + i * (TILESIZE - 1), BOARD_VSTARTPOS + j * (TILESIZE - 1));
			DrawTile(m_board->GetTile(i, j), p, g);
			

			//// Frame
			//g.setColour(juce::Colours::white);
			//g.drawRect(BOARD_HSTARTPOS + i * (TILESIZE - 1), BOARD_VSTARTPOS + j * (TILESIZE - 1), TILESIZE, TILESIZE, 1);
		}
	}
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::timerCallback()
{
	bool ok = m_board->Pump(m_oozeAmount);
	if (!ok)
	{
		stopTimer();
	}

	this->repaint();
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
	const juce::ScopedLock lock(m_lock);

	juce::Point<int> clickPos = event.getMouseDownPosition();

	for (int i = 0; i < m_board->GetNumCols(); i++)
	{
		for (int j = 0; j < m_board->GetNumRows(); j++)
		{
			switch (m_board->GetTileType(i, j))
			{
			case TilePiece::TYPE_START_N:
			case TilePiece::TYPE_START_S:
			case TilePiece::TYPE_START_E:
			case TilePiece::TYPE_START_W:
			case TilePiece::TYPE_VERTICAL:
			case TilePiece::TYPE_HORIZONTAL:
			case TilePiece::TYPE_NW_ELBOW:
			case TilePiece::TYPE_NE_ELBOW:
			case TilePiece::TYPE_SE_ELBOW:
			case TilePiece::TYPE_SW_ELBOW:
			case TilePiece::TYPE_CROSS:
				break;

			default:
				break;
			}

			juce::Rectangle<int> tile(BOARD_HSTARTPOS + i * (TILESIZE - 1), BOARD_VSTARTPOS + j * (TILESIZE - 1), TILESIZE, TILESIZE);
			if (tile.contains(clickPos))
			{
				TilePiece::Type newType = m_queue->Pop();
				m_board->SetTileType(i, j, newType);
			}
		}
	}
}

juce::Colour MainComponent::GetTileTypeTestColor(TilePiece::Type t)
{
	switch (t)
	{
	case TilePiece::TYPE_START_N:
		return juce::Colours::teal;
	case TilePiece::TYPE_START_S:
		return juce::Colours::plum;
	case TilePiece::TYPE_START_E:
		return juce::Colours::aliceblue;
	case TilePiece::TYPE_START_W:
		return juce::Colours::bisque;
	case TilePiece::TYPE_VERTICAL:
		return juce::Colours::darksalmon;
	case TilePiece::TYPE_HORIZONTAL:
		return juce::Colours::firebrick;
	case TilePiece::TYPE_NW_ELBOW:
		return juce::Colours::goldenrod;
	case TilePiece::TYPE_NE_ELBOW:
		return juce::Colours::khaki;
	case TilePiece::TYPE_SE_ELBOW:
		return juce::Colours::mediumaquamarine;
	case TilePiece::TYPE_SW_ELBOW:
		return juce::Colours::olivedrab;
	case TilePiece::TYPE_CROSS:
		return juce::Colours::tan;

	default:
		break;
	}

	return juce::Colours::black;
}

void MainComponent::DrawTile(const TilePiece* tile, juce::Point<int> origin, juce::Graphics& g)
{
	// Draw tile's Background color
	if (tile->GetType() != TilePiece::TYPE_NONE)
	{
		g.setColour(GetTileTypeTestColor(tile->GetType()));
		g.fillRect(origin.getX(), origin.getY(), TILESIZE, TILESIZE);
	}

	// Draw pipe shape
	static constexpr float pipeThickness = 20.0f;
	static constexpr float oozeThickness = 15.0f;
	static constexpr int halfTile = TILESIZE / 2;

	juce::Line<int> line;
	g.setColour(juce::Colours::black);
	const Pipe* pipe = dynamic_cast<const Pipe*>(tile);
	if (pipe != nullptr)
	{
		switch (pipe->GetType())
		{
			case TilePiece::TYPE_START_N:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX(), origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_START_S:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX(), origin.getY() + TILESIZE);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_START_E:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + TILESIZE, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);

				if (pipe->GetOozeLevel() > 50.0f)
				{
					int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / 100.0f);
					line = juce::Line < int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);

					g.setColour(juce::Colours::limegreen);
					g.drawLine(line.toFloat(), oozeThickness);
				}
			}
			break;

		case TilePiece::TYPE_START_W:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX(), origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_VERTICAL:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + TILESIZE);
				g.drawLine(line.toFloat(), pipeThickness);

				if (!pipe->IsEmpty())
				{
					int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / 100.0f);
					if (pipe->GetFlowDirection() == Pipe::DIR_N)
						line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + TILESIZE - fill, origin.getX() + halfTile, origin.getY() + TILESIZE);
					else if (pipe->GetFlowDirection() == Pipe::DIR_S)
						line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + fill);

					g.setColour(juce::Colours::limegreen);
					g.drawLine(line.toFloat(), oozeThickness);
				}
			}
			break;

		case TilePiece::TYPE_HORIZONTAL:
			{
				line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + TILESIZE, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);

				if (!pipe->IsEmpty())
				{
					int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / 100.0f);
					if (pipe->GetFlowDirection() == Pipe::DIR_E)
						line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);
					else if (pipe->GetFlowDirection() == Pipe::DIR_W)
						line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);

					g.setColour(juce::Colours::limegreen);
					g.drawLine(line.toFloat(), oozeThickness);
				}
			}
			break;

		case TilePiece::TYPE_NW_ELBOW:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
				line = juce::Line < int>(origin.getX(), origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);

				if (!pipe->IsEmpty())
				{
					g.setColour(juce::Colours::limegreen);
					int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / 100.0f);

					if (pipe->GetFlowDirection() == Pipe::DIR_N)
					{
						if (pipe->GetOozeLevel() < 50.0f)
						{
							line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + halfTile);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + TILESIZE - fill, origin.getX() + halfTile, origin.getY() + TILESIZE);
							g.drawLine(line.toFloat(), oozeThickness);
						}
					}
					else if (pipe->GetFlowDirection() == Pipe::DIR_W)
					{
						if (pipe->GetOozeLevel() < 50.0f)
						{
							line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + fill);
							g.drawLine(line.toFloat(), oozeThickness);
						}
						else
						{
							line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + halfTile);
							g.drawLine(line.toFloat(), oozeThickness);
							line = juce::Line<int>(origin.getX() + TILESIZE - fill, origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + halfTile);
							g.drawLine(line.toFloat(), oozeThickness);
						}
					}
				}
			}
			break;

		case TilePiece::TYPE_NE_ELBOW:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
				line = juce::Line <int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + TILESIZE, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_SE_ELBOW:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + TILESIZE);
				g.drawLine(line.toFloat(), pipeThickness);
				line = juce::Line <int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + TILESIZE, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_SW_ELBOW:
			{
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + TILESIZE);
				g.drawLine(line.toFloat(), pipeThickness);
				line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + halfTile, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
			}
			break;

		case TilePiece::TYPE_CROSS:
			{
				line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + TILESIZE, origin.getY() + halfTile);
				g.drawLine(line.toFloat(), pipeThickness);
				line = juce::Line<int>(origin.getX() + halfTile, origin.getY(), origin.getX() + halfTile, origin.getY() + TILESIZE);
				g.drawLine(line.toFloat(), pipeThickness);

				//if (!pipe->IsEmpty())
				//{
				//	int fill = static_cast<int>(TILESIZE * pipe->GetOozeLevel() / 100.0f);
				//	if (pipe->GetFlowDirection() == Pipe::DIR_E)
				//		line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);
				//	else if (pipe->GetFlowDirection() == Pipe::DIR_W)
				//		line = juce::Line<int>(origin.getX(), origin.getY() + halfTile, origin.getX() + fill, origin.getY() + halfTile);

				//	g.setColour(juce::Colours::limegreen);
				//	g.drawLine(line.toFloat(), oozeThickness);
				//}
			}
			break;

		default:
			break;
		}
	}

	// Frame
	g.setColour(juce::Colours::white);
	g.drawRect(origin.getX(), origin.getY(), TILESIZE, TILESIZE, 1);
}