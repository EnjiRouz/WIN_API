#pragma once

void WaitForConnection(const LPCWSTR &pipeName);

void FileCreation(HANDLE &hMessageHistory, const LPCWSTR &pipeName);
