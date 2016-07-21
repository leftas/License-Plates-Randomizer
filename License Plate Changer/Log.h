#pragma once
#ifndef _LOG_H_
#define _LOG_H_
class Log
{
public:
	enum Type {
		Normal,
		Debug,
		Error,
		FatalError
	};
	static bool Log::Init(bool CreateConsole = false, bool DebugMode = false);
	static void Write(Type type, const char * format, ...);
private:
	static bool s_bConsole;
	static bool s_bDebugVersion;
	static bool s_bInitialized;
};
#endif