// Hook DLL 文件

#include <windows.h>
HHOOK global_hook;

LRESULT CALLBACK MyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(global_hook, nCode, wParam, lParam);
}
extern "C" __declspec(dllexport) void SetHook()
{
	global_hook = SetWindowsHookEx(WH_CBT, MyProc, GetModuleHandle(TEXT("hook.dll")), 0);
}
extern "C" __declspec(dllexport) void UnHook()
{
	if (global_hook) UnhookWindowsHookEx(global_hook);
}

bool APIENTRY DllMain(HANDLE handle, DWORD dword, LPVOID lpvoid)
{
	HWND hwnd = FindWindowW(L"valve001", NULL);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (GetCurrentProcessId() == pid)
	{
		MessageBox(hwnd, TEXT("inject"), 0, 0);
	}
	return true;
}

// 加载程序

#include <windows.h>

int main()
{
	HMODULE hMod = LoadLibrary(TEXT("hook.dll"));
	typedef void(*pSetHook)(void);
	pSetHook SetHook = (pSetHook)GetProcAddress(hMod, "SetHook");
	SetHook();
	while (1)
	{
		Sleep(1000);
	}
	return 0;
}