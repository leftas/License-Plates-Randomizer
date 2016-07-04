#include "Include.h"

#pragma comment(lib,"libMinHook.lib")

typedef void(__fastcall* tGenerateLicensePlate)(__int64 CVehicle, __int64 numberForGeneratingHash);
tGenerateLicensePlate oGenerateLicensePlate = nullptr;

typedef void(__fastcall* tChangeLicensePlate)(__int64 CVehicle, char* szLicensePlateText);
tChangeLicensePlate oChangeLicensePlate = nullptr;

DWORD64 g_addressToHook;
HINSTANCE g_hinstThisModule;
char* g_szLicensePlateFormat;

void hkGenerateLicensePlate(__int64 CVehicle, __int64 numberForGeneratingHash)
{	
	char szGeneratedPlate[9];

	srand(numberForGeneratingHash);

	Log::Write(Log::Type::Debug, "Number used for generation: %I64X %%I64d", numberForGeneratingHash, numberForGeneratingHash);

	for (auto i = 0; i < strlen(g_szLicensePlateFormat); i++)
	{
		switch (g_szLicensePlateFormat[i])
		{
		case '?':
			szGeneratedPlate[i] = 65 + rand() % 26;
			break;
		case '#':
			szGeneratedPlate[i] = 48 + rand() % 10;
			break;
		default:
			szGeneratedPlate[i] = g_szLicensePlateFormat[i];
			break;
		}
	}

	oChangeLicensePlate(CVehicle, szGeneratedPlate);
}

void SettingsFileInitialization()
{
	FileManager::Initialize(".//LicensePlateChanger.ini");
	
	if (GetFileAttributesA(FileManager::s_szFilePath) == INVALID_FILE_ATTRIBUTES) 
	{
		FileManager::WriteBoolean("Main", "Enabled", true);
		FileManager::WriteBoolean("Main", "Logging", false);
		FileManager::WriteString("Main", "LicensePlateFormat", "??? ###");
	}

	g_szLicensePlateFormat = FileManager::ReadString("Main", "LicensePlateFormat", "??? ###");
}

void Revert()
{
	MH_DisableHook(reinterpret_cast<void*>(g_addressToHook));
	MH_Uninitialize();
	oChangeLicensePlate = nullptr;
	oGenerateLicensePlate = nullptr;
}

void MainFunction()
{
	MODULEINFO moduleInfo;
	DWORD64 dwChangeVehicleLicensePlateFunc;
	
	SettingsFileInitialization();

	if(FileManager::ReadBoolean("Main","Logging", true))
	{
		Log::Init(false);
	}

	if(FileManager::ReadBoolean("Main", "Enabled", true) == false)
	{
		FreeLibraryAndExitThread(g_hinstThisModule, 0);
		return;
	}

	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &moduleInfo, sizeof(moduleInfo));

	Log::Write(Log::Type::Debug, "Base address: %I64X", moduleInfo.EntryPoint);

	g_addressToHook = Pattern::Scan(moduleInfo, "48 89 5C 24 ? 4C 89 74 24 ? 4C 89 7C 24 ?");
	
	Log::Write(Log::Type::Debug, "License plate generator address: %I64X", g_addressToHook);

	if(g_addressToHook == NULL)
	{
		Log::Write(Log::Type::Error, "Failed to find a the license plate generator function address");
		return;
	}

	if(MH_Initialize() != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to initialize MinHook library");
		return;
	}
	
	Log::Write(Log::Type::Debug, "Successfully initialized MinHook");


	if (MH_CreateHook(reinterpret_cast<LPVOID>(g_addressToHook), hkGenerateLicensePlate, reinterpret_cast<void**>(&oGenerateLicensePlate)) != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to create the hook onto the function ");
		return;
	}

	Log::Write(Log::Type::Debug, "Successfully created the hook onto the function");


	if(MH_EnableHook(reinterpret_cast<void*>(g_addressToHook)) != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to hook the function");
		return;
	}

	Log::Write(Log::Type::Debug, "Successfully hooked onto the function");

	dwChangeVehicleLicensePlateFunc = *reinterpret_cast<DWORD*>(g_addressToHook + 0x1FD);
	Log::Write(Log::Type::Debug, "License plate change function relative call address: %X", dwChangeVehicleLicensePlateFunc);
	dwChangeVehicleLicensePlateFunc += g_addressToHook + 0x201;
	Log::Write(Log::Type::Debug, "License plate change function abosulute address: %I64X", dwChangeVehicleLicensePlateFunc);

	if(*reinterpret_cast<DWORD64*>(dwChangeVehicleLicensePlateFunc) != 0x74894808245C8948)
	{
		Log::Write(Log::Type::Error, "Failed to find 'ChangeVehicleLicensePlate' function");
		Revert();
		return;
	}

	oChangeLicensePlate = reinterpret_cast<tChangeLicensePlate>(dwChangeVehicleLicensePlateFunc);
	Log::Write(Log::Type::Debug, "Successfully found 'ChangeVehicleLicensePlate' function");
}



bool WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstDLL);
		CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainFunction), nullptr, 0, nullptr);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Revert();
	}

	return true;
}