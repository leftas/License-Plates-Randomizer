#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_
class FileManager
{
public:
	static void Initialize(const char* szFileName);
	static int ReadInteger(char* szSection, char* szKey, int iDefaultValue);
	static float ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue);
	static bool ReadBoolean(char* szSection, char* szKey, bool bolDefaultValue);
	static char* ReadString(char* szSection, char* szKey, const char* szDefaultValue);
	static bool WriteInteger(char* szSection, char* szKey, int iValue);
	static bool WriteFloat(const char* szSection, const char* szKey, float fltValue);
	static bool WriteBoolean(char* szSection, char* szKey, bool bValue);
	static bool WriteString(char* szSection, char* szKey, char* szValue);
	static char s_szFilePath[MAX_PATH];
};
#endif
