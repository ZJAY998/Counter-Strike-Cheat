#include <windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

HHOOK global_hook;
DWORD jump = 0;


// ���ⰴ������Ƭ��
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

// ��ת����,ִ�б����������ָ��Ƭ�Σ�����ת��ԭʼָ��ĺ������ִ��
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

// �ڴ˺�����DIY�����Ŀ������������ǵ���ת���������ڻ���͸�ӷ����
HRESULT __stdcall MyDrawIndexedPrimitive(LPDIRECT3DDEVICE9 m_pDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	HRESULT Result = S_FALSE;
	IDirect3DVertexBuffer9 *pStreamData = NULL;
	UINT iOffsetInBytes, iStride;
	if (m_pDevice->GetStreamSource(0, &pStreamData, &iOffsetInBytes, &iStride) == D3D_OK) pStreamData->Release();  // �õ�ģ����Դ
	if (iStride != Fvalue)            // ����Դ������Fvalueʱ������Ⱦ������ֱ��ȥ��
	{
		Result = Transfer_DrawIndexedPrimitive(m_pDevice, type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	}
	return Result;
}

// ͨ��Ӳ���뷽ʽ��ȡ��GetDrawIndexedPrimitive�����Ļ���ַ
ULONG_PTR GetDrawIndexedPrimitiveAddr()
{
	HANDLE handle = GetModuleHandle(TEXT("d3d9.dll"));      // ���d3d9.dllģ���ַ
	if (handle == INVALID_HANDLE_VALUE) return NULL;
	return(ULONG_PTR)handle + 0x5CD20;                      // ���ƫ��
}

// ��ʼHook
bool HookDrawIndexedPrimitive()
{
	ULONG_PTR address = GetDrawIndexedPrimitiveAddr();
	DWORD oldProtect = 0;
	if (VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &oldProtect))   // �����ڴ汣����ʽΪ�ɶ�д
	{
		DWORD value = (DWORD)MyDrawIndexedPrimitive - address - 5;                 // �������Ҫ��ת�ֽ�
		jump = address + 5;                                                        // ������һ����ת�ֽ�
		__asm
		{
			mov eax, address
			mov byte ptr[eax], 0xe9                                                 // ���Ϊ jmp
			add eax, 1                                                              // ָ�����
			mov ebx, value                                                          // ��ת
			mov dword ptr[eax], ebx                                                 // ��ֵ��ת��ַ(Զ��ת)
		}
		VirtualProtect((LPVOID)address, 5, oldProtect, &oldProtect);               // �ָ��ڴ汣����ʽ
	}
	return true;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (lParam = VK_HOME)     // �����Ϲ�꽫��������ģ�ͱ�ż�2
		{
			OutputDebugStringA("+1 \n");
			Fvalue = Fvalue+1;
		}
		if (lParam == VK_END)   // �����¹�����������������ģ�ͱ�ż�2
		{
			OutputDebugStringA("-1 \n");
			Fvalue = Fvalue-1;
		}
	}
	return CallWindowProc(Global_OldProc, hwnd, uMsg, wParam, lParam);  // ȫ���ȼ��ص�����
}


// �������ж��������Ϸ���������ע��
bool APIENTRY DllMain(HANDLE handle, DWORD dword, LPVOID lpvoid)
{
	HWND hwnd = FindWindowW(L"Valve001", NULL);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (GetCurrentProcessId() == pid)
	{
		HookDrawIndexedPrimitive();

		Global_OldProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)WindowProc);  // ע��ȫ���ȼ�
	}
	return true;
}