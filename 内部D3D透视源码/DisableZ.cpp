#include <windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

HHOOK global_hook;
DWORD jump = 0;

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

// 在此函数中DIY你个项目
HRESULT __stdcall MyDrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	//  OutputDebugStringA("执行我自己的函数,中转函数\r\n");


	IDirect3DVertexBuffer9 *pStreamData = NULL;
	UINT iOffsetInBytes, iStride;
	if (m_pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK) pStreamData->Release();  // 得到模型来源
	//if (iStride == 4)            // 得到来源为4的时候，才会关闭Z轴（此处为敌人ID）
	//{
	//	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);     // 关闭Z轴缓冲
	//}
	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);     // 关闭Z轴缓冲
	return Transfer_DrawIndexedPrimitive(m_pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

ULONG_PTR GetDrawIndexedPrimitiveAddr()
{
	HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));      // 获得d3d9.dll模块基址
	if (handle == INVALID_HANDLE_VALUE) return NULL;
	return(ULONG_PTR)handle + 0x5CD20;                      // 相加偏移
}

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



bool APIENTRY DllMain(HANDLE handle, DWORD dword, LPVOID lpvoid)
{
	HWND hwnd = FindWindowW(L"Valve001", NULL);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (GetCurrentProcessId() == pid)
	{
		HookDrawIndexedPrimitive();
	}
	return true;
}