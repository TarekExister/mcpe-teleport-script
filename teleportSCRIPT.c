#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

typedef long long int qword;
typedef struct proc { int is_attached; int pid; HANDLE hproc; }proc;
void usage(proc p, DWORD64 ptr_base_address, int *offsets, int PointerLevel);
proc attach_proc();
qword get_mod(int pid);

int main(void)
{
	proc p = attach_proc();
	qword ptr_base = get_mod(p.pid) + 0x036A1FB0;
	int offsets[7] = { 0x0,0x3A8,0x88,0x0,0x138,0x8,0x498 };
	usage(p, ptr_base, &offsets, 7);
	return 0;
}

proc attach_proc() {
	proc _p = { 0,0,NULL };
	PROCESSENTRY32W pentry = { sizeof(PROCESSENTRY32W) };
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hsnap == INVALID_HANDLE_VALUE) return _p;
	if (Process32FirstW(hsnap, &pentry)) {
		do {
		if (!lstrcmpW(pentry.szExeFile, L"Minecraft.Windows.exe")) {
			_p.is_attached = 1;
			_p.hproc = OpenProcess(PROCESS_ALL_ACCESS, 0, pentry.th32ProcessID);
			_p.pid = pentry.th32ProcessID;
			CloseHandle(hsnap);
			return _p;
		}
		} while (Process32NextW(hsnap, &pentry));
	}
	CloseHandle(hsnap);
	return _p;
}

DWORD64 get_ptr_ad(proc p,DWORD64 ptr_base_address, int *offsets, int PointerLevel)
{
	DWORD64 address = ptr_base_address;
	DWORD64 temp;
	for (int x = 0; x < PointerLevel; x++){
		ReadProcessMemory(p.hproc, (LPCVOID)address, &temp, sizeof(temp), 0);
		address = temp + offsets[x];
	}
	return address;
}

void usage(proc p, DWORD64 ptr_base_address, int *offsets, int PointerLevel) {
	float x, y, z;
	int dc;
	typedef union coords{ float start[6]; unsigned char buffer[24]; }coords;
	coords c;
	DWORD64 address;
	printf("[values must be separated by space]\n");
	while (1) {
	printf("<x,y,z>: ");
	scanf("%f%f%f", &c.start[0], &c.start[1], &c.start[2]);
		if (p.is_attached) {
			address = get_ptr_ad(p, ptr_base_address, offsets, PointerLevel);
			c.start[3] = c.start[0] + 0.60f;
			c.start[4] = c.start[1] + 1.80f;
			c.start[5] = c.start[2] + 0.60f;
			WriteProcessMemory(p.hproc, (LPVOID)address, c.buffer, 24, 0);
		}
		else break;
		while ((dc = getchar()) != '\n');
	}
}
qword get_mod(int pid) {
	MODULEENTRY32W mod = { sizeof(MODULEENTRY32) }; 
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if (hsnap == INVALID_HANDLE_VALUE) return 0;
		while (Module32NextW(hsnap, &mod)){
			if (lstrcmp(mod.szModule,L"Minecraft.Windows.exe") == 0){
				CloseHandle(hsnap);
				return (qword)mod.modBaseAddr;
			}
		}
		return 0;
}