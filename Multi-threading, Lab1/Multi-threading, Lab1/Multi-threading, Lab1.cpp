/* Multi-threading, Lab1.cpp
1. Написать программу, которая будет запускать несколько потоков.Каждый поток должен проработать какое - то время, выдать информацию на экран.
2. Изменить программу таким образом, чтобы поток мог принимать и изменять аргумент, первичный поток передает аргумент, принимает его новое значение и распечатывает.
*/

#include "pch.h"
#include "Windows.h"
#include <iostream>	
#include <ctime>	// для функции clock

using namespace std;

DWORD WINAPI Thread_1(LPVOID lpParam)
{
	// запуск секундомера
	clock_t startTime = clock();

	int* numbers= (int*) lpParam;
	for (int j = 0; j < 10000; j++)
	{
		for (int i = 0; i < 50000; i++)
		{
			numbers[i]++;
		}
	}
	// расчет времени работы 1 потока с переводом в миллисекунды
	clock_t endTime = clock();
	cout << "Thread_1 operating time (ms): " << (double)(endTime - startTime) * 1000 / CLOCKS_PER_SEC << endl;
	return 0;
}

DWORD WINAPI Thread_2(LPVOID lpParam)
{
	// запуск секундомера
	clock_t startTime = clock();

	int* numbers = (int*)lpParam;
	for (int j = 0; j < 10000; j++)
	{
		for (int i = 50000; i < 100000; i++)
		{
			numbers[i]++;
		}
	}
	// расчет времени работы 2 потока с переводом в миллисекунды
	clock_t endTime = clock();
	cout << "Thread_2 operating time (ms): " << (double)(endTime - startTime) * 1000 / CLOCKS_PER_SEC << endl;
	return 0;
}


int main()
{
	// заполнение массива
	int numbers[100000] = { 0 };

	// запуск секундомера
	clock_t startTime = clock();

	// создание потоков
	// CREATE_SUSPENDED - система создает поток, 
	// инициализирует его и приостанавливает до последующих указаний
	// 0 - создаёт потом в запущнном состоянии
	HANDLE t1 = CreateThread(NULL, 0, Thread_1, (LPVOID)numbers, 0, NULL);
	HANDLE t2 = CreateThread(NULL, 0, Thread_2, (LPVOID)numbers, 0, NULL);

	// остановка выполнения потока до завершения работы того потока, Handle которого ей передали
	WaitForSingleObject(t1, INFINITE);
	WaitForSingleObject(t2, INFINITE);

	// расчет времени работы программы с переводом в миллисекунды
	clock_t endTime = clock();
	cout << "Operating time (ms): " << (double)(endTime-startTime)*1000 / CLOCKS_PER_SEC << endl;
	cout << "New value of the random element from numbers = "<<numbers[rand () % 99999+0]<<endl;
	cout << "New value of the random element from 1st half of numbers = " << numbers[rand() % 49999 + 0] << endl;
	cout << "New value of the random element from 2nd half of numbers = " << numbers[rand() % 99999 + 50000] << endl;

	system("pause");
	return 0;
}