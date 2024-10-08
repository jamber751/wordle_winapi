#include <windows.h>
#include <conio.h>
#include <iostream>

using namespace std;

const WORD GREEN_BG = BACKGROUND_GREEN | FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;
const WORD YELLOW_BG = BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;
const WORD GREY_BG = BACKGROUND_INTENSITY | FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;

void setConsoleColor(HANDLE hConsole, WORD color, COORD position, DWORD length)
{
    DWORD written;
    FillConsoleOutputAttribute(hConsole, color, length, position, &written);
}

bool isGuessRight(string feedback)
{
    for (int i = 0; i < feedback.length(); i++)
    {
        if (feedback[i] != 'G')
        {
            return false;
        }
    }
    return true;
}

void printGuessFeedback(const string &guess, const string &feedback)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD cursorPosition = {0, 0};

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    cursorPosition = csbi.dwCursorPosition;

    for (size_t i = 0; i < guess.length(); ++i)
    {
        cout << "+---";
    }
    cout << "+" << endl;

    for (size_t i = 0; i < guess.length(); ++i)
    {
        cout << "| " << guess[i] << " ";

        COORD letterPos = {cursorPosition.X + 2 + static_cast<SHORT>(i * 4), cursorPosition.Y + 1};

        COORD blockStartPos = {cursorPosition.X + static_cast<SHORT>(i * 4) + 1, cursorPosition.Y + 1};
        switch (feedback[i])
        {
        case 'G':
            setConsoleColor(hConsole, GREEN_BG, blockStartPos, 3);
            break;
        case 'Y':
            setConsoleColor(hConsole, YELLOW_BG, blockStartPos, 3);
            break;
        case 'B':
            setConsoleColor(hConsole, GREY_BG, blockStartPos, 3);
            break;
        }
    }
    cout << "|" << endl;

    for (size_t i = 0; i < guess.length(); ++i)
    {
        cout << "+---";
    }
    cout << "+" << endl;
}

int main()
{
    HANDLE hNamedPipe;
    char pipeName[] = "\\\\.\\pipe\\demo_pipe";

    hNamedPipe = CreateFile(
        pipeName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        cerr << "Connection with the named pipe failed." << endl
             << "The last error code: " << GetLastError() << endl;
        return 0;
    }

    for (int i = 0; i < 6; i++)
    {
        string guess;
        cout << "Enter your guess (5-letter word): ";
        cin >> guess;

        while (guess.length() != 5)
        {
            cout << "Invalid input. Please enter a 5-letter word: ";
            cin >> guess;
        }

        DWORD bytesWritten;
        if (!WriteFile(hNamedPipe, guess.c_str(), guess.length(), &bytesWritten, NULL))
        {
            cerr << "Writing to the named pipe failed." << endl;
            break;
        }

        char buffer[128];
        DWORD bytesRead;
        if (!ReadFile(hNamedPipe, buffer, sizeof(buffer), &bytesRead, NULL))
        {
            cerr << "Reading feedback from the named pipe failed." << endl;
            break;
        }
        buffer[bytesRead] = '\0';

        string feedback(buffer);
        printGuessFeedback(guess, feedback);

        if (isGuessRight(feedback))
        {
            cout << "You won" << endl;
            _getch();
            return 0;
        }
    }

    CloseHandle(hNamedPipe);
    cout << "You lost" << endl;
    _getch();
    return 0;
}
