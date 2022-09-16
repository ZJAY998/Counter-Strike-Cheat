#include <windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

HHOOK global_hook;
DWORD jump = 0;


// 虚拟按键代码片段
WNDPROC Global_OldProc = NULL;
DWORD Fvalue = 5000;

LRESULT CALLBACK MyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(global_hook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void SetHook()
{
	global_hook = SetWindowsHookEx(WH_CBT, MyProc, GetModuleHandle(TEXT("hook.dll")), 0);
}

// 中转函数,执行被我们填充后的指令片段，并跳转到原始指令的后面继续执行
__declspec(naked) HRESULT __stdcall Transfer_DrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	__asm{
		mov edi, edi
			push ebp
			mov ebp, esp
			mov eax, jump
			jmp eax
	}
}

// 在此函数中DIY你的项目，这个就是我们的中转函数，用于绘制透视方框等
HRESULT __stdcall MyDrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	HRESULT Result = S_FALSE;
	IDirect3DVertexBuffer9 *pStreamData = NULL;
	UINT iOffsetInBytes, iStride;
	if (m_pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK) pStreamData->Release();  // 得到模型来源
	if (iStride != Fvalue)            // 当来源不等于Fvalue时，就渲染，否则直接去除
	{
		Result = Transfer_DrawIndexedPrimitive(m_pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	}
	return Result;
}

// 通过硬编码方式获取到GetDrawIndexedPrimitive函数的基地址
ULONG_PTR GetDrawIndexedPrimitiveAddr()
{
	HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));      // 获得d3d9.dll模块基址
	if (handle == INVALID_HANDLE_VALUE) return NULL;
	return(ULONG_PTR)handle + 0x5CD20;                      // 相加偏移
}

// 开始Hook
bool HookDrawIndexedPrimitive()
{
	ULONG_PTR address = GetDrawIndexedPrimitiveAddr();
	DWORD oldProtect = 0;
	if (VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect))   // 设置内存保护方式为可读写
	{
		DWORD value = (DWORD)MyDrawIndexedPrimitive - address - 5;                 // 计算出需要跳转字节
		jump = address + 5;                                                        // 计算下一个跳转字节
		__asm
		{
			mov eax, address
			mov byte ptr[eax], 0xe9                                                 // 填充为 jmp
			add eax, 1                                                              // 指针递增
			mov ebx, value                                                          // 中转
			mov dword ptr[eax], ebx                                                 // 赋值跳转地址(远跳转)
		}
		VirtualProtect((LPVOID)address, 5, oldProtect, &oldProtect);               // 恢复内存保护方式
	}
	return true;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (lParam = VK_HOME)     // 按下上光标将，我们让模型编号加2
		{
			OutputDebugStringA("+1 \n");
			Fvalue = Fvalue+1;
		}
		if (lParam == VK_END)   // 按下下光标键，我们让我们让模型编号减2
		{
			OutputDebugStringA("-1 \n");
			Fvalue = Fvalue-1;
		}
	}
	return CallWindowProc(Global_OldProc, hwnd, uMsg, wParam, lParam);  // 全局热键回调函数
}


// 主函数判断如果是游戏标题则进行注入
bool APIENTRY DllMain(HANDLE handle, DWORD dword, LPVOID lpvoid)
{
	HWND hwnd = FindWindowW(L"Valve001", NULL);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (GetCurrentProcessId() == pid)
	{
		HookDrawIndexedPrimitive();

		Global_OldProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)WindowProc);  // 注册全局热键
	}
	return true;
}