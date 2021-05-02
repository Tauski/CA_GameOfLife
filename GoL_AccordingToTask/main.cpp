
#ifdef _WIN32
#include <iostream>
#include <sstream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <vector>

#elif defined __linux__
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>

#else
#error OS not supported 
#endif

/**
	CONWAY'S GAME OF LIFE 
		- with modifiable board, placable cells and manual or automatic generations

		RULES
		If the cell is alive:
			1. If it has 1 or no neighbors, it will turn "dead". As if by solitude.
			2. if it has 4 or more neighbors, it will turn "dead". As if by overpopulation.
			3. If it has 2 or 3 neighbors, it will remain "alive".
		If the cell is dead:
			1. If it has exactly three neighbors, it will turn "alive", as if by regrowth.
	
	This project uses console output to visually display generations. This is not fastest or even recommended way of doing things.
	But that said this is still quite fast depending of the size of board (I suggest no larger than 50x50 for slower PC:s) and speed of your PC.
	Also it is easy to use and modify how you like. 
*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Global functions

//Checks that given string is able to convert to int
bool isNumber(const std::string &s)
{
	if (s.empty())//user just pressed enter
		return false;

	if (s.length() > 4)//dont allow longer than 4 strings
		return false;

	for (int i = 0; i < s.length(); i++)//goes through each char in string, check for numbers and whitespaces
	{
		if (!(s[i] >= '0' && s[i] <= '9' || !s[i] == ' '))
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

//for console cursor position setting only works on window

#ifdef _WIN32
void setCursorPosition(int x, int y)
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	std::cout.flush();
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hOut, coord);
}

//Clear screen only works on windows
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
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class side start

//Class of the game to keep hold of boards and necessary functions
class GameOfLife
{
public:

	//Contstuctor: gets int width, int height and string initialmode
	GameOfLife(int boardWidth, int boardHeight, std::string menuChoice);

	//Destructor deletes allocated memory
	~GameOfLife();

	//updates the board with next generation
	void onUpdate();

	//draws (outputs) the board into console (# alive . dead)
	void draw(int newStates[]);

	//Places individual cells
	void placeCells();

	//returns the amount of generations before only stills or empty
	const int getGenerations() { return m_generation; };

	//Flag for game end
	bool gameEnd = false;

private:
	int *m_output;			//holds ouput int array
	int *m_state;			//itn array that holds current state
	int m_size;				//holds size of array
	int m_aliveCount;		//counter that holds number of alive cells
	int m_failSafeCounter;	//counter that is incemented if alive cells and old cells are equals
	int m_width;			//holds width of the game array
	int m_height;			//holds height of the game array
	int m_generation;		//holds current generation
	int m_noStateChange;	//holds previous generations number of state changes
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//constructor: takes 2 arguments width and height
GameOfLife::GameOfLife(int boardWidth, int boardHeight, std::string menuChoice)
{
	m_width = boardWidth;
	m_height = boardHeight;
	m_size = boardWidth * boardHeight;
	m_output = new int[m_width * m_height];
	m_state = new int[m_width * m_height];
	memset(m_output, 0, m_width * m_height * sizeof(int));
	memset(m_state, 0, m_width * m_height * sizeof(int));

	//initialization of m_state
	if (menuChoice == "1") //random fill with alive and dead cells
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
#ifdef _WIN32
	clearScreen();
#elif defined __linux__ // don't know about linux console cursor stuff so i just call system("clear") and output new board
	system("clear");
#endif  	
	
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

//update loop of the game
void GameOfLife::onUpdate()
{
	//State change counters
	int noStateChange = 0;
	int noStateChangeOld = m_noStateChange;	

	//values for checking when there's no changes in alive count --> game is still or dead
	int aliveCountNew = 0;
	int oldCount = (m_aliveCount != 0) ? m_aliveCount : 0;

	//lambda function to return the value in output array on x y coordinates (1 or 0)
	auto cell = [&](int x, int y)
	{
		return m_output[y * m_width + x];				
	};	

	//store current state to output state 
	for (int i = 0; i < m_size; i++)
	{
		m_output[i] = m_state[i];
	}

	//nested for loop to go through all cells
	for (int x = 0; x < m_width -0; x++)
	{
		for (int y = 0; y < m_height -0; y++)
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
				if ((m_state[y * m_width + x] = nNeighbours == 2 || nNeighbours == 3) == 1)//if it becomes dead, state changed
				{
					noStateChange++; //check for when no state changes for oscillators and movers						
				}
			}
			else//not alive...
			{
				m_state[y*m_width + x] = nNeighbours == 3; //if it becomes alive		
			}

			//draw cell states
			#ifdef _WIN32
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
			#endif // _WIN32

			if (m_state[y * m_width + x] == 1)
			{
				aliveCountNew++; //get amount of current alive cells
			}			
		}
	}

	//Game logic
	m_generation++; //increase generation
	m_noStateChange = noStateChange; //save it in member	
	m_aliveCount = aliveCountNew;  //save it in member	

	if (m_aliveCount == oldCount) // If they are the same we might be in end condition
	{
		if (noStateChangeOld == noStateChange) //check also statechange amounts
		{
			if (m_noStateChange == m_aliveCount) // when no state changes are lower than alive count we have mover or ocillator 
			{
				m_failSafeCounter++; //repeating cells are in the same spot, increment failsafe counter, sometimes alive count can be same multiple generations				
			}
		}
		else
		{
			m_failSafeCounter = 0;
		}
	}	

	if (m_failSafeCounter == 10 && m_aliveCount == oldCount)//if 10 consecutive generations with exact same alive counts it's highly likely that game is in still or oscillation state 
	{		
		m_generation - 10;		
		gameEnd = true;
	}
	
#ifdef __linux__
	draw(m_state); // seperate draw call for linux 
#endif // !_WIN32
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameOfLife::placeCells()
{
	//getline strings for placing cells
	std::string sCellNumber; //number of manually placable cells
	std::string sCoordinatesX;
	std::string sCoordinatesY;	

	bool cellNumberAnswer = false; //users answer to how many cells he wants to place
	bool coordinatesAnwer = false; //user gave correct coordinates

	//lambda function to set state of a cell
	auto set = [&](int x, int y, std::string s)
	{
		int p = 0;
		for (auto c : s) //loop string s
		{
			m_state[y * m_width + x + p] = c == L'#' ? 1 : 0; //If string has # character set according state to alive else dead
			p++;
		}
	};

	//set maximum placable cells to 1000 even if there were more possible cells
	int maxSize = (m_size > 1000) ? 1000 : m_size;	

	while (!cellNumberAnswer)
	{
		std::cout << "How many cells would you like to place? (MAX =" << maxSize << " ): ";
		std::getline(std::cin, sCellNumber);
		if (isNumber(sCellNumber) && stoi(sCellNumber) < maxSize)
		{
			cellNumberAnswer = true;
		}
	}

	//Counter of placable cells left
	int cellCounter = stoi(sCellNumber);

	for (int i = 0; i < stoi(sCellNumber); i++)
	{
		coordinatesAnwer = false;
		std::cout << "Give coordinates to place the cell in grid (0-" << m_width-1 << " X and 0-" << m_height-1 << " Y " << ")  " << cellCounter << ": left" << std::endl;
		while (!coordinatesAnwer)
		{
			std::getline(std::cin, sCoordinatesX);
			std::getline(std::cin, sCoordinatesY);
			if (!isNumber(sCoordinatesX) || !isNumber(sCoordinatesY) || stoi(sCoordinatesX) > m_width-1 || stoi(sCoordinatesY) > m_height-1) //must be number and x must be less than width and y must be less that height
			{
				std::cout << "Give coordinates to place the cell within grid of size ( 0-" << m_width-1 << " X and 0-" << m_height-1 << " Y " << ")" << std::endl;
			}
			else
			{
				set(stoi(sCoordinatesX), stoi(sCoordinatesY), "#");
				coordinatesAnwer = true;
				cellCounter--;
			}
		}

		if (m_size > 1000) //draw current state only if size is not too big 1000 characters
		{
			continue;
		}
		else
		{
			draw(m_state); 
		}		
	}
}

//Class side end
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Main side start

int main()
{
	//Strings for getlines
	std::string sMenuChoice;
	std::string sWidth;
	std::string sHeight;
	std::string sUpdateTime;
	std::string sStyleChoice;
	std::string sSpace;
	std::string sRestart;
	
	//Checks for user input while loops
	bool restartChoice = false;
	bool menuChoice = false;
	bool boardAnswer = false;
	bool stageAnswer = false;
	bool modeAnswer = false;
	bool timerAnswer = false;
	bool manualStartAnswer = false;
	bool space = false;

	//START	
	while (!restartChoice)
	{
		while (!menuChoice) //Wait for usable board answer
		{
			std::cout << "Game of Life \n" << std::endl;
			std::cout << "(1) Build random board " << std::endl;
			std::cout << "(2) Build your own board " << std::endl;
			std::getline(std::cin, sMenuChoice);
			if (sMenuChoice == "1" || sMenuChoice == "2")
			{
				menuChoice = true;
			}
		}
		while (!boardAnswer) //Wait for usable board answer
		{
			std::cout << "Give size of the board WIDTH x HEIGHT (MAX = 360 x 360)\n" << std::endl;
			std::getline(std::cin, sWidth);
			std::getline(std::cin, sHeight);
			if (isNumber(sWidth) && isNumber(sHeight) && stoi(sWidth) < 360 && stoi(sWidth) < 360)
			{
				boardAnswer = true;
			}
		}

		//create game object instance
		GameOfLife game(stoi(sWidth), stoi(sHeight), sMenuChoice);
		if (sMenuChoice == "2") // handle building board 
		{
			game.placeCells();
		}

		//Select game mode, auto or manual	
		while (!modeAnswer) //Wait for usable board answer
		{
			std::cout << "Give your preferred update mode 'auto(1)' or 'manual(2)'" << std::endl;
			std::getline(std::cin, sStyleChoice);
			if (sStyleChoice == "1" || sStyleChoice == "2")
			{
				modeAnswer = true;
			}
		}

		//User wanted automatic generations
		std::cout << "Give timer length as ms (larger boards take longer to output) : ";
		if (sStyleChoice == "1")
		{
			while (!timerAnswer) //Wait for usable board answer
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

			#ifdef _WIN32
				clearScreen();
			#elif defined __linux__
				system("clear")
			#endif // _WIN32		

			while (!game.gameEnd) //run game loop
			{
				game.onUpdate();
				std::this_thread::sleep_for(std::chrono::milliseconds(stoi(sUpdateTime)));
			}

			std::cout << "Game lasted for: " << game.getGenerations() << " generations" << std::endl;
		}
		else //user wanted manual generations
		{

			#ifdef _WIN32
						clearScreen();
			#elif defined __linux__
						system("clear")
			#endif // _WIN32

			std::cout << "Press space" << std::endl;
			while (!game.gameEnd)
			{
				if (GetAsyncKeyState(VK_SPACE) < 0 && space == false)
				{
					space = true;
				}
				if (GetAsyncKeyState(VK_SPACE) == 0 && space == true) //only proceed after space is released
				{
					game.onUpdate();
					space = false;
				}
			}
			std::cout << "Game lasted for: " << game.getGenerations() << " generations" << std::endl;
			std::cin.get(); //just to keep game closing before seeing generations
		}

		std::cout << "\n RESTART? (Y,N)" << std::endl;
		std::getline(std::cin, sRestart);

		if (sRestart == "n" || sRestart == "N")
		{
			return 0;
		}
	}
}
	//End of main

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
