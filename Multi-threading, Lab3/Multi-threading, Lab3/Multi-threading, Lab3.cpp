#include "pch.h"
#include "Windows.h"
#include <iostream>	
#include <tchar.h>
#include <strsafe.h>
#include <cstring>

using namespace std;

char str[1000] = { NULL };

int n; // число потоков
int *turn = NULL;
int *stage = NULL;

HANDLE hMutex;

// вход в критическую секцию
// Алгоритм Петтерсона для n процессов
// stage[process] - номер стадии, в которой находится процесс с идентификатором process
// turn[i] = process - идентификатор процесса process, зашедшего последним в стадию под номером i
void enterRegion(int process) 
{
	// перебор стадий
	for (int i = 1; i <= n-1; i++) 
	{
		stage[process] = i;
		turn[i] = process;
		// просмотр остальных процессов
		for (int j = 1; j <= n; j++) 
		{
			if (j == process)
				continue;
			if (stage[j] >= i && turn[i] == process)
			{
			//	cout << "Wait on thread: " << process << endl;
			}
			while (stage[j] >= i && turn[i] == process);
		}
	}
}

// выход из критической секции
void leaveRegion(int process) 
{
	stage[process] = 0;
}

// параметры, которые будут передаваться
// каждому потоку
typedef struct Data {
	int threadChar;
	int threadId;
} DATA, *PDATA;

// закрытие потока, освобождение памяти
void CleanUp(HANDLE *hThread, int n, PDATA  *toFree, int count)
{
	for (int i = 0; i < n; i++)
	{
		if (hThread[i] != NULL) CloseHandle(hThread[i]);
	}
	for (int i = 0; i < count; i++)
	{
		HeapFree(GetProcessHeap(), 0, toFree[i]);
	}
}

// поток
DWORD WINAPI Thread(LPVOID lpParam)
{
	// приведение параметров к правильному типу данных
	PDATA pData;
	pData = (PDATA)lpParam;

	// ожидание завершения выполнения мьютекса
	//WaitForSingleObject(hMutex, INFINITE);

	enterRegion(pData->threadId);

	// поток 10 раз записывает в строку свой уникальный символ
	char key[2] = { char(pData->threadChar), NULL };
	for (int i = 0; i < 10; i++)
	{
		strcat_s(str, key);
		
		Sleep(1000);

		// вывод промежуточной строки
		cout << "Thread result: " << str << endl;
	}

	// освобождение мьютекса
	//ReleaseMutex(hMutex);

	leaveRegion(pData->threadId);
	
	return 0;
}

int main()
{
	// создание мьютекса
	hMutex = CreateMutex(NULL, FALSE, NULL);

	PDATA pData = NULL;

	cout << "Enter a number of threads, please" << endl;
	cin >> n;

	// проверка на корректность введенных данных
	if (!cin||n<1) {
		cerr << "Wrong data";
		return 1;
	}

	// создание динамических массивов 32-разрядных беззнаковых 
	// целых числел, дескрипторов, структур
	DWORD *dwThreadId = new DWORD[n];
	HANDLE *hThread = new HANDLE[n];
	PDATA *toFree = new PDATA[n];

	turn = new int[n];
	stage = new int[n + 1];

	stage[n + 1] = 0;

	for (int i = 0; i < n; i++)
	{
		turn[i] = 0;
		stage[i] = 0;
	}

	// создание максимального количества потоков
	for (int i = 0; i < n; i++)
	{

		// выделение памяти для передачи данных потоку
		pData = (PDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			sizeof(DATA));
		if (pData == NULL)
			ExitProcess(2);
	
		// генерация данных для каждого потока
		pData->threadChar = (rand() % 26) + 'a';
		pData->threadId = i+1;
		toFree[i] = pData;

		// создание потоков в запущенном состоянии
		hThread[i] = CreateThread(NULL, 0, Thread, pData, 0, NULL);		
			   
		// закрытие потока, освобождение памяти и 
		// завершение работы процесса
		if (hThread[i] == NULL)
		{
			CleanUp(hThread, n, toFree, n);
			ExitProcess(i);
		}
	}

	// ожидание завершения выполнения потоков
	WaitForMultipleObjects(n, hThread, TRUE, INFINITE);
	
	// закрытие потока, освобождение памяти
	CleanUp(hThread, n, toFree, n);

	// закрытие мьютекса, освобождение памяти
	CloseHandle(hMutex);

	// вывод итоговой строки
	cout << "Final result: " << str << endl;

	delete[] dwThreadId,hThread,toFree,stage,turn;
	
	system("pause");
	return 0;
}