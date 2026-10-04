#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

int main() {
	DWORD pID = 0;
	uintptr_t module_base = 0;
	HANDLE hSnap_proc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 pe{};
	MODULEENTRY32 me{};
	pe.dwSize = sizeof(PROCESSENTRY32);
	me.dwSize = sizeof(MODULEENTRY32);

	if (hSnap_proc == NULL) {
		MessageBoxA(NULL, "Failed to update dependencies. Please reinstall the program or try again after a system reboot.", "Warning", MB_ICONEXCLAMATION);
		return 0;
	}

	Process32First(hSnap_proc, &pe);
	do {
		if (wcscmp(pe.szExeFile, L"gta_sa.exe") == 0) {
			pID = pe.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnap_proc, &pe));

	if (pID == 0) {
		MessageBoxA(NULL, "Software update failed - please install the new version manually at https://amd.com", "Warning", MB_ICONEXCLAMATION);
		return 0;
	}

	HANDLE hSnap_mod = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
	Module32First(hSnap_mod, &me);
	do {
		if (wcscmp(me.szModule, L"samp.dll") == 0) {
			module_base = (uintptr_t)me.modBaseAddr;
			break;
		}
	} while (Module32Next(hSnap_mod, &me));

	CloseHandle(hSnap_proc);
	CloseHandle(hSnap_mod);

	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, NULL, pID);
	if (proc == NULL) {
		MessageBoxA(NULL, "The program has closed after a critical error has occurred. (0xDEADBEEF)", "Error", MB_ICONEXCLAMATION);
		return 0;
	}

	DWORD SAMP_INFO;
	DWORD SAMP_SETTINGS;

	float distance;
	unsigned char no_nametags_behind_walls, show_nametags;

	uintptr_t info_off = module_base + 0x21A0F8;

	BOOL ok = ReadProcessMemory(proc, (LPCVOID)info_off, &SAMP_INFO, sizeof(SAMP_INFO), NULL);
	if (!ok) {
		MessageBoxA(NULL, "The program has closed after a critical error has occurred. (0xDADFCAA4)", "Error", MB_ICONEXCLAMATION);
		return 0;
	}

	uintptr_t sett_off = SAMP_INFO + 0x3C5;

	ReadProcessMemory(proc, (LPCVOID)sett_off, &SAMP_SETTINGS, sizeof(SAMP_SETTINGS), NULL);

	uintptr_t fNameTagsDistance = SAMP_SETTINGS + 0x27;
	uintptr_t byteNoNametagsBehindWalls = SAMP_SETTINGS + 0x2F;
	uintptr_t byteShowNameTags = SAMP_SETTINGS + 0x38;

	ReadProcessMemory(proc, (LPCVOID)byteNoNametagsBehindWalls, &no_nametags_behind_walls, sizeof(no_nametags_behind_walls), NULL);
	if (no_nametags_behind_walls == 0) { // well, it'll set it to my values... therefore, i can make it open to run/stop
		no_nametags_behind_walls = 1;
		show_nametags = 1; distance = 80.f;

		WriteProcessMemory(proc, (LPVOID)byteNoNametagsBehindWalls, &no_nametags_behind_walls, sizeof(no_nametags_behind_walls), NULL);
		WriteProcessMemory(proc, (LPVOID)fNameTagsDistance, &distance, sizeof(distance), NULL);
		WriteProcessMemory(proc, (LPVOID)byteShowNameTags, &show_nametags, sizeof(show_nametags), NULL);

		MessageBoxA(NULL, "Successfully disabled custom AMD processing", "Success", MB_ICONEXCLAMATION);
		return 0;
	}
	else {
		no_nametags_behind_walls = 0;
		show_nametags = 1; distance = 2000.f;

		WriteProcessMemory(proc, (LPVOID)byteNoNametagsBehindWalls, &no_nametags_behind_walls, sizeof(no_nametags_behind_walls), NULL);
		WriteProcessMemory(proc, (LPVOID)fNameTagsDistance, &distance, sizeof(distance), NULL);
		WriteProcessMemory(proc, (LPVOID)byteShowNameTags, &show_nametags, sizeof(show_nametags), NULL);

		MessageBoxA(NULL, "Successfully updated AMD dependencies", "Success", MB_OK);
		return 0;
	}

	return 0;
}
