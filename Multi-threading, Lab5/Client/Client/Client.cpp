// Client

#include "pch.h"
#include "Windows.h"
#include <iostream>
#include <string>
#include "Client.h"

using namespace std;

// переменные для потоков
HANDLE hSender, hReader;

// мьютексы для чтения и записи
HANDLE hSenderMutex, hReaderMutex;

// поток-отправитель
DWORD WINAPI SenderThread(LPVOID lpParam)
{
	HANDLE hMessageHistory;

	// переменные для WriteFile()
	BOOL bWriteFile;
	char szWriteFileBuffer[1024] = "Hello From NAMED PIPE CLIENT!";
	DWORD dwWriteBufferSize = sizeof(szWriteFileBuffer);
	DWORD dwNoBytesWrite;

	LPCWSTR pipeName = L"\\\\.\\pipe\\clientServer"; 

	// ожидание подключения к каналу и
	// создание файла истории сообщений
	WaitForConnection(pipeName);
	FileCreation(hMessageHistory, pipeName);

	// цикл обмена данными с клиентским процессом
	while (true)
	{
		WaitForSingleObject(hSenderMutex, INFINITE);

		cout << ">";
		cin.getline(szWriteFileBuffer, 1024);

		if (cin.bad() || cin.fail())
		{
			cin.clear();
			cin.ignore(1024, '\n');
		}

		// операция записи
		bWriteFile = WriteFile(hMessageHistory, szWriteFileBuffer, dwWriteBufferSize, &dwNoBytesWrite, NULL);
		if (bWriteFile == FALSE)
		{
			cout << "WriteFile Failed with Error No - " << GetLastError() << endl;
			break;
		}
		cout << "WriteFile Success" << endl;

		ReleaseMutex(hSenderMutex);
	}

	// разъединение
	CloseHandle(hMessageHistory);

	return 0;
}

// поток-читатель
DWORD WINAPI ReaderThread(LPVOID lpParam)
{
	HANDLE hMessageHistory;

	// переменные для ReadFile()
	BOOL bReadFile;
	char szReadFileBuffer[1024];
	DWORD dwReadBufferSize = sizeof(szReadFileBuffer);
	DWORD dwNoBytesRead;

	LPCWSTR pipeName = L"\\\\.\\pipe\\serverClient"; 

	// ожидание подключения к каналу и
	// создание файла истории сообщений
	WaitForConnection(pipeName);
	FileCreation(hMessageHistory, pipeName);

	// цикл обмена данными с клиентским процессом
	while (true)
	{
		WaitForSingleObject(hReaderMutex, INFINITE);

		cout << ">";

		// операция чтения
		bReadFile = ReadFile(hMessageHistory, szReadFileBuffer, dwReadBufferSize, &dwNoBytesRead, NULL);
		if (bReadFile == FALSE)
		{
			cout << "ReadFile Failed with Error No - " << GetLastError() << endl;
			break;
		}
		cout << "ReadFile Success" << endl;
		cout << "MESSAGE FROM SERVER: " << szReadFileBuffer << endl;

		ReleaseMutex(hReaderMutex);
	}

	// разъединение
	CloseHandle(hMessageHistory);

	return 0;
}

// ожидание подключения к каналу
void WaitForConnection(const LPCWSTR &pipeName)
{
	BOOL bWait;
	bWait = WaitNamedPipe(pipeName, NMPWAIT_USE_DEFAULT_WAIT);
	if (bWait == FALSE)
	{
		cout << "WaitNamedPipe Failed with Error No - " << GetLastError() << endl;
	}
	cout << "WaitNamedPipe Success" << endl;
}

// создание файла истории сообщений
void FileCreation(HANDLE &hMessageHistory, const LPCWSTR &pipeName)
{
	hMessageHistory = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hMessageHistory == INVALID_HANDLE_VALUE)
	{
		cout << "File Creation Failed with Error No - " << GetLastError() << endl;
	}
	cout << "File Creation Success" << endl;
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