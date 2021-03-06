// Multi-threading, Lab4.cpp

/*Рассмотрим задачу читатель - писатель.У нас есть хранилище данных, ограниченного размера,
с которым работают несколько потоков.Потоки могут выполнять операции чтения и записи данных.
Есть потоки писатели и потоки читатели.
Функциональность потоков читателя и писателя можно записать следующим
образом.
Writer:
while (true) {
	PrepareData();  // Подготовить данные
							 // Подключаемся к базе данных
	Write(Data);   // Записать данные
						   //Увеличиваем счетчик записанной информации
	…// Отключаемся от базы
}

Reader:
while (true) {
	// Подключаемся к базе данных
	Read(&Data);   // Прочитать и удалить данные
							  //Уменьшаем счетчик записанной информации
							  // Отключаемся от базы
	UseData();        // Использовать данные
}

Потоки читатели могут работать с базой одновременно, т.е.несколько читателей могут считывать различную
информацию из базы данных.Потоки писатели работают только с полным доступом, если поток пишет в базу,
то больше никто в нее не пишет и не читает.При этом возникают различные проблемы синхронизации :
писатели могут писать в переполненную базу, читатели могут считывать из пустой базы,
нарушаться синхронизация работы читателей и писателей.

Задание : При помощи семафоров\мьютексов обеспечить согласованный доступ нескольких потоков к базе данных.
Корректное решение должно удовлетворять следующим условиям :
	потоки выполняются параллельно;
	размер базы данных меньше, чем могут записать туда все потоки писатели;
	потоки - писатели должны записать в базу определенное количество элементов, потоки - читатели должны считать из базы определенное количество элементов;
	во время выполнения потоком операции записи, база данными не использоваться другими потоками;
	потоки читатели удаляют ту информацию, которую читают в базе данных;
	во время выполнения потоком операции чтения, другие потоки читатели могут вставать в очередь для чтения;
	потоки должны завершить работу в течение конечного времени.

Методические указания :
•	Потоки писатели записывают в базу данных определенное количество порций данных, например 5. Если они пытаются записать в базу данных,
	но у них не получается(база полна), то попытка записи не засчитывается, считаются только успешные записи в базу.
•	Потоки писатели пишут в базу уникальную информацию, например, каждый пишет свой уникальный символ.
•	Поток читатели считывают из базы данных определенное количество порций данных, например 5. Засчитываются только успешные считывания.
•	База представляет собой обычный контейнер, можно массив, запись идет в конец, считывание идет с начала. (Например очередь).
	База данных имеет ограничение по количеству информации.В начале работы, база данных должна быть пустая.
•	Чтобы промоделировать нормальную работу, между операциями чтения / записи, открытия / закрытия и т.п.нужно вставить небольшую случайную паузу.
•	Если количество потоков читателей и писателей равны, и они должны записывать / считывать одинаковое количество информации, то после окончания работы всех потоков база данных должна быть пуста.
*/

#include "pch.h"
#include "Windows.h"
#include <iostream>	
#include <tchar.h>
#include <strsafe.h>
#include <queue>

using namespace std;

// мьютексы для чтения и записи
HANDLE hReaderMutex, hWriterMutex;

// база данных
queue<int> dataBase, readersQueue;
int dataBaseLength = 0;

// параметры, которые будут передаваться
// потоку-читателю
typedef struct {
	int readerThreadId;
} READER, *PREADER;

// параметры, которые будут передаваться
// потоку-писателю
typedef struct {
	int writerThreadId;
} WRITER, *PWRITER;

// поток-читатель
DWORD WINAPI Reader(LPVOID lpParam)
{
	int count=5;

	// приведение параметров к правильному типу данных
	PREADER data;
	data = (PREADER)lpParam;

	while(count!=0) 
	{
		Sleep(data->readerThreadId * 100);

		WaitForSingleObject(hReaderMutex, INFINITE);
		readersQueue.push(data->readerThreadId);
		ReleaseMutex(hReaderMutex);

		WaitForSingleObject(hWriterMutex, INFINITE);
		if (!dataBase.empty())
		{
			// считывание и удаление записи
			count--;
			dataBase.pop();
			cout << "Reading: " << data->readerThreadId << endl;

		}
		ReleaseMutex(hWriterMutex);

		WaitForSingleObject(hReaderMutex, INFINITE);
		readersQueue.pop();
		ReleaseMutex(hReaderMutex);
	}
	return 0;
}

// поток-писатель
DWORD WINAPI Writer(LPVOID lpParam)
{
	int count = 5;

	// приведение параметров к правильному типу данных
	PWRITER data;
	data = (PWRITER)lpParam;

	while(count!=0)
	{		
		Sleep(data->writerThreadId*100);
		WaitForSingleObject(hWriterMutex, INFINITE);

		WaitForSingleObject(hReaderMutex, INFINITE);
		if (!readersQueue.empty())
		{	
			ReleaseMutex(hReaderMutex);
			ReleaseMutex(hWriterMutex);
			continue;
		}
		ReleaseMutex(hReaderMutex);

		if (dataBase.size() == dataBaseLength)
		{
			ReleaseMutex(hWriterMutex);
			continue;
		}
		// произведение записи
		count--;
		dataBase.push(data->writerThreadId);
		cout << "Writing: " << data->writerThreadId<< endl;
		ReleaseMutex(hWriterMutex);
		Sleep(1000);
	}
	return 0;
}

// закрытие потоков-писятелей, освобождение памяти
void CleanUpWriters(HANDLE *hThread, int t, PWRITER  *toFree, int count)
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

// закрытие потоков-читателей
void CleanUpReaders(HANDLE *hThread, int t, PREADER  *toFree, int count)
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

// создание потоков-писателей
void CreateWriters(int & i, int writers, PWRITER &dataW, PWRITER * toFreeW, HANDLE * hWriterThread)
{
	// создание максимального количества потоков
	for (i = 0; i < writers; i++)
	{
		// выделение памяти для передачи данных потоку
		dataW = (PWRITER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			sizeof(WRITER));
		if (dataW == NULL)
			ExitProcess(2);

		// генерация данных для каждого потока
		dataW->writerThreadId = i;
		toFreeW[i] = dataW;

		// создание потоков в запущенном состоянии
		hWriterThread[i] = CreateThread(NULL, 0, Writer, dataW, 0, NULL);

		// закрытие потока, освобождение памяти и 
		// завершение работы процесса
		if (hWriterThread[i] == NULL)
		{
			CleanUpWriters(hWriterThread, writers, toFreeW, writers);
			ExitProcess(i);
		}
	}
}

// создание потоков-читателей
void CreateReaders(int & i, int readers, PREADER &dataR, PREADER * toFreeR, HANDLE * hReaderThread)
{
	// создание максимального количества потоков
	for (i = 0; i < readers; i++)
	{
		// выделение памяти для передачи данных потоку
		dataR = (PREADER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			sizeof(READER));
		if (dataR == NULL)
			ExitProcess(2);

		// генерация данных для каждого потока
		dataR->readerThreadId = i;
		toFreeR[i] = dataR;

		// создание потоков в запущенном состоянии
		hReaderThread[i] = CreateThread(NULL, 0, Reader, dataR, 0, NULL);

		// закрытие потока, освобождение памяти и 
		// завершение работы процесса
		if (hReaderThread[i] == NULL)
		{
			CleanUpReaders(hReaderThread, readers, toFreeR, readers);
			ExitProcess(i);
		}
	}
}

// создание, выполнение и завершение работы потоков
void ThreadsExecution(int writers, int readers, int &i)
{
	hReaderMutex = CreateMutex(NULL, FALSE, NULL);
	hWriterMutex = CreateMutex(NULL, FALSE, NULL);

	PREADER dataR = NULL;
	PWRITER dataW = NULL;

	DWORD *dwWriterThreadId = new DWORD[writers];
	HANDLE *hWriterThread = new HANDLE[writers];
	PWRITER *toFreeW = new PWRITER[writers];

	DWORD *dwReaderThreadId = new DWORD[readers];
	HANDLE *hReaderThread = new HANDLE[readers];
	PREADER *toFreeR = new PREADER[readers];

	CreateWriters(i, writers, dataW, toFreeW, hWriterThread);
	CreateReaders(i, readers, dataR, toFreeR, hReaderThread);

	// ожидание завершения выполнения потоков
	WaitForMultipleObjects(readers, hReaderThread, TRUE, INFINITE);
	WaitForMultipleObjects(writers, hWriterThread, TRUE, INFINITE);

	// закрытие потока, освобождение памяти
	CleanUpReaders(hReaderThread, readers, toFreeR, readers);
	CleanUpWriters(hWriterThread, writers, toFreeW, writers);

	// закрытие мьютекса, освобождение памяти
	CloseHandle(hReaderMutex);
	CloseHandle(hWriterMutex);

	delete[] dwReaderThreadId, dwWriterThreadId, hReaderThread, hWriterThread, toFreeR, toFreeW;
}

int main()
{
	int i, readers, writers;
	cout << "Enter the number of readers" << endl;
	cin >> readers;
	cout << "Enter the number of writers" << endl;
	cin >> writers;
	cout << "Enter the database size" << endl;
	cin >> dataBaseLength;

	// проверка на корректность введенных данных
	if (!cin || readers < 1 || writers < 1 || dataBaseLength < 1)
	{
		cerr << "Wrong data";
		return 1;
	}

	ThreadsExecution(writers, readers, i);

	// вывод размера итоговой бд
	cout << "Database size after reading/writing process: " << dataBase.size() << endl;

	system("pause");
	return 0;
}
