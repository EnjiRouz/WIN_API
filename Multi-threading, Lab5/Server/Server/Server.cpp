// Server

#include "pch.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include "Server.h"

using namespace std;

// переменные для потоков
HANDLE hSender, hReader;

// мьютексы для чтения и записи
HANDLE hSenderMutex, hReaderMutex;

// переменные для инициализации Named Pipe
char szInputBuffer[1024];
char szOutputBuffer[1024];
DWORD dwszInputBuffer = sizeof(szInputBuffer);
DWORD dwszOutputBuffer = sizeof(szOutputBuffer);

// ConnectNamedPipe
BOOL bConnectNamedPipe;

// FlushBuffer
BOOL bFlushFileBuffer;

// поток-отправитель
DWORD WINAPI SenderThread(LPVOID lpParam)
{
	HANDLE hNewNamedPipe;

	// переменные для WriteFile()
	BOOL bWriteFile;
	char szWriteFileBuffer[1024] = "Hello From NAMED PIPE SERVER!";
	DWORD dwWriteBufferSize = sizeof(szWriteFileBuffer);
	DWORD dwNoBytesWrite;

	LPCWSTR pipeName= L"\\\\.\\pipe\\serverClient"; 

	// создание и подключение к каналу
	PipeCreation(hNewNamedPipe, pipeName);
	ConnectToPipe(hNewNamedPipe);

	// цикл обмена данными с клиентским процессом
	while (true)
	{
		WaitForSingleObject(hSenderMutex, INFINITE);
		
		cout << ">";
		cin.getline(szWriteFileBuffer, 1024);

		if (cin.bad() || cin.fail() )
		{
			cin.clear(); 
			cin.ignore(1024, '\n'); 
		}

		// операция записи
		bWriteFile = WriteFile(hNewNamedPipe, szWriteFileBuffer, dwWriteBufferSize, &dwNoBytesWrite, NULL);
		if (bWriteFile == FALSE)
		{
			cout << "WriteFile Failed with Error No - " << GetLastError() << endl;
			break;
		}
		cout << "WriteFile Success" << endl;

		// сбрасывание File Buffer
		bFlushFileBuffer = FlushFileBuffers(hNewNamedPipe);
		if (bFlushFileBuffer == FALSE)
		{
			cout << "FlushFileBuffer Failed with Error No - " << GetLastError() << endl;
			break;
		}
		cout << "FlushFileBuffer Success" << endl;

		ReleaseMutex(hSenderMutex);
	}

	// разъединение
	DisconnectNamedPipe(hNewNamedPipe);
	CloseHandle(hNewNamedPipe); 

	return 0;
}

// поток-читатель
DWORD WINAPI ReaderThread(LPVOID lpParam)
{
	HANDLE hNewNamedPipe;

	// переменные для ReadFile()
	BOOL bReadFile;
	char szReadFileBuffer[1024];
	DWORD dwReadBufferSize = sizeof(szReadFileBuffer);
	DWORD dwNoBytesRead;

	LPCWSTR pipeName = L"\\\\.\\pipe\\clientServer"; 

	// создание и подключение к каналу
	PipeCreation(hNewNamedPipe, pipeName);
	ConnectToPipe(hNewNamedPipe);

	// цикл обмена данными с клиентским процессом
	while (true) 
	{
		WaitForSingleObject(hReaderMutex, INFINITE);

		cout << ">";

		// операция чтения
		bReadFile = ReadFile(hNewNamedPipe, szReadFileBuffer, dwReadBufferSize, &dwNoBytesRead, NULL);
		if (bReadFile == FALSE)
		{
			cout << "ReadFile Failed with Error No - " << GetLastError() << endl;
			break;
		}
		cout << "ReadFile Success" << endl;
		cout << "MESSAGE FROM CLIENT: " << szReadFileBuffer << endl;

		ReleaseMutex(hReaderMutex);
	}

	// разъединение
	DisconnectNamedPipe(hNewNamedPipe);
	CloseHandle(hNewNamedPipe);

	return 0;
}

// создание канала
void PipeCreation(HANDLE &hNewNamedPipe, const LPCWSTR &pipeName)
{
	hNewNamedPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
		dwszOutputBuffer, dwszInputBuffer, 0, NULL);
	if (hNewNamedPipe == INVALID_HANDLE_VALUE)
	{
		cout << "NamedPipe Creation Failed with Error No - " << GetLastError() << endl;
	}
	cout << "NamedPipe Creation Success" << endl;
}

// подключение к каналу
void ConnectToPipe(const HANDLE &hNewNamedPipe)
{
	bConnectNamedPipe = ConnectNamedPipe(hNewNamedPipe, NULL);
	if (bConnectNamedPipe == FALSE)
	{
		cout << "Connection Failed with Error No - " << GetLastError() << endl;
	}
	cout << "Connection Success" << endl;
}

int main()
{
	cout << "\t\t...NAMED PIPE SERVER..." << endl;

	hSenderMutex = CreateMutex(NULL, FALSE, NULL);
	hReaderMutex = CreateMutex(NULL, FALSE, NULL);

	hSender = CreateThread(NULL, 0, SenderThread, NULL, 0, NULL);
	hReader = CreateThread(NULL, 0, ReaderThread, NULL, 0, NULL);

	WaitForSingleObject(hSender, INFINITE);
	WaitForSingleObject(hReader, INFINITE);

	CloseHandle(hSenderMutex);
	CloseHandle(hReaderMutex);

	system("pause");
	return 0;
}
/*
1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112
*/