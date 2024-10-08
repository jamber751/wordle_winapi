#include <windows.h>
#include <iostream>
using namespace std;

void provideFeedback(const string &guess, const string &correctWord, char *feedback)
{
    for (size_t i = 0; i < guess.length(); i++)
    {
        if (guess[i] == correctWord[i])
        {
            feedback[i] = 'G';
        }
        else if (correctWord.find(guess[i]) != string::npos)
        {
            feedback[i] = 'Y';
        }
        else
        {
            feedback[i] = 'B';
        }
    }
    feedback[guess.length()] = '\0';
}

int main()
{
    HANDLE hNamedPipe;
    string correctWord;

    cout << "Enter your word (5-letter word): ";
    cin >> correctWord;

    while (correctWord.length() != 5)
    {
        cout << "Invalid input. Please enter a 5-letter word: ";
        cin >> correctWord;
    }

    hNamedPipe = CreateNamedPipe(
        "\\\\.\\pipe\\demo_pipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        1,
        128,
        128,
        INFINITE,
        NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        cerr << "Create named pipe failed." << endl
             << "The last error code: " << GetLastError() << endl;
        return 0;
    }

    cout << "The server is waiting for connection with a client." << endl;
    if (!ConnectNamedPipe(hNamedPipe, NULL))
    {
        cerr << "The connection failed." << endl
             << "The last error code: " << GetLastError() << endl;
        CloseHandle(hNamedPipe);
        return 0;
    }

    for (int i = 0; i < 6; i++)
    {
        char buffer[128];
        DWORD bytesRead;

        if (!ReadFile(hNamedPipe, buffer, sizeof(buffer), &bytesRead, NULL))
        {
            cerr << "Read file failed." << endl;
            break;
        }

        buffer[bytesRead] = '\0';
        string guess(buffer);
        cout << "Received guess: " << guess << endl;

        char feedback[6];
        provideFeedback(guess, correctWord, feedback);

        DWORD bytesWritten;
        if (!WriteFile(hNamedPipe, feedback, 6, &bytesWritten, NULL))
        {
            cerr << "Writing to the named pipe failed." << endl;
            break;
        }
        cout << "Written " << bytesWritten << " bytes of feedback." << endl;
    }

    CloseHandle(hNamedPipe);
    cout << "The data are read by the server." << endl;
    return 0;
}
