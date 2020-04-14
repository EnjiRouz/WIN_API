#pragma once

void ConnectToPipe(const HANDLE &hNewNamedPipe);

void PipeCreation(HANDLE &hNewNamedPipe, const LPCWSTR &pipeName);
