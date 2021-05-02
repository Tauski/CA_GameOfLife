
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <vector>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Global checks

//Checks that given string is able to convert to int
bool isNumber(const std::string &s);

//faster string to int conversion
int fast_stoi(const char *p);
//bool isValid(const std::string &x, const std::string &y);

//for console cursor position setting (faster update but not so nice looking)
void setCursorPosition(int x, int y)
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	std::cout.flush();
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hOut, coord);
}

//Clear screen to avoid system
void clearScreen()
{
	HANDLE hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count;
	DWORD cellCount;
	COORD homeCoords = { 0,0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;
	

	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
	))return;

	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	))return;

	SetConsoleCursorPosition(hStdOut, homeCoords);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class side start

//Class of the game to keep hold of boards and necessary functions
class GameOfLife
{
public:

	GameOfLife(int boardWidth, int boardHeight, std::string initialMode); // each game is created with width and height
	~GameOfLife();

	//updates the board with next generation
	void onUpdate();

	//draws (outputs) the board into console (# alive . dead)
	void draw(int newStates[]);		

	//builds the board with user placed cells or patterns
	void build();	

	//returns the size of board
	int getSize() { return m_size; };

	//outputs popular prebuild game of life patterns where user can choose what to place
	std::vector<int> showPatterns();

	//Places individual cells
	void placeCells();

	//Places predetermined patterns
	void placePatterns();	

private:
	int *m_output;
	int *m_state;
	int m_size;
	int m_width;
	int m_height;
	int m_generation;	
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//constructor: takes 2 arguments width and height
GameOfLife::GameOfLife(int boardWidth, int boardHeight, std::string initialMode)
{
	m_width = boardWidth;
	m_height = boardHeight;
	m_size = boardWidth * boardHeight;
	m_output = new int[m_width * m_height];
	m_state = new int[m_width * m_height];
	memset(m_output, 0, m_width * m_height * sizeof(int));
	memset(m_state, 0, m_width * m_height * sizeof(int));

	//initialization of m_state
	if (initialMode == "random") //random fill with alive and dead cells
	{	
		for (int i = 0; i < m_size; i++)
		{
			m_state[i] = rand() % 2;
		}
	}
	else //fill with dead cells
	{
		for (int i = 0; i < m_size; i++)
		{
			m_state[i] = 0;
		}
	}
	draw(m_state);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GameOfLife::~GameOfLife()
{
	delete[] m_output;
	delete[] m_state;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Draws the current states of cells.
void GameOfLife::draw(int newStates[])
{
	clearScreen();
	std::cout << "Current generation :  " << m_generation << " \n" << std::endl;
	std::string sOut;

	//visualize output as a grid 
	int row = 0;
	for (int i = 0; i < m_size; i++)
	{
		row++;
		if (row == m_width && i != m_size) //When at end of a row add line change
		{
			sOut = (m_state[i] == 1) ? "#" : ".";
			std::cout << sOut << std::endl;
			row = 0;
		}
		else //No line change
		{
			sOut = (m_state[i] == 1) ? "#" : ".";
			std::cout << sOut;
		}
	}	
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Update loop of the game
void GameOfLife::onUpdate()
{
	//output current generation
	std::cout << "Current Generation: " << m_generation << "\n" << std::endl;

	//lambda function to return the value in output array on x y coordinates (1 or 0)
	auto cell = [&](int x, int y)
	{
		return m_output[y * m_width + x];
	};

	//store current state to output state output state
	for (int i = 0; i < m_size; i++)
	{
		m_output[i] = m_state[i];
	}

	//nested for loop to go through all cells
	for (int x = 0; x < m_width - 0; x++)
	{
		for (int y = 0; y < m_height - 0; y++)
		{
			/*						Neighbours of one cell
			
								x-1 y-1		X-0 y-1			X+1 y-1
								X-1 y-0	   this cell		X+1 y+0
								X-1	y+1		X+0 y+1			X+1 y+1
			*/
			int nNeighbours = cell(x - 1, y - 1) + cell(x - 0, y - 1) + cell(x + 1, y - 1) +
							  cell(x - 1, y + 0) +			0		  +	cell(x + 1, y + 0) +
							  cell(x - 1, y + 1) + cell(x + 0, y + 1) + cell(x + 1, y + 1);

			if (cell(x, y) == 1)//when alive...
			{
				m_state[y * m_width + x] = nNeighbours == 2 || nNeighbours == 3; //and if 2 or 3 neighbours, stay alive else die
			}
			else//not alive
			{
				m_state[y*m_width + x] = nNeighbours == 3; //come alive when 3 neighbours
			}

			//Fast but not so nice looking... if you want to try uncomment this and comment draw at the end of scope		

			if (cell(x, y) == 1)
			{
				setCursorPosition(x, y);
				std::cout << "#";
			}
			else
			{
				setCursorPosition(x, y);
				std::cout << ".";
			}			
		}
	}

	
	m_generation++; // generation counter
	//draw(m_output); // draw states 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameOfLife::build()
{
	std::string sBuildMode;
	bool buildModeAnswer = false;

	//Get users preference of building the board
	std::cout << "Would you like to place Individual 'cells', 'patterns' or 'both'?: ";
	while (!buildModeAnswer)
	{
		std::getline(std::cin, sBuildMode);
		if (sBuildMode == "cells" || sBuildMode == "patterns" || sBuildMode == "both")
		{
			buildModeAnswer = true;
		}
		else
		{
			std::cout << "Would you like to place Individual 'cells', 'patterns' or 'both'?: ";
		}
	}

	std::cout << "got mode answer" << std::endl;

	if (sBuildMode == "cells")
	{
		placeCells();
	}
	else if (sBuildMode == "patterns")
	{
		placePatterns();
	}
	else
	{
		placePatterns();
		placeCells();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameOfLife::placePatterns()
{
	bool patternCountAnswer = false;
	std::string sPatternCount;

	//lambda function to set state of a cell
	auto set = [&](int x, int y, std::string s)
	{
		int p = 0;
		for (auto cell : s)
		{
			m_state[y * m_width + x + p] = cell == L'#' ? 1 : 0;
			p++;
		}
	};

	//int maxPatterns = (m_size > 1000) ? 1000 : m_size;
	int maxPatterns = 100;
	std::cout << "Please enter number or patterns you wish to place (MAX = " << maxPatterns << ") : " << std::endl;
	while(!patternCountAnswer)
	{
		std::getline(std::cin, sPatternCount);
		if (!isNumber(sPatternCount) || stoi(sPatternCount) > maxPatterns)
		{
			std::cout << "Please enter number of patterns you wish to place: " << std::endl;
		}
		else
		{
			patternCountAnswer = true;
		}
	}

	for (int i = 0; i < stoi(sPatternCount); i++)
	{
		std::vector<int> placePattern = showPatterns();

		std::cout << "placePattern : " << placePattern.at(0) << " " << placePattern.at(1) << " " << placePattern.at(2);

		int x = placePattern.at(1);
		int y = placePattern.at(2);

		switch (placePattern.at(0))
		{
		case 1:
			set(x, y, "##");
			set(x, y + 1, "##");
			break;
		case 2:
			set(x, y, " ## ");
			set(x, y + 1, "#  #");
			set(x, y + 2, " ## ");
			break;
		case 3:
			set(x, y, " ## ");
			set(x, y + 1, "#  #");
			set(x, y + 2, " # #");
			set(x, y + 3, "  # ");
			break;
		case 4:
			set(x, y, "##");
			set(x, y + 1, "# #");
			set(x, y + 2, " #");
			break;
		case 5:
			set(x, y, " #");
			set(x, y + 1, "# #");
			set(x, y + 2, " #");
			break;
		case 6:
			set(x, y, " # ");
			set(x, y + 1, " # ");
			set(x, y + 2, " # ");
			break;
		case 7:
			set(x, y, "    ");
			set(x, y + 1, " ###");
			set(x, y + 2, "### ");
			set(x, y + 3, "    ");
			break;
		case 8:
			set(x, y, "##  ");
			set(x, y + 1, "##  ");
			set(x, y + 2, "  ##");
			set(x, y + 3, "  ##");
			break;
		case 9:
			set(x, y, "  #  ");
			set(x, y + 1, "  #  ");
			set(x, y + 2, " # # ");
			set(x, y + 3, "  #  ");
			set(x, y + 4, "  #  ");
			set(x, y + 5, "  #  ");
			set(x, y + 6, "  #  ");
			set(x, y + 7, " # # ");
			set(x, y + 8, "  #  ");
			set(x, y + 9, "  #  ");
			break;
		case 10:
			set(x, y + 1, " ##");
			set(x, y + 2, "##");
			set(x, y + 3, " #");
			break;
		case 11:
			set(x, y + 1, "      # ");
			set(x, y + 2, "##      ");
			set(x, y + 3, " #   ###");
			break;
		case 12:
			set(x, y + 1, " #     ");
			set(x, y + 2, "   #   ");
			set(x, y + 3, "##  ###");
			break;
		case 13:
			set(x, y + 1, " # ");
			set(x, y + 2, "  #");
			set(x, y + 3, "###");
			break;
		case 14:
			set(x, y, "#  # ");
			set(x, y + 1, "    #");
			set(x, y + 2, "#   #");
			set(x, y + 3, " ####");
			break;
		case 15:
			set(x, y, "  #   ");
			set(x, y + 1, "#   # ");
			set(x, y + 2, "     #");
			set(x, y + 3, "#    #");
			set(x, y + 4, " #####");
			break;
		}
		clearScreen();
		draw(m_state);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameOfLife::placeCells()
{
	std::string sCellNumber; //number of manually placable cells
	std::string sCoordinatesX;
	std::string sCoordinatesY;

	bool cellNumberAnswer = false; //users answer to how many cells he wants to place
	bool coordinatesAnwer = false; //user gave correct coordinates

	//lambda function to set state of a cell
	auto set = [&](int x, int y, std::string s)
	{
		int p = 0;
		for (auto cell : s)
		{
			m_state[y * m_width + x + p] = cell == L'#' ? 1 : 0;
			p++;
		}
	};

	//set maximum placable cells to 1000 even if there were more possible cells
	int maxSize = (m_size > 1000) ? 1000 : m_size;

	//Real logic
	std::cout << "How many cells would you like to place? (MAX =" << m_size << " ): ";
	while (!cellNumberAnswer)
	{
		std::getline(std::cin, sCellNumber);
		if (!isNumber(sCellNumber))
		{
			std::cout << "How many cells would you like to place? Enter only numbers 0123456789";
		}
		else
		{
			cellNumberAnswer = true;
		}
	}

	int cellCounter = stoi(sCellNumber);
	for (int i = 0; i < stoi(sCellNumber); i++)
	{		
		coordinatesAnwer = false;
		std::cout << "Give coordinates to place the cell in grid (" << m_width << " X and " << m_height << " Y " << ")  " << cellCounter << ": left" << std::endl;
		while (!coordinatesAnwer)
		{
			std::getline(std::cin, sCoordinatesX);
			std::getline(std::cin, sCoordinatesY);
			if (!isNumber(sCoordinatesX) || !isNumber(sCoordinatesY) ) //must be number and x must be less than width and y must be less that height
			{
				std::cout << "Give coordinates to place the cell within grid of size (" << m_width << " X and " << m_height << " Y " << ")" << std::endl;
			}
			else
			{
				set(stoi(sCoordinatesX) - 1, stoi(sCoordinatesY) - 1, "#");
				coordinatesAnwer = true;
				cellCounter--;
			}
		}
		draw(m_state);		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<int> GameOfLife::showPatterns()
{
	bool showAnswer = false;
	bool patternLocationAnswer = false;
	std::string sOkAnswer;
	std::string sLocationAnswerX;
	std::string sLocationAnswerY;

	std::cout << "Common patterns in Game of Life \n" << std::endl;

	std::cout << "--------------------------------------------------------- \n" << std::endl;

	std::cout << "STILLS: So no change in next generation \n" << std::endl;
	
	std::cout << "block 1 " << "beehive 2 " << " loaf 3 " << " boat 4 " << " tub 5\n" << std::endl;
	std::cout << "....    " << "......    " << "......  " << ".....   " << "..... \n"
				 ".##.    " << "..##..    " << "..##..  " << ".##..   " << "..#.. \n"
				 ".##.    " << ".#..#.    " << ".#..#.  " << ".#.#.   " << ".#.#. \n"
				 "....    " << "..##..    " << "..#.#.  " << "..#..   " << "..#.. \n"
				 "        " << "......    " << "...#..  " << ".....   " << "..... \n" << std::endl;

	std::cout << "--------------------------------------------------------- \n" << std::endl;

	std::cout << "OSCILLATORS: Patterns that return to their original state after finite number of generations \n" << std::endl;

	std::cout << "blinker 6 " << " toad 7 " << "beacon 8 " << " penta-decathlon 9\n" << std::endl;
	std::cout << ".....     " << "......  " << "......   " << "..#..  \n"
				 "..#..     " << "..###.  " << ".##...   " << "..#..  \n"
				 "..#..     " << ".###..  " << ".##...   " << ".#.#.  \n"
				 "..#..     " << "......  " << "...##.   " << "..#..  \n"
				 "          " << "        " << "...##.   " << "..#..  \n"
				 "          " << "        " << "......   " << "..#..  \n"
				 "          " << "        " << "         " << "..#..  \n"
				 "          " << "        " << "         " << ".#.#.  \n"
				 "          " << "        " << "         " << "..#..  \n"
			     "          " << "        " << "         " << "..#..  \n" << std::endl;

	std::cout << "--------------------------------------------------------- \n" << std::endl;

	std::cout << " METHUSELAHS: Patterns that evolve for long periods before stabilizing \n" << std::endl;

	std::cout << "R-pentomino 10 " << "  diehard 11 " << " acorn 12 \n" << std::endl;
	std::cout << ".....	         " << "..........   " << ".........  \n"
				 "..##.	         " << ".......#..   " << "..#......  \n"
				 ".##..	         " << ".##.......   " << "....#....  \n"
				 "..#..	         " << "..#...###.   " << ".##..###.  \n"
				 ".....	         " << "..........   " << ".........  \n" << std::endl;

	std::cout << "--------------------------------------------------------- \n" << std::endl;

	std::cout << " SPACESHIPS: Patterns that move across the grid\n" << std::endl;

	std::cout << "glider 13   " << " LW-ship 14 " << " MW-ship 15\n" << std::endl;
	std::cout << ".....	      " << "...#..#...  " << "....#....  \n"
				 "..#..	      " << ".......#..  " << "..#...#..  \n"
				 "...#.	      " << "...#...#..  " << ".......#.  \n"
				 ".###.	      " << "....####..  " << "..#....#.  \n"
				 ".....	      " << "..........  " << "...#####.  \n" << std::endl;

	std::cout << "--------------------------------------------------------- \n" << std::endl;

	std::cout << "Please choose pattern 1-15 : " << std::endl;
	while (!showAnswer)
	{
		std::getline(std::cin, sOkAnswer);
		if (!isNumber(sOkAnswer) || stoi(sOkAnswer) < 15 && stoi(sOkAnswer) > 0)
		{
			showAnswer = true;
		}
		else
		{
			std::cout << "Please choose pattern 1-15 : " << std::endl;
		}
	}

	std::cout << "Please choose patterns 0.0 location in grid coordinates X Y (MAX_X= "<< m_width <<",  MAX_Y = "<< m_height <<"): " << std::endl;
	while (!patternLocationAnswer)
	{
		std::getline(std::cin, sLocationAnswerX);
		std::getline(std::cin, sLocationAnswerY);
		if (!isNumber(sLocationAnswerX) || !isNumber(sLocationAnswerY) && stoi(sLocationAnswerX) > m_width || stoi(sLocationAnswerY) > m_height)
		{
			std::cout << "Please choose patterns 0.0 location in grid coordinates X Y (MAX_X= " << m_width << ",  MAX_Y = " << m_height << "): " << std::endl;
		}
		else
		{
			patternLocationAnswer = true;
		}
	}

	std::vector<int> returnVec;
	returnVec.push_back(stoi(sOkAnswer));
	returnVec.push_back(stoi(sLocationAnswerX));
	returnVec.push_back(stoi(sLocationAnswerY));

	std::cout << "return vec" << returnVec.at(0) << std::endl;
	return returnVec;
	
}

//Class side end
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main side start

int main()
{

	//Strings for getlines
	std::string sWidth;
	std::string sHeight;
	std::string sInitialMode;
	std::string sUpdateTime;
	std::string sStyleChoice;	

	//Checks for user input while loops
	bool boardAnswer = false;
	bool stageAnswer = false;
	bool modeAnswer = false;
	bool timerAnswer = false;

	//START
	std::cout << "Game of Life \n" << std::endl;
	std::cout << "Give size of the board WIDTH x HEIGHT (MAX = 1000 x 1000)\n" << std::endl;
	while (!boardAnswer) //Wait for usable board answer
	{		
		std::getline(std::cin, sWidth);
		std::getline(std::cin, sHeight);	
		if (!isNumber(sWidth) || !isNumber(sHeight))
		{
			std::cout << "Give size of the board WIDTH x HEIGHT as numbers 0123456789:  " << std::endl;
		}
		else
		{
			boardAnswer = true;
		}
	}

	std::cout << "Build your initial game state or randomize. Enter 'build' or 'random'" << std::endl;	
	while (!stageAnswer)
	{
		std::getline(std::cin, sInitialMode);
		if (sInitialMode != "build" && sInitialMode != "random")
		{
			std::cout << "Please give valid answer! Enter 'build' or 'random'" << std::endl;
		}
		else
		{
			stageAnswer = true;
		}
	}

	//create game object instance
	GameOfLife game(stoi(sWidth), stoi(sHeight), sInitialMode);
	if (sInitialMode == "build") // handle building in class side
	{
		game.build();
	}

	//Select game mode, auto or manual
	std::cout << "Give you preferred update mode 'auto' or 'manual' (with auto you choose timer lenght, with manual you press space when you want new generations.)" << std::endl;	
	while (!modeAnswer) //wait for mode answer
	{
		std::getline(std::cin, sStyleChoice);
		std::cout << sStyleChoice << std::endl;
		if (sStyleChoice != "auto" && sStyleChoice != "manual")
		{
			std::cout << "Please give correct style 'auto' or 'manual'" << std::endl;
		}
		else
		{
			modeAnswer = true;
		}
	}

	//User wanted automatic generations
	std::cout << "Give timer length as ms (keep in mind that larger boards take longet to print so if your board is over 50x50 you might not get fast refresh rate) : ";
	if (sStyleChoice == "auto")
	{
		while (!timerAnswer)//Wait for users correct update time input
		{
			std::getline(std::cin, sUpdateTime);
			if (!isNumber(sUpdateTime))
			{
				std::cout << "Please give correct timer as number" << std::endl;
			}
			else
			{
				timerAnswer = true;
			}
		}				

		while (true) //run game loop
		{
			game.onUpdate();
			std::this_thread::sleep_for(std::chrono::milliseconds(stoi(sUpdateTime)));
		}
	}
	else //user wanted manual generations
	{
		std::cout << "Press SPACE to generate next generation" << std::endl;

		while (true)
		{		
			if (GetKeyState(VK_SPACE) & 0x80)
			{
				std::cout << "Space was pressed " << std::endl;
				game.onUpdate();
			}
		}		
	}

	return 0;
}

bool isNumber(const std::string &s)
{
	if (s.empty())//user just pressed enter
		return false;
	

	for (int i = 0; i < s.length(); i++)//goes through each char in string, check for numbers
	{	
		if (!(s[i] >= '0' && s[i] <= '9'))
		{
			return false;
		}
		else if (stoi(s) > 1000) // 1000 is max numerical im willing to handle this way. feel free to change it but it might brick :D
		{
			return false;
		}
	}
	return true;
}

//"naive" fast conversion of string to int
int fast_stoi(const char *p)
{
	int x = 0;
	bool neg = false;
	if (*p == '-')
	{
		neg = true;
		++p;
	}

	while (*p >= '0' && *p <= '9')
	{
		x = (x * 10) + (*p - '0');
		++p;
	}
	if (neg)
	{
		x = -x;
	}
	return x;
}

//Main side end
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////