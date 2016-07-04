#include "Include.h"

char FileManager::s_szFilePath[MAX_PATH];

void FileManager::Initialize(const char* szFileName)
{
	memset(s_szFilePath, 0, sizeof(s_szFilePath));
	memcpy(s_szFilePath, szFileName, strlen(szFileName));
}

int FileManager::ReadInteger(char* szSection, char* szKey, int iDefaultValue)
{
	int iResult = GetPrivateProfileIntA(szSection, szKey, iDefaultValue, s_szFilePath);
	return iResult;
}

float FileManager::ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue)
{
	char szResult[256];
	char szDefault[256];
	float fltResult;
	sprintf(szDefault, "%f", fltDefaultValue);
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, sizeof(szResult), s_szFilePath);
	fltResult = atof(szResult);
	return fltResult;
}

bool FileManager::ReadBoolean(char* szSection, char* szKey, bool bDefaultValue)
{
	char szResult[5];
	char szDefault[5];
	bool bResult;
	sprintf(szDefault, "%s", bDefaultValue ? "True" : "False");
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, sizeof(szResult), s_szFilePath);
	bResult = (strcmp(szResult, "True") == 0 || strcmp(szResult, "true") == 0) ? true : false;
	return bResult;
}

char* FileManager::ReadString(char* szSection, char* szKey, const char* szDefaultValue)
{
	char* szResult = new char[256];
	memset(szResult, 0x00, 256);
	GetPrivateProfileStringA(szSection, szKey, szDefaultValue, szResult, sizeof(szResult), s_szFilePath);
	return szResult;
}

bool FileManager::WriteInteger(char* szSection, char* szKey, int iValue)
{
	char szValue[256];
	sprintf(szValue, "%d", iValue);
	return WritePrivateProfileStringA(szSection, szKey, szValue, s_szFilePath);
}

bool FileManager::WriteFloat(const char* szSection, const char* szKey, float fltValue)
{
	char szValue[256];
	sprintf(szValue, "%f", fltValue);
	return WritePrivateProfileStringA(szSection, szKey, szValue, s_szFilePath);
}

bool FileManager::WriteBoolean(char* szSection, char* szKey, bool bValue)
{
	char szValue[5];
	sprintf(szValue, "%s", bValue ? "True" : "False");
	return WritePrivateProfileStringA(szSection, szKey, szValue, s_szFilePath);
}

bool FileManager::WriteString(char* szSection, char* szKey, char* szValue)
{
	return WritePrivateProfileStringA(szSection, szKey, szValue, s_szFilePath);
}
