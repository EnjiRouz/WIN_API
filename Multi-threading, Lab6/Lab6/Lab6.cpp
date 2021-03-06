// Lab6.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//Задание: Написать приложение, которое при помощи системных вызовов выбранной операционой системы совершит рекурсивный обход заданного каталога.
//	Для каждого файла, найденного в каком - либо каталоге, нужно собрать следующую информацию : имя файла, тип файла, владелец файла и права доступа на файл.
//	Вся собранная информация записывается в какой - нибудь текстовый лог - файл, формат записи разработать самостоятельно.
//	Кроме того, при получении информации от файла, необходимо изменить время создания файла на текущее.
//
//	Методические указания.
//	1.	Работа с файлами и каталогами должна происходить при помощи системных вызовов выбранной операционной системы.
//	2.	Получение информации о файле, и управление атрибутами файла должно происходить при помощи  системных вызовов выбранной операционной системы.
//	3.	Для создания лог - файла и добавление к нему информации, желательно использовать  системные вызовы выбранной операционной системы.

// https://docs.microsoft.com/en-us/windows/desktop/fileio/file-management-functions
// https://docs.microsoft.com/ru-ru/windows/desktop/SecAuthZ/finding-the-owner-of-a-file-object-in-c--
// http://writeulearn.com/file-details-recursively-folder-directory-windows/
// https://www.tenouk.com/cpluscodesnippet/creategetfiletypesizehandle.html
// https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-shellexecuteexa
// http://www.cyberforum.ru/win-api/thread1175099.html
// https://forum.sources.ru/index.php?showtopic=220373
// http://www.cyberforum.ru/win-api/thread906826.html
// https://forum.sources.ru/index.php?showtopic=344104
// https://forum.sources.ru/index.php?showtopic=131622
// https://docs.microsoft.com/en-us/windows/desktop/api/aclapi/nf-aclapi-geteffectiverightsfromacla
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363362%28v=vs.85%29.aspx
// https://docs.microsoft.com/en-us/windows/desktop/wes/windows-event-log
// https://docs.microsoft.com/en-us/windows/desktop/wes/saving-events-to-a-log-file
// https://docs.microsoft.com/ru-ru/windows/desktop/EventLog/reporting-an-event
// http://www.cyberforum.ru/cpp-beginners/thread1250623.html


/* REMARKS
 * Not all file systems can record creation and last access time, and not all file systems record them in the same manner. 
 * For example, on the FAT file system, create time has a resolution of 10 milliseconds, write time has a resolution of 2 seconds,
 * and access time has a resolution of 1 day. On the NTFS file system, access time has a resolution of 1 hour.
*/

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <iomanip> 
#include <tchar.h>
#include "accctrl.h"
#include "aclapi.h"
#include <authz.h>
#include "Lab6.h"
#include "setupapi.h"
#include <sstream>

#pragma comment(lib,"setupapi.lib")

using namespace std;

// переменные для перевода времени файла в формат системного времени
SYSTEMTIME systemTime;
FILETIME fileTime;

// переменные для получения прав доступа на файл и имени владельца файла
PSID pOwnerSid;
PACL pDacl;
ACCESS_MASK accessRights;
TCHAR buf[1024];
TCHAR szName[MAX_PATH];
DWORD dwNameLen = MAX_PATH;
TCHAR szDomainName[MAX_PATH];
DWORD dwDomainNameLen = MAX_PATH;
SID_NAME_USE use;
TRUSTEE trustee = { NULL, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_NAME, TRUSTEE_IS_USER };

string space="                                                         ";

// перевод времени в строку в заданном формате
void ConvertTimeToString(char  time[256], SYSTEMTIME &systemTime)
{
	snprintf(time, 256, "%d-%02d-%02d %02d:%02d:%02d.%03d", systemTime.wYear, systemTime.wMonth, systemTime.wDay,
							 systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
}

// получение прав доступа на файл
void DisplayAccessMask(ACCESS_MASK Mask, const HANDLE &hLogFileMutex, std::wstring &logBuffer, std::wstringstream &ws, const HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3])
{
	wcout << "  Access Rights: ";
	ws << "   Access Rights: " << space.c_str() << space.c_str() << endl;
	LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	if (((Mask & GENERIC_ALL) == GENERIC_ALL) || ((Mask & FILE_ALL_ACCESS) == FILE_ALL_ACCESS))
	{
		wcout << "Full Control  ";
		ws << "   - Full Control  " << space.c_str()<< space.c_str() << endl;
		LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	}
	
	if (((Mask & GENERIC_READ) == GENERIC_READ) || ((Mask & FILE_GENERIC_READ) == FILE_GENERIC_READ))
	{
		wcout << "Read  ";
		ws << "   - Read  " << space.c_str() << space.c_str() << endl;
		LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	}
	
	if (((Mask & GENERIC_WRITE) == GENERIC_WRITE) || ((Mask & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE))
	{
		wcout << "Write  ";
		ws << "   - Write  " << space.c_str() << space.c_str() << endl;
		LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	}
	
	if (((Mask & GENERIC_EXECUTE) == GENERIC_EXECUTE) || ((Mask & FILE_GENERIC_EXECUTE) == FILE_GENERIC_EXECUTE))
	{
		wcout << "Execute  ";
		ws << "   - Execute  " << space.c_str() << space.c_str() << endl;
		LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	}	
}

// отправление данные в лог-файл
void LogEvent(const HANDLE &hLogFileMutex, std::wstring &logBuffer, std::wstringstream &ws, const HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3])
{
	wstringstream temp;
	WaitForSingleObject(hLogFileMutex, INFINITE);
	logBuffer = ws.str();
	string s(logBuffer.begin(), logBuffer.end());

	// продолжение записи в конец файла
	SetFilePointer(hLogFile, 0, NULL, FILE_END);
	WriteFile(hLogFile, s.c_str(), sizeof(ws)-100, &dwBytesWritten, NULL);

	// перевод строки
	WriteFile(hLogFile, newLine, strlen(newLine), &dwBytesWritten, 0);
	ReleaseMutex(hLogFileMutex);

	// отчисттка буффера
	ws.swap(temp);
}

// получение и вывод времени создания файла
void GetCreationTime(WIN32_FIND_DATA &wDataFound, char * time, std::wstringstream & ws, HANDLE & hLogFileMutex, std::wstring & logBuffer, HANDLE & hLogFile, DWORD & dwBytesWritten, char * newLine)
{
	// конвертация времени в системный формат
	// вывод имения файла, его тип (расширение), время создания ДО изменения 
	FileTimeToSystemTime(&wDataFound.ftCreationTime, &systemTime);
	ConvertTimeToString(time, systemTime);
	wcout << "* " << left << setw(30) << wDataFound.cFileName << "creation time: " << time << endl;
	ws << "* " << left << setw(30) << wDataFound.cFileName << "creation time: " << time << space.c_str() << endl;
	LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
}

// установка текущего времени файлу в качестве времени создания
void SetCreationTime(const HANDLE &hFindFiles, WIN32_FIND_DATA &wDataFound)
{
	// получение текущего времени системы
	// конвертация его в формат времени файла
	// установка текущего времени файлу в качестве времени создания
	GetSystemTime(&systemTime);
	SystemTimeToFileTime(&systemTime, &fileTime);
	SetFileTime(hFindFiles, &wDataFound.ftCreationTime, (LPFILETIME)NULL, &fileTime);
}

// получние информации о владельце файла и о правах доступа на файл
void GetOwnerAndAccessInfo(const LPCWSTR &fullPath, AUTHZ_ACCESS_REPLY & AccessReply, BYTE * Buffer, HANDLE & hLogFileMutex, std::wstring & logBuffer, std::wstringstream & ws, HANDLE & hLogFile, DWORD & dwBytesWritten, char * newLine)
{
	// получение имени владельца файла
	GetNamedSecurityInfo(fullPath, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, &pOwnerSid, NULL, &pDacl, NULL, NULL);
	LookupAccountSid(NULL, pOwnerSid, szName, &dwNameLen, szDomainName, &dwDomainNameLen, &use);

	// получение прав доступа на файл
	GetEffectiveRightsFromAcl(pDacl, &trustee, &accessRights);
	wsprintf(buf, TEXT("  Owner: %s\t\t\t\tCURRENT_USER access rights: 0x%08X"), szName, accessRights);

	// вывод информации о правах доступа на файл и о владельце
	AccessReply.GrantedAccessMask = (PACCESS_MASK)(Buffer);
	DisplayAccessMask(*(PACCESS_MASK)(AccessReply.GrantedAccessMask), hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
	wstring securityInfo(buf);
	wcout << "\n" << securityInfo << "\n" << endl;
	ws << "\n" << securityInfo << "\n" << space.c_str() << endl;
	LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);
}

// рекрусивный обход директории
void RecursiveFileSearch(const LPCWSTR &path, std::wstringstream &ws, HANDLE &hLogFileMutex, std::wstring &logBuffer, HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3], char  time[256], AUTHZ_ACCESS_REPLY &AccessReply, BYTE  Buffer[1024])
{
	// переменные для поиска файла и доступа к его атрибутам
	HANDLE hFindFiles;
	WIN32_FIND_DATA wDataFound;
	WIN32_FILE_ATTRIBUTE_DATA Info;

	// осуществление обхода директории
	hFindFiles = FindFirstFile(path, &wDataFound);
	if (hFindFiles != INVALID_HANDLE_VALUE)
	{
		do
		{
			// получение имени каталога, который находится внутри заданной директории
			if ((wDataFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (lstrcmp(wDataFound.cFileName, L".") != NULL) && (lstrcmp(wDataFound.cFileName, L"..") != NULL))
			{
				// получение ПОЛНОГО пути к файлу
				LPCWSTR fileName = (LPCWSTR)(wDataFound.cFileName);
				wstring temp = wstring(path);
				temp = temp.substr(0, temp.size() - 1);
				temp = temp + fileName+L"\\*";
				LPCWSTR fullPath = temp.c_str();
				RecursiveFileSearch(fullPath, ws, hLogFileMutex, logBuffer, hLogFile, dwBytesWritten, newLine, time, AccessReply, Buffer);
			}

			//получение информации о файле
			if (!(wDataFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wstring temp = wstring(path);
				temp = temp.substr(0, temp.size() - 1);

				wcout << "File in directory " << temp << " :" << endl;
				ws << "File in directory " << temp << " :" << space.c_str() << endl;
				LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);

				GetCreationTime(wDataFound, time, ws, hLogFileMutex, logBuffer, hLogFile, dwBytesWritten, newLine);
				SetCreationTime(hFindFiles, wDataFound);

				// вывод имения файла, его тип (расширение), время создания ПОСЛЕ изменения 
				ConvertTimeToString(time, systemTime);
				wcout << "* " << left << setw(30) << wDataFound.cFileName << "new creation time: " << time << endl;
				ws << "* " << left << setw(30) << wDataFound.cFileName << "new creation time: " << time << space.c_str() << endl;
				LogEvent(hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);

				// получение ПОЛНОГО пути к файлу
				LPCWSTR fileName = (LPCWSTR)(wDataFound.cFileName);
				temp=temp+fileName;
				LPCWSTR fullPath = temp.c_str();

				GetOwnerAndAccessInfo(fullPath, AccessReply, Buffer, hLogFileMutex, logBuffer, ws, hLogFile, dwBytesWritten, newLine);

				// перевод строки
				SetFilePointer(hLogFile, 0, NULL, FILE_END);
				WriteFile(hLogFile, newLine, strlen(newLine), &dwBytesWritten, 0);
			}

		} while (FindNextFile(hFindFiles, &wDataFound));
	}
	FindClose(hFindFiles);
}

int main()
{
	setlocale(LC_ALL, "");

	// директория, в которой будет осуществлен поиск файлов
	LPCWSTR path = L"C:\\Users\\EnjiRouz\\Desktop\\temp\\*";

	// переменная для конвертации времени в строку
	char time[256] = { NULL };

	// переменная для получения прав доступа
	BYTE Buffer[1024];
	AUTHZ_ACCESS_REPLY AccessReply = { 0 };

	// переменные для лог файла
	HANDLE hLogFile;
	HANDLE hLogFileMutex;
	wstring logBuffer;
	DWORD dwBytesWritten;
	wstringstream ws;
	
	//код перевода строки и возврата каретки
	char newLine[] = "\x0D\x0A";

	// создание лог-файла
	hLogFileMutex = CreateMutex(NULL, FALSE, NULL);
	hLogFile= CreateFile(TEXT("C:\\Users\\EnjiRouz\\Desktop\\temp\\my.log"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		// рекрусивный обход директории
		RecursiveFileSearch(path, ws, hLogFileMutex, logBuffer, hLogFile, dwBytesWritten, newLine, time, AccessReply, Buffer);
		CloseHandle(hLogFileMutex);
		CloseHandle(hLogFile);
	} else 
		{
			cout << "Error Create File Log" << endl;
		}
	
	system("pause");
	return 0;
}