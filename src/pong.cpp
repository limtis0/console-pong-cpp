#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <time.h>
#include <conio.h> // For kbhit
#include <fstream>
#include <sstream>
#include <mmsystem.h> // For sounds

using namespace std;

// <3
void clearScreen()
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}

// Fix to the clearScreen() bug
void hideConsoleCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.dwSize = 50; // Needs to be between 1 and 100 to work
    cursorInfo.bVisible = false; // Sets the cursor invisible
    SetConsoleCursorInfo(hOut, &cursorInfo);
}


// Ball directions
enum eBallDir { STOP = 0, LEFT = 1, UPLEFT = 2, DOWNLEFT = 3, RIGHT = 4, UPRIGHT = 5, DOWNRIGHT = 6 };

// Ball class
class cBall
{
private:
    int x, y;
    int startX, startY;
    eBallDir direction;

public:
    cBall(int xCoord, int yCoord)
    {
        startX = xCoord;
        startY = yCoord;
        x = xCoord;
        y = yCoord;
        direction = STOP;
    }

    void reset()
    {
        x = startX;
        y = startY;
        direction = STOP;
    }

    inline void changeDirection(eBallDir dir) { direction = dir; }

    inline eBallDir getDirection() { return direction; }

    inline int getX() { return x; }

    inline int getY() { return y; }

    // Used at the start of a round
    inline void randomDirection() { direction = (eBallDir)((rand() % 6) + 1); }

    void move()
    {
        switch (direction)
        {
        case STOP:
            break;
        case LEFT:
            x--;
            break;
        case UPLEFT:
            x--; y--;
            break;
        case DOWNLEFT:
            x--; y++;
            break;
        case RIGHT:
            x++;
            break;
        case UPRIGHT:
            x++; y--;
            break;
        case DOWNRIGHT:
            x++; y++;
            break;
        }
    }
};


// Paddle Class
class cPaddle
{
private:
    int x, y;
    int startX, startY;

public:
    cPaddle(int xCoord, int yCoord)
    {
        startX = xCoord;
        startY = yCoord;
        x = xCoord;
        y = yCoord;
    }

    void reset()
    {
        x = startX;
        y = startY;
    }

    inline int getX() { return x; }

    inline int getY() { return y; }

    inline void moveUp() { y--; }

    inline void moveDown() { y++; }
};


class cEngine
{
private:
    int width, heigth;
    int scoreL, scoreR;
    int frameCount; // For AI
    char upL, downL, upR, downR; // Controls
    bool gameStarted; // For statistics
    bool singlePlayerMode;
    bool quit;

    cBall* ball;
    cPaddle* player1;
    cPaddle* player2;

public:
    cEngine(int w, int h)
    {
        gameStarted = false;
        quit = false;
        width = w;
        heigth = h;
        scoreL = scoreR = 0;
        frameCount = 0;

        hideConsoleCursor();
        srand(time(NULL));
        rand(); // Look if interested ;) www.stackoverflow.com/questions/7866754/

        // Controls
        upL = 'w'; downL = 's';
        upR = 'i'; downR = 'k';

        ball = new cBall(width / 2, heigth / 2); // Ball centered
        player1 = new cPaddle(2, heigth / 2 - 2); // Paddle is 4 blocks heigth so (-4 / 2)
        player2 = new cPaddle(width - 3, heigth / 2 - 2);
    }

    // Destructor to prevent memory leak
    ~cEngine() { delete ball, player1, player2; }

    void scoreUp(cPaddle* player)
    {
        if (player == player1)
        {
            scoreL++;
        }
        else
        {
            scoreR++;
        }
        ball->reset();
        player1->reset();
        player2->reset();
    }

    void saveStats()
    {
        int statistics[4] = { 0, 0, 0, 0 };

        // Reading stats from file
        ifstream statsIn("statistics.txt");
        if (statsIn)
        {
            stringstream ss;
            ss << statsIn.rdbuf();

            string temp;
            int found;
            int foundCount = 0;

            // For every word
            while (!ss.eof())
            {
                ss >> temp;
                if (stringstream(temp) >> found) // If word is an integer
                {
                    statistics[foundCount] = found; // Adds it to the array
                    foundCount += 1;
                }
            }
        }
        statsIn.close();

        // Writing stats to file
        ofstream statsOut("statistics.txt");
        statsOut << "Games played: " << statistics[0] + gameStarted << endl;
        statsOut << "Overall score: " << statistics[1] + scoreL + scoreR << endl;
        statsOut << "Left paddle score: " << statistics[2] + scoreL << endl;
        statsOut << "Right paddle score: " << statistics[3] + scoreR;
        statsOut.close();
    }

    void loadConfig()
    {
        ifstream configIn("config.cfg");
        if (configIn)
        {
            stringstream ss;
            ss << configIn.rdbuf();

            string setting, value;

            // For every word
            while (!ss.eof())
            {
                ss >> setting;
                ss >> value;
                if (setting == "leftPaddleUp:")
                {
                    upL = value[0];
                }
                else if (setting == "leftPaddleDown:")
                {
                    downL = value[0];
                }
                else if (setting == "rightPaddleUp:")
                {
                    upR = value[0];
                }
                else if (setting == "rightPaddleDown:")
                {
                    downR = value[0];
                }
            }
        }
        else
        {
            ofstream configOut("config.cfg");
            configOut << "leftPaddleUp: w" << endl;
            configOut << "leftPaddleDown: s" << endl;
            configOut << "rightPaddleUp: i" << endl;
            configOut << "rightPaddleDown: k";
            configOut.close();
        }
        configIn.close();
    }

    void startMenu()
    {
        PlaySound(TEXT("assets\\soundtrack.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

        HANDLE hOut;
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << "    _/_/_/      _/_/    _/      _/    _/_/_/" << endl;
        cout << "   _/    _/  _/    _/  _/_/    _/  _/       " << endl;
        cout << "  _/_/_/    _/    _/  _/  _/  _/  _/  _/_/  " << endl;
        cout << " _/        _/    _/  _/    _/_/  _/    _/   " << endl;
        cout << "_/          _/_/    _/      _/    _/_/_/    " << endl;
        cout << endl << endl;

        SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << string(6, ' ') << "Choose the number of players:" << endl;
        cout << string(12, ' ') << "(1) One player" << endl;
        cout << string(12, ' ') << "(2) Two players" << endl;
        cout << string(12, ' ') << "(q) Quit" << endl;

        while (true)
        {
            if (_kbhit())
            {
                char currentKey = _getch();
                if (currentKey == '1')
                {
                    singlePlayerMode = true;
                    gameStarted = true;
                    break;
                }
                else if (currentKey == '2')
                {
                    singlePlayerMode = false;
                    gameStarted = true;
                    break;
                }
                else if (currentKey == 'q')
                {
                    quit = true;
                    break;
                }
            }
        }
        system("cls"); // Bugfix: Part of menu stays on screen
    }

    void draw()
    {
        HANDLE hOut;
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        clearScreen();

        // Drawing top wall
        SetConsoleTextAttribute(hOut, FOREGROUND_BLUE);
        for (int i = 0; i < width; i++)
        {
            cout << '\xB1';
        }
        SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << endl;

        for (int y = 0; y < heigth; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // Getting coordinates
                int ballX = ball->getX();
                int ballY = ball->getY();
                int player1X = player1->getX();
                int player1Y = player1->getY();
                int player2X = player2->getX();
                int player2Y = player2->getY();

                // Right and left walls
                if (x == 0 || x == width - 1)
                {
                    SetConsoleTextAttribute(hOut, FOREGROUND_BLUE);
                    cout << '\xB1';
                    SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                }
                // Ball
                else if (x == ballX && y == ballY)
                {
                    cout << 'O';
                }
                // Paddle 1
                else if (x == player1X && (y >= player1Y && y <= player1Y + 3))
                {
                    cout << '\xDB';
                }
                // Paddle 2
                else if (x == player2X && (y >= player2Y && y <= player2Y + 3))
                {
                    cout << '\xDB';
                }
                else
                {
                    cout << ' ';
                }
            }
            cout << endl;
        }

        // Drawing bottom wall
        SetConsoleTextAttribute(hOut, FOREGROUND_BLUE);
        for (int i = 0; i < width; i++)
        {
            cout << '\xB1';
        }
        SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << endl;
        cout << string(width / 2 - 7, ' ') << "SCORE: " << scoreL << " - " << scoreR << endl;

        // Controls (single- and multi- player)
        if (!singlePlayerMode)
        {
            cout << string(width / 2 - 9, ' ') << upL << '/' << downL << " & " << upR << '/' << downR << " to move" << endl;
        }
        else
        {
            cout << string(width / 2 - 6, ' ') << upL << '/' << downL << " to move" << endl;
        }

        cout << string(width / 2 - 8, ' ') << "PRESS Q TO QUIT" << endl;
    }

    void input()
    {
        ball->move();

        int ballX = ball->getX();
        int ballY = ball->getY();
        int player1X = player1->getX();
        int player1Y = player1->getY();
        int player2X = player2->getX();
        int player2Y = player2->getY();

        // On keyboard press
        if (_kbhit())
        {
            char currentKey = _getch();

            // Player1 controls
            if (currentKey == upL)
            {
                if (player1Y > 0) {
                    player1->moveUp();
                }
            }

            if (currentKey == downL)
            {
                if (player1Y < heigth - 4)
                {
                    player1->moveDown();
                }
            }

            // Player2 controls
            if (!singlePlayerMode) // Blocked on single-player mode
            {
                if (currentKey == upR)
                {
                    if (player2Y > 0)
                    {
                        player2->moveUp();
                    }
                }
                else if (currentKey == downR)
                {
                    if (player2Y < heigth - 4)
                    {
                        player2->moveDown();
                    }
                }
            }

            // Quit
            if (currentKey == 'q')
            {
                quit = true;
            }

            // Moves ball on input
            if (ball->getDirection() == STOP)
            {
                ball->randomDirection();
            }
        }
    }

    void logic()
    {
        int ballX = ball->getX();
        int ballY = ball->getY();
        int player1X = player1->getX();
        int player1Y = player1->getY();
        int player2X = player2->getX();
        int player2Y = player2->getY();

        // Single-player AI. Moves every 3 frames
        if (singlePlayerMode)
        {
            if (frameCount == 0)
            {
                if (player2Y + 3 < ballY)
                {
                    player2->moveDown();
                }
                else if (player2Y > ballY)
                {
                    player2->moveUp();
                }
            }
            frameCount++;
            if (frameCount == 2)
            {
                frameCount = 0;
            }
        }

        // If ball hits left paddle
        if (ballX == player1X + 1)
        {
            if (ballY >= player1Y && ballY <= player1Y + 3)
            {
                ball->changeDirection((eBallDir)(rand() % 3 + 4));
            }
        }
        // Right paddle
        if (ballX == player2X - 1)
        {
            if (ballY >= player2Y && ballY <= player2Y + 3)
            {
                ball->changeDirection((eBallDir)(rand() % 3 + 1));
            }
        }
        // Bottom wall
        if (ballY == heigth - 1)
        {
            ball->changeDirection(ball->getDirection() == DOWNRIGHT ? UPRIGHT : UPLEFT);
        }
        // Top wall
        else if (ballY == 0)
        {
            ball->changeDirection(ball->getDirection() == UPRIGHT ? DOWNRIGHT : DOWNLEFT);
        }
        // Left wall
        else if (ballX <= 0)
        {
            scoreUp(player2);
        }
        // Right wall
        else if (ballX >= width - 1)
        {
            scoreUp(player1);
        }
    }

    void run()
    {
        loadConfig();
        startMenu();
        while (!quit)
        {
            draw();
            input();
            logic();
        }
        saveStats();
    }
};


int main()
{
    cEngine pong(40, 20);
    pong.run();
    return 0;
}
