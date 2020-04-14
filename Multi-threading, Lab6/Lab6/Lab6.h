void ConvertTimeToString(char  time[256], SYSTEMTIME &systemTime);
void DisplayAccessMask(ACCESS_MASK Mask, const HANDLE &hLogFileMutex, std::wstring &logBuffer, std::wstringstream &ws, const HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3]);
void LogEvent(const HANDLE &hLogFileMutex, std::wstring &logBuffer, std::wstringstream &ws, const HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3]);

void GetCreationTime(WIN32_FIND_DATA &wDataFound, char * time, std::wstringstream & ws, HANDLE & hLogFileMutex, std::wstring & logBuffer, HANDLE & hLogFile, DWORD & dwBytesWritten, char * newLine);

void SetCreationTime(const HANDLE &hFindFiles, WIN32_FIND_DATA &wDataFound);

void GetOwnerAndAccessInfo(const LPCWSTR &fullPath, AUTHZ_ACCESS_REPLY & AccessReply, BYTE * Buffer, HANDLE & hLogFileMutex, std::wstring & logBuffer, std::wstringstream & ws, HANDLE & hLogFile, DWORD & dwBytesWritten, char * newLine);

void RecursiveFileSearch(const LPCWSTR &path, std::wstringstream &ws, HANDLE &hLogFileMutex, std::wstring &logBuffer, HANDLE &hLogFile, DWORD &dwBytesWritten, char  newLine[3], char  time[256], AUTHZ_ACCESS_REPLY &AccessReply, BYTE  Buffer[1024]);
