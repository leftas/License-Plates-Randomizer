#include "Include.h"

#pragma comment(lib,"libMinHook.lib")

typedef void(__fastcall* tGenerateLicensePlate)(__int64 CCustomShaderEffectVehicle, __int64 numberForGeneratingHash);
tGenerateLicensePlate oGenerateLicensePlate = nullptr;

typedef void(__fastcall* tChangeLicensePlate)(__int64 CCustomShaderEffectVehicle, char* szLicensePlateText);
tChangeLicensePlate oChangeLicensePlate = nullptr;

DWORD64 g_addressToHook;
HINSTANCE g_hinstThisModule;
char* g_szLicensePlateFormatRegularVehicles,
	* g_szLicensePlateFormatSpecialVehicles = nullptr,
	  g_szCharachtersToExclude[36];
DWORD64 g_dwPreviousCVehicle;

void Revert();

bool IsCharAllowed(char chCharacterToCheck)
{
	for (int i = 0; i < 36 && g_szCharachtersToExclude != nullptr && g_szCharachtersToExclude[i] != '\0'; i++)
	{
		if (g_szCharachtersToExclude[i] == chCharacterToCheck)
		{
			return false;
		}
	}
	return true;
}

void hkGenerateLicensePlate(__int64 CCustomShaderEffectVehicle, int numberForGeneratingHash)
{
	CONTEXT* pCxtRegisters = new CONTEXT;
	DWORD64 CVehicle;

	memset(pCxtRegisters, 0, sizeof(pCxtRegisters));
	RtlCaptureContext(pCxtRegisters); // Yes, it's pretty nasty, but I have do this in a first place.

	CVehicle = pCxtRegisters->Rsi;
	delete pCxtRegisters;

	char szGeneratedPlate[16];
	char* format = g_szLicensePlateFormatRegularVehicles;
	DWORD64 dwTemp;
	size_t formatLength;
	bool bSameVehicle = CVehicle == g_dwPreviousCVehicle;

	g_dwPreviousCVehicle = CVehicle;

	srand(numberForGeneratingHash + time(nullptr));

	if (g_szLicensePlateFormatSpecialVehicles != nullptr && *reinterpret_cast<BYTE*>(CVehicle + 0x891) & 4) // Checking for emergency vehicles. And selecting format if user wanted specific format for them.
	{
		format = g_szLicensePlateFormatSpecialVehicles;
	}

	Log::Write(Log::Type::Debug, "Emergency vehicle address to the bit flag: %I64X Emergency Vehicle: %X ", CVehicle, *reinterpret_cast<byte*>(CVehicle + 0x83B));

	formatLength = strlen(format);

	for (auto i = 0; i < formatLength; i++)
	{
		do
		{
			switch (format[i])
			{
			case '?':
				szGeneratedPlate[i] = 65 + rand() % 26;
				break;
			case '#':
				szGeneratedPlate[i] = 48 + rand() % 10;
				break;
			default:
				szGeneratedPlate[i] = format[i];
				break;
			}

			if (g_szCharachtersToExclude == nullptr || szGeneratedPlate[i] == format[i]) // Breaking, because if the user wanted this format, we don't need to check if that is allowed char.
			{
				break; // nasty
			}

		} while (!IsCharAllowed(szGeneratedPlate[i]));

	}
	szGeneratedPlate[9] = '\0';
	Log::Write(Log::Type::Debug, "Generated license plate: %s", szGeneratedPlate);

	if (bSameVehicle) // Needed for emergency vehicles
	{
		dwTemp = *reinterpret_cast<DWORD64*>(CVehicle + 0x48); // CVehicleDrawHandler
		dwTemp = *reinterpret_cast<DWORD64*>(dwTemp + 0x30); // CCustomShaderEffectVehicle
		if (dwTemp != 0)
		{
			oChangeLicensePlate(dwTemp, szGeneratedPlate);
		}
	}

	return oChangeLicensePlate(CCustomShaderEffectVehicle, szGeneratedPlate);
}

bool IsSpace(char c)
{
	return (c == '\r' || c == '\t' || c == ' ' || c == '\n');
}

void SettingsFileInitialization()
{
	bool debugMode;
	INIReader* iniReader = new INIReader(".//LicensePlatesRandomizer.ini");
	std::string szTempStringHolder;
	g_szLicensePlateFormatRegularVehicles = new char[16];


	if (iniReader->ParseError() > 0)
	{
		MessageBox(nullptr, TEXT("Failed to open INI file."), TEXT("ERROR"), MB_OK | MB_ICONERROR);
	}
	strcpy(g_szLicensePlateFormatRegularVehicles, iniReader->Get("Main", "RegularVehiclesLicensePlateFormat", "??? ###").c_str());

	if (iniReader->GetBoolean("Main", "EnableSpecialVehiclesLicensePlateFormat", false))
	{
		g_szLicensePlateFormatSpecialVehicles = new char[16];
		strcpy(g_szLicensePlateFormatSpecialVehicles, iniReader->Get("Main", "SpecialVehiclesLicensePlateFormat", "########").c_str());
	}

	debugMode = iniReader->GetBoolean("Main", "Debug", false);

	if (debugMode || iniReader->GetBoolean("Main", "Logging", false))
	{
		Log::Init(false, debugMode);
	}

	if (iniReader->GetBoolean("Main", "Enabled", true) == false)
	{
		return Revert();
	}

	szTempStringHolder = iniReader->Get("Main", "ExcludeCharacters", "");

	if (!szTempStringHolder.empty() && szTempStringHolder != " ")
	{
		BYTE nCounter = 0;
		auto tempTokenzier = new char[szTempStringHolder.size() + 1];
		char* tempToken;
		memset(g_szCharachtersToExclude, 0, sizeof(g_szCharachtersToExclude));
		szTempStringHolder.erase(remove_if(szTempStringHolder.begin(), szTempStringHolder.end(), IsSpace), szTempStringHolder.end());
		strcpy(tempTokenzier, szTempStringHolder.c_str());

		tempToken = strtok(tempTokenzier, ",");
		while (tempToken != nullptr)
		{
			g_szCharachtersToExclude[nCounter++] = *tempToken;
			tempToken = strtok(nullptr, ",");
		}
	}
	delete iniReader;
}

void Revert()
{
	MH_DisableHook(reinterpret_cast<void*>(g_addressToHook));
	MH_Uninitialize();
	oChangeLicensePlate = nullptr;
	oGenerateLicensePlate = nullptr;

	if (g_szLicensePlateFormatRegularVehicles != nullptr)
	{
		delete[] g_szLicensePlateFormatRegularVehicles;
	}

	if (g_szLicensePlateFormatSpecialVehicles != nullptr)
	{
		delete[] g_szLicensePlateFormatSpecialVehicles;
	}

	FreeLibraryAndExitThread(g_hinstThisModule, 0);
}

void GetMainProcessModuleInfo(MODULEINFO& moduleInfo)
{
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &moduleInfo, sizeof(moduleInfo));

	Log::Write(Log::Type::Debug, "Base address: %I64X", moduleInfo.EntryPoint);
}

void MainFunction()
{
	MODULEINFO moduleInfo;

	DWORD64 dwChangeVehicleLicensePlateFunc,
			dwCustomShaderEffectVehicleInitInstruction;

	SettingsFileInitialization();

	GetMainProcessModuleInfo(moduleInfo);

	dwCustomShaderEffectVehicleInitInstruction = Pattern::Scan(moduleInfo, "41 8B D6 48 8B CB E8 ? ? ? ? 48 8B 03") + 3;

	if (dwCustomShaderEffectVehicleInitInstruction == 3) //
	{
		Log::Write(Log::Type::Error, "Failed to find a the CustomShaderEffectVehicle initialization function address");
		return Revert();
	}

	//*reinterpret_cast<DWORD*>(dwCustomShaderEffectVehicleInitInstruction) = 0xE8F18948; Probably was a bad idea, hahahha.

	g_addressToHook = *reinterpret_cast<DWORD*>(dwCustomShaderEffectVehicleInitInstruction + 4) + (dwCustomShaderEffectVehicleInitInstruction + 8);

	Log::Write(Log::Type::Debug, "License plate generator address: %I64X", g_addressToHook);

	if (g_addressToHook == NULL)
	{
		Log::Write(Log::Type::Error, "Failed to find a the license plate generator function address");
		return Revert();
	}

	if (MH_Initialize() != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to initialize MinHook library");
		return Revert();
	}

	Log::Write(Log::Type::Debug, "Successfully initialized MinHook");


	if (MH_CreateHook(reinterpret_cast<LPVOID>(g_addressToHook), hkGenerateLicensePlate, reinterpret_cast<void**>(&oGenerateLicensePlate)) != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to create the hook onto the 'GenerateLicensePlate' function ");
		return Revert();
	}

	Log::Write(Log::Type::Debug, "Successfully created the hook onto the 'GenerateLicensePlate' function");


	if (MH_EnableHook(reinterpret_cast<void*>(g_addressToHook)) != MH_OK)
	{
		Log::Write(Log::Type::Error, "Failed to hook onto the 'GenerateLicensePlate' function");
		return Revert();
	}

	Log::Write(Log::Type::Debug, "Successfully hooked onto the 'GenerateLicensePlate' function");

	dwChangeVehicleLicensePlateFunc = *reinterpret_cast<DWORD*>(g_addressToHook + 0x1FD);
	Log::Write(Log::Type::Debug, "License plate change function relative call address: %X", dwChangeVehicleLicensePlateFunc);
	dwChangeVehicleLicensePlateFunc += g_addressToHook + 0x201;
	Log::Write(Log::Type::Debug, "License plate change function absolute address: %I64X", dwChangeVehicleLicensePlateFunc);

	if (*reinterpret_cast<DWORD64*>(dwChangeVehicleLicensePlateFunc) != 0x74894808245C8948)
	{
		Log::Write(Log::Type::Error, "Failed to find 'ChangeVehicleLicensePlate' function");
		return Revert();
	}

	oChangeLicensePlate = reinterpret_cast<tChangeLicensePlate>(dwChangeVehicleLicensePlateFunc);
	Log::Write(Log::Type::Debug, "Successfully found 'ChangeVehicleLicensePlate' function");
}



bool WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
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