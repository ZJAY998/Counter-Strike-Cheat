#include <windows.h> 
#include<tchar.h> 
#include<d3d9.h>
#pragma comment( lib, "d3d9.lib") 

#define null NULL
#define RETURN return

LPDIRECT3D9             g_pD3D = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;

struct CUSTOMVERTEX
{

	float x, y, z, rhw;
	DWORD color;
};
#define FVF ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE ) 

HRESULT InitD3D(HWND hWnd)
{
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS   d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);
	return S_OK;
}

HRESULT InitVB()
{
	CUSTOMVERTEX v[] =
	{
		100, 000, 0, 1, 0xffff0000,
		300, 50, 0, 1, 0xff00ff00,
		500, 400, 0, 1, 0xff0000ff
	};

	g_pd3dDevice->CreateVertexBuffer(3 * sizeof(v), 0, FVF, D3DPOOL_DEFAULT, &g_pVB, 0);

	void* vb;
	g_pVB->Lock(0, 0, (void**)&vb, 0);
	memcpy(vb, v, sizeof(v));
	g_pVB->Unlock();
	return S_OK;
}
void Render()
{
	g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 0), 1, 0);

	g_pd3dDevice->BeginScene();
	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	g_pd3dDevice->SetFVF(FVF);
	//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 10);
	g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 4);
	g_pd3dDevice->EndScene();

	g_pd3dDevice->Present(0, 0, 0, 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	message == WM_CLOSE ? PostQuitMessage(0) : 0;
	return DefWindowProc(hWnd, message, wParam, lParam);
}
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, PTSTR, int)
{
	wchar_t cn[] = L"ClassName";
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = cn;
	RegisterClass(&wc);

	HWND hWnd = CreateWindow(cn, 0, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, SW_SHOW);

	InitD3D(hWnd);
	InitVB();
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	return 0;
}