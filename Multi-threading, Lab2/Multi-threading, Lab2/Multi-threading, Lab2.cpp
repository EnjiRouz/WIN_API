/* Multi-threading, Lab2.cpp
1. Задание:
Написать многопоточную программу, которая считает сумму заданного ряда. На вход в программу подается количество потоков и количество членов ряда (или точность).
2. Рекомендации:
	Задавать количество потоков и членов ряда можно в самой программе
	Каждый поток считает свою часть ряда, и возвращает полученное значение первичному потоку. Первичный поток получает итоговую сумму.
Вариант 2 (1/(2^n))+(1/(3^n)) 
Решение ряда для быстрой проверки https://www.kontrolnaya-rabota.ru/s/ryad/summa/?infinity=1&nm=&function=%281%2F%282%5En%29%29%2B%281%2F%283%5En%29%29&N=n&n0=1
*/

#include "pch.h"
#include "Windows.h"
#include <iostream>	
#include <tchar.h>
#include <strsafe.h>
#include <cmath> // для работы pow()

// для безопасного вывода параметров из потока (для дебага)
#define BUF_SIZE 255

using namespace std;

// параметры, которые будут передаваться
// каждому потоку
typedef struct Data {
	int start;
	int n;
	int step;
	double *totalSum;
} DATA, *PDATA;

// вычисление членов ряда по формуле
double GetNthElement(int i)
{
	return(1 / pow(2, i)) + (1 / pow(3, i));
}

// закрытие потока, освобождение памяти
void CleanUp(HANDLE *hThread, int t, PDATA  *toFree, int count)
{
	for (int i = 0; i < t; i++)
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
	HANDLE hStdout;
	PDATA pData;
	pData = (PDATA)lpParam;

	// для вывода параметров (см.ниже, это для дебага)
	TCHAR msgBuf[BUF_SIZE];
	size_t cchStringSize;
	DWORD dwChars;

	// дескриптор активного экранного буфера консоли
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
		return 1;

	// вычисление суммы членов ряда в определенных параметрами пределах
	int i = pData->start;
	while (i <= pData->n)
	{
		*(pData->totalSum) += GetNthElement(i);
		i+=(pData->step);
	}

	// вывод параметров в "безопасном режиме" (для потока)
	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
		pData->start, i-=pData->step);
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	WriteConsole(hStdout, msgBuf, cchStringSize, &dwChars, NULL);

	return 0;
}

int main()
{
	// итоговая сумма
	double totalSum = 0;
	PDATA pData=NULL;
	int i;

	int n; // число членов ряда
	cout << "Enter a number of members of a row, please" << endl;
	cin >> n;
	
	int t; // число потоков
	cout << "Enter a number of threads, please" << endl;
	cin >> t;
	
	// проверка на корректность введенных данных
	if (!cin) {
		cerr << "Wrong data";
		return 1;
	}

	if (n > 0 && t > 0)
	{
		if (n < t) t = n;

		// создание динамических массивов 32-разрядных беззнаковых 
		// целых числел, дескрипторов, структур
		DWORD *dwThreadId = new DWORD[t];
		HANDLE *hThread = new HANDLE[t];	
		PDATA *toFree = new PDATA[t];

		// создание максимального количества потоков
		for (i = 0; i < t; i++)
		{
			// выделение памяти для передачи данных потоку
			pData = (PDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				sizeof(DATA));
			if (pData == NULL)
				ExitProcess(2);
			toFree[i] = pData;

			// генерация данных для каждого потока
			pData->start = i+1;
			pData->n = n;
			pData->step = t;
			pData->totalSum = &totalSum;
			
			// создание потоков в запущенном состоянии
			hThread[i] = CreateThread(NULL, 0, Thread, pData, 0, NULL);

			// закрытие потока, освобождение памяти и 
			// завершение работы процесса
			if (hThread[i] == NULL)
			{
				CleanUp(hThread, t, toFree, t);
				ExitProcess(i);
			}
		}
		// ожидание завершения выполнения потоков
		WaitForMultipleObjects(t, hThread, TRUE, INFINITE);
		
		// закрытие потока, освобождение памяти
		CleanUp(hThread,t,toFree,t);

		cout << "Sum of a row = " << totalSum << endl;

		delete[] dwThreadId;
		delete[] hThread;
		delete[] toFree;
	}
	system("pause");
	return 0;
}