#include "include.h"



char g_logFile[MAX_PATH];
bool Log::s_bInitialized;
bool Log::s_bConsole;


bool Log::Init(bool createConsole)
{
	FILE* file;
	memset(g_logFile, 0, sizeof(g_logFile));

	if (GetCurrentDirectoryA(sizeof(g_logFile), g_logFile))
	{
		strcat_s(g_logFile, "/LicensePlateChanger.log");
		if ((fopen_s(&file, g_logFile, "w")) == 0 && file != nullptr)
		{
			fprintf_s(file, "    License Plate Changer\n");
			fprintf_s(file, "       (C) 2015 Leftas   \n");
			fclose(file);
		}
		else
		{
			MessageBoxA(nullptr, "Failed to open VelocityLimitRemover.txt", "FATAL ERROR", MB_ICONERROR);
			return false;
		}
	}
	else
	{
		MessageBoxA(nullptr, "GetCurrentDirectory failed", "ERROR", MB_OK);
		return false;
	}
	if (createConsole) // doesn't work for GTA V
	{
		HWND handle = GetConsoleWindow();
		if (handle != nullptr)
		{
			ShowWindow(handle, SW_RESTORE);

			ShowWindow(handle, SW_MAXIMIZE);

			ShowWindow(handle, SW_SHOW);
		}
		else
		{
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
		}
		s_bConsole = true;
	}
	s_bInitialized = true;
	return true;
}

void Log::Write(Log::Type type, const char* format, ...)
{
	if(s_bInitialized != true)
	{
		return;
	}

	FILE* file;
	va_list message;
	char timestamp[25], logType[15], logBuffer[4096], logMessage[4096];
	struct tm *sTm;

	auto now = time(nullptr);
	sTm = localtime(&now);

	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", sTm);
	memset(logType, 0, sizeof(logType));
	switch (type)
	{
	case Log::Type::Normal:
		strcpy_s(logType, "Normal");
		break;
	case Log::Type::Debug:
		strcpy_s(logType, "Debug");
#ifndef _DEBUG
		return;
#endif // !DEBUG
		break;
	case Log::Type::Error:
		strcpy_s(logType, "Error");
		break;
	case Log::Type::FatalError:
		strcpy_s(logType, "Fatal Error");
		break;

	}

	va_start(message, format);
	_vsnprintf_s(logBuffer, sizeof(logBuffer), format, message);
	va_end(message);
	sprintf_s(logMessage, "[%s][%s]: %s", timestamp, logType, logBuffer);
	if ((fopen_s(&file, g_logFile, "a")) == 0 && file != nullptr)
	{
		fprintf_s(file, "%s \n", logMessage);
		fclose(file);
		if (Log::s_bConsole)
			printf_s("%s \n", logMessage);
		if (type == Log::Type::Error || type == Log::Type::FatalError)
		{
#pragma warning(suppress: 6054)
			MessageBoxA(nullptr, logMessage, logType, MB_ICONERROR);
			if (type == Log::Type::FatalError)
			{
				ExitProcess(0);
			}
		}
	}
}