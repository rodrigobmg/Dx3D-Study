#include <windows.h>
#include <string>
#include <fstream>
#include <d3d9.h>
#include <d3dx9.h>
#include "../math/Math.h"
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )

#include <vector>
#include <map>

using namespace std;

LPDIRECT3DDEVICE9	g_pDevice = NULL;
const int WINSIZE_X = 1024;		//�ʱ� ������ ���� ũ��
const int WINSIZE_Y = 768;	//�ʱ� ������ ���� ũ��
const int WINPOS_X = 0; //�ʱ� ������ ���� ��ġ X
const int WINPOS_Y = 0; //�ʱ� ������ ���� ��ġ Y
D3DLIGHT9 g_Light;
D3DMATERIAL9 g_Mtrl;

LPD3DXMESH g_Mesh;
POINT g_CurPos;
bool g_LButtonDown = false;
Matrix44 g_LocalTm;

// animation
bool g_ani = false;
int g_aniVtxIdx = -1;
Vector3 g_aniSrcPos;
Vector3 g_aniDestPos;
float g_aniAlpha = 0; // 0~1


// ���ؽ� ����ü
struct Vertex
{
	Vertex() {}
	Vertex(float x0, float y0, float z0) : p(Vector3(x0, y0, z0))
	{}
	Vector3 p;
	Vector3 n;
	static const DWORD FVF;
};
//���ؽ� ����ü ����.
const DWORD Vertex::FVF  = D3DFVF_XYZ | D3DFVF_NORMAL;



// �ݹ� ���ν��� �Լ� ������ Ÿ��
LRESULT CALLBACK WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
bool InitDirectX(HWND hWnd);
bool InitVertexBuffer();
void Render(int timeDelta);
bool ReadModelFile( const string &fileName, LPDIRECT3DVERTEXBUFFER9 &vtxBuff, int &vtxSize,  LPDIRECT3DINDEXBUFFER9 &idxBuff, int &faceSize );
void Spread();


int APIENTRY WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, 
	int nCmdShow)
{
	wchar_t className[32] = L"Sample";
	wchar_t windowName[32] = L"Sample";

	//������ Ŭ���� ���� ����
	//���� �̷��� ������ ����ڴ� ��� ����
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;			//�����쿡�� ����ϴ� ������ �޸𸮼���( �׳� 0 �̴�  �Ű澲������ )
	WndClass.cbWndExtra = 0;			//�����쿡�� ����ϴ� ������ �޸𸮼���( �׳� 0 �̴�  �Ű澲������ )
	WndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);		//������ ������
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );			//�������� Ŀ����� ����
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );		//����������ܸ�� ����
	WndClass.hInstance = hInstance;				//���α׷��ν��Ͻ��ڵ� 
	WndClass.lpfnWndProc = (WNDPROC)WndProc;			//������ ���ν��� �Լ� ������
	WndClass.lpszMenuName = NULL;						//�޴��̸� ������ NULL
	WndClass.lpszClassName = className;				//���� �ۼ��ϰ� �ִ� ������ Ŭ������ �̸�
	WndClass.style	 = CS_HREDRAW | CS_VREDRAW;	//������ �׸��� ��� ���� ( ����� ����ɶ� ȭ�鰻�� CS_HREDRAW | CS_VREDRAW )

	//������ �ۼ��� ������ Ŭ�������� ���
	RegisterClass( &WndClass );

	//������ ����
	//������ ������ �ڵ��� �������� g_hWnd �� �޴´�.
	HWND hWnd = CreateWindow(
		className,				//�����Ǵ� �������� Ŭ�����̸�
		windowName,				//������ Ÿ��Ʋ�ٿ� ��µǴ� �̸�
		WS_OVERLAPPEDWINDOW,	//������ ��Ÿ�� WS_OVERLAPPEDWINDOW
		WINPOS_X,				//������ ���� ��ġ X 
		WINPOS_Y,				//������ ���� ��ġ Y
		WINSIZE_X,				//������ ���� ũ�� ( �۾������� ũ�Ⱑ �ƴ� )
		WINSIZE_Y,				//������ ���� ũ�� ( �۾������� ũ�Ⱑ �ƴ� )
		GetDesktopWindow(),		//�θ� ������ �ڵ� ( ���α׷����� �ֻ��� ������� NULL �Ǵ� GetDesktopWindow() )
		NULL,					//�޴� ID ( �ڽ��� ��Ʈ�� ��ü�� �������ΰ�� ��Ʈ�� ID �� ��	
		hInstance,				//�� �����찡 ���� ���α׷� �ν��Ͻ� �ڵ�
		NULL					//�߰� ���� NULL ( �Ű���� )
		);

	//�����츦 ��Ȯ�� �۾����� ũ��� �����
	RECT rcClient = { 0, 0, WINSIZE_X, WINSIZE_Y };
	AdjustWindowRect( &rcClient, WS_OVERLAPPEDWINDOW, FALSE );	//rcClient ũ�⸦ �۾� �������� �� ������ ũ�⸦ rcClient �� ���ԵǾ� ���´�.

	//������ ũ��� ������ ��ġ�� �ٲپ��ش�.
	SetWindowPos( hWnd, NULL, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, 
		SWP_NOZORDER | SWP_NOMOVE );

	if (!InitDirectX(hWnd))
	{
		return 0;
	}

	InitVertexBuffer();
	ShowWindow( hWnd, nCmdShow );


	//�޽��� ����ü
	MSG msg;		
	ZeroMemory( &msg, sizeof( MSG ) );

	int oldT = GetTickCount();
	while (msg.message != WM_QUIT)
	{
		//PeekMessage �� �޽��� ť�� �޽����� ��� ���α׷��� ���߱� �ʰ� ������ �ȴ�.
		//�̶� �޽���ť�� �޽����� ������ false �� ���ϵǰ� �޽����� ������ true �� �����̵ȴ�.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage( &msg ); //���� Ű���� �� ���ڸ� �����Ͽ� WM_CHAR �޽����� �߻���Ų��.
			DispatchMessage( &msg );  //�޾ƿ� �޽��� ������ ������ ���ν��� �Լ��� �����Ų��.
		}
		else
		{
			const int curT = GetTickCount();
			const int elapseT = curT - oldT;
			if (elapseT > 15)
			{
				oldT = curT;
				Render(elapseT);
			}
		}
	}

	if (g_Mesh)
		g_Mesh->Release();
	if (g_pDevice)
		g_pDevice->Release();
	return 0;
}


//
// ������ ���ν��� �Լ� ( �޽��� ť���� �޾ƿ� �޽����� ó���Ѵ� )
//
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hWnd);
		if (wParam == VK_TAB)
		{
			static bool flag = true;
			g_pDevice->SetRenderState(D3DRS_CULLMODE, flag? D3DCULL_CCW : D3DCULL_NONE);
			g_pDevice->SetRenderState(D3DRS_FILLMODE, flag? D3DFILL_SOLID : D3DFILL_WIREFRAME);
			flag = !flag;
		}
		if (wParam == VK_SPACE)
		{
			Spread();
		}
		break;

	case WM_LBUTTONDOWN:
		{
			g_LButtonDown = true;
			g_CurPos.x = LOWORD(lParam);
			g_CurPos.y = HIWORD(lParam);
		}
		break;

	case WM_LBUTTONUP:
		g_LButtonDown = false;
		break;

	case WM_MOUSEMOVE:
		if (g_LButtonDown)
		{
			POINT pos;
			pos.x = LOWORD(lParam);
			pos.y = HIWORD(lParam);

			const int x = pos.x - g_CurPos.x;
			const int y = pos.y - g_CurPos.y;

			Matrix44 mat1;
			mat1.SetRotationY( -x * 0.01f );
			Matrix44 mat2;
			mat2.SetRotationX( -y * 0.01f );

			g_CurPos = pos;

			g_LocalTm *= (mat1 * mat2);
		}
		break;

	case WM_DESTROY: //�����찡 �ı��ȴٸ�..
		PostQuitMessage(0);	//���α׷� ���� ��û ( �޽��� ������ ���������� �ȴ� )
		break;
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}


bool InitDirectX(HWND hWnd)
{
	LPDIRECT3D9 d3d9;
	d3d9 = Direct3DCreate9( D3D_SDK_VERSION );

	// �ϵ���� ������ ������ �ͼ� �ڽ��� ���� ���μ��� Ÿ���� ������
	D3DCAPS9 caps;

	//Direct3D9 ��ü ���� ���� ī���� �ϵ���� ������ ������ �´�.
	d3d9->GetDeviceCaps(
		D3DADAPTER_DEFAULT,			//�� ���÷��� �׷��� ī�� �׳� D3DADAPTER_DEFAULT
		D3DDEVTYPE_HAL,				//����̽�Ÿ�� ���� �׳� D3DDEVTYPE_HAL
		&caps						//����̽� ������ �޾ƿ� D3DCAPS9 ������
		);

	//������� ó������� ������ �÷��� ��
	int vertexProcessing = 0;

	//���� ��ġ�� ���� ���� �ϵ���� ����� �����Ѱ�
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;


	//3. D3DPRESENT_PARAMETERS ����ü ������ ����
	//���� �̷��� Device �� ����ٴٶ�� ����

	D3DPRESENT_PARAMETERS d3dpp;		
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;				//���� ���ۿ� ���׽� ���� ũ�� ���� 24bit ���ٽ� ���� 8 ��Ʈ
	d3dpp.BackBufferCount = 1;						//����� ���� �׳� 1��
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;			//����� ���� ���� ����8��Ʈ,����8��Ʈ,�׸�8��Ʈ,����8��Ʈ
	d3dpp.BackBufferHeight = WINSIZE_Y;				//����� �ȼ� ũ��
	d3dpp.BackBufferWidth = WINSIZE_X;				//����� �ȼ� ũ��
	d3dpp.EnableAutoDepthStencil = true;						//�ڵ� ���̹��� ��� ���� ( �׳� true )
	d3dpp.Flags = 0;						//�߱� �÷��� ( �ϴ� 0 )
	d3dpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;	//ȭ�� �ֻ��� ( �׳� D3DPRESENT_RATE_DEFAULT ����� �ֻ����� ���Ͻ� )
	d3dpp.hDeviceWindow = hWnd;					//Device �� ��µ� ������ �ڵ�
	d3dpp.MultiSampleQuality = 0;						//��Ƽ ���ø� ��
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;		//��Ƽ ���ø� Ÿ�� 
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;	//ȭ�� ���� ���� ( �׳� D3DPRESENT_INTERVAL_ONE ����� �ֻ����� ���Ͻ� )
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	//ȭ�� ���� ü�� ���
	d3dpp.Windowed = true;						//������ ����? ( �̰� false �� Ǯ��ũ�� �ȴ�! )

	//4. Device ����
	if (FAILED(d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,					//�� ���÷��� �׷��� ī�� �׳� D3DADAPTER_DEFAULT
		D3DDEVTYPE_HAL,						//����̽�Ÿ�� ���� �׳� D3DDEVTYPE_HAL
		hWnd,								//����̽��� ����� ������ �ڵ�
		vertexProcessing,					//���� ó�� ��Ŀ� ���� �÷���
		&d3dpp,								//�տ��� ������ D3DPRESENT_PARAMETERS ����ü ������
		&g_pDevice							//���� ����̽� ����������
		)))
	{
		//����̽� ���� ����
		d3d9->Release(); // Deivce �� ����� ���� ������ Direct3D9 ��ü�� ����
		d3d9 = NULL;

		MessageBoxA( hWnd, "CreateDevice() - FAILED", "FAILED", MB_OK );
		return false;
	}


	//����̽� ���� ����
	d3d9->Release(); // Deivce �� ��������� �� ���̻� �ʿ���� ( �������! )
	d3d9 = NULL;
	return true;
}


bool InitVertexBuffer()
{
	D3DXCreateBox(g_pDevice, 200, 200, 200, &g_Mesh, NULL);

	ZeroMemory(&g_Mtrl, sizeof(g_Mtrl));
	g_Mtrl.Ambient = D3DXCOLOR(1,1,1,1);
	g_Mtrl.Diffuse = D3DXCOLOR(1,1,1,1);
	g_Mtrl.Specular = D3DXCOLOR(1,1,1,1);
	g_Mtrl.Emissive = D3DXCOLOR(0,0,0,1);
	g_Mtrl.Power = 0.f;

	D3DXCOLOR color(1,1,1,1);
	ZeroMemory(&g_Light, sizeof(g_Light));
	g_Light.Type = D3DLIGHT_DIRECTIONAL;
	g_Light.Ambient = color * 0.4f;
	g_Light.Diffuse = color;
	g_Light.Specular = color * 0.6f;
	g_Light.Direction = *(D3DXVECTOR3*)&Vector3(0,-1,0);	

	Matrix44 V;
	Vector3 camPos(0,200,-500);
	Vector3 lookAt(0, 0, 0);
	Vector3 dir = lookAt - camPos;
	dir.Normalize();
	V.SetView(camPos, dir, Vector3(0,1,0));
	g_pDevice->SetTransform(D3DTS_VIEW, (D3DXMATRIX*)&V);

	Matrix44 proj;
	proj.SetProjection(D3DX_PI * 0.5f, (float)WINSIZE_X / (float) WINSIZE_Y, 1.f, 1000.0f) ;
	g_pDevice->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX*)&proj) ;

	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	g_pDevice->SetLight(0, &g_Light); // ���� ����.
	g_pDevice->LightEnable(
		0, // Ȱ��ȭ/ ��Ȱ��ȭ �Ϸ��� ���� ����Ʈ ���� ���
		true); // true = Ȱ��ȭ �� false = ��Ȱ��ȭ

	return true;
}


//����
void Render(int timeDelta)
{
	//ȭ�� û��
	if (SUCCEEDED(g_pDevice->Clear( 
		0,			//û���� ������ D3DRECT �迭 ����		( ��ü Ŭ���� 0 )
		NULL,		//û���� ������ D3DRECT �迭 ������		( ��ü Ŭ���� NULL )
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,	//û�ҵ� ���� �÷��� ( D3DCLEAR_TARGET �÷�����, D3DCLEAR_ZBUFFER ���̹���, D3DCLEAR_STENCIL ���ٽǹ���
		D3DCOLOR_XRGB(0,0,0),			//�÷����۸� û���ϰ� ä���� ����( 0xAARRGGBB )
		1.0f,				//���̹��۸� û���Ұ� ( 0 ~ 1 0 �� ī�޶󿡼� ���ϰ���� 1 �� ī�޶󿡼� ���� �� )
		0					//���ٽ� ���۸� ä�ﰪ
		)))
	{
		//ȭ�� û�Ұ� ���������� �̷�� ���ٸ�... ������ ����
		g_pDevice->BeginScene();

		if (g_Mesh)
		{
			if (g_ani)
			{
				if (g_aniAlpha > 1.f)
				{
					g_aniAlpha = 1.f;
					g_ani = false;
				}

				LPDIRECT3DVERTEXBUFFER9 vtxBuff;
				g_Mesh->GetVertexBuffer(&vtxBuff);
				Vertex *vertices;
				vtxBuff->Lock(0, 0, (void**)&vertices, 0);
				Vector3 v = g_aniSrcPos.Interpolate(g_aniDestPos, g_aniAlpha);
				vertices[ g_aniVtxIdx].p = v;
				vtxBuff->Unlock();

				g_aniAlpha += timeDelta * 0.0005f;
			}
		}

		//r.SetTranslate(Vector3(0, 0, -498)); // teapot
		g_pDevice->SetTransform(D3DTS_WORLD, (D3DXMATRIX*)&g_LocalTm);
		g_pDevice->SetMaterial(&g_Mtrl);
		if (g_Mesh)
			g_Mesh->DrawSubset(0);

		//������ ��
		g_pDevice->EndScene();
		//�������� �������� �������� ���� ȭ������ ����
		g_pDevice->Present( NULL, NULL, NULL, NULL );
	}
}


void Spread()
{
	LPDIRECT3DVERTEXBUFFER9 vtxBuff;
	g_Mesh->GetVertexBuffer(&vtxBuff);

	Vertex *vertices;
	vtxBuff->Lock(0, 0, (void**)&vertices, 0);

	LPDIRECT3DINDEXBUFFER9 idxBuff;
	g_Mesh->GetIndexBuffer(&idxBuff);
	WORD *indices;
	idxBuff->Lock(0, 0, (void**)&indices, 0);

	const int numFace = g_Mesh->GetNumFaces();
	vector<int> adjInfo(numFace*3, 0);
	g_Mesh->GenerateAdjacency(0.0, (DWORD*)&adjInfo[0]);

	for (int i=0; i < numFace-1; ++i)
	{
		for (int k=0; k < 3; ++k)
		{
			const int adjFaceIdx = adjInfo[ i*3 + k];
			if (adjFaceIdx < 0)
				continue;

			int selectVtxIdx = -1;
			for (int m=0; m < 3; ++m)
			{
				const int correspondAdjFaceIdx = adjInfo[ adjFaceIdx*3 + m];
				if (i == correspondAdjFaceIdx)
				{
					const int vtxIdxTable[] = {2,0,1};
					selectVtxIdx = indices[ adjFaceIdx*3 + vtxIdxTable[ m]];
					break;
				}
			}

			if (selectVtxIdx < 0)
				continue;

			const int vtxIdxEdge1[3] = {0, 1, 2};
			const int vtxIdxEdge2[3] = {1, 2, 0};
			const int remainVtxIdx[3] = {2, 0, 1};
			const int vtxIdx1 = indices[ i*3 + vtxIdxEdge1[ k]];
			const int vtxIdx2 = indices[ i*3 + vtxIdxEdge2[ k]];
			const int remIdx = indices[ i*3 + remainVtxIdx[ k]];

			Vector3 contactPos;
			{
				Vector3 v1 = vertices[ vtxIdx2].p - vertices[ vtxIdx1].p;
				Vector3 v2 = vertices[ selectVtxIdx].p  - vertices[ vtxIdx1].p;
				v1.Normalize();
				const float len = v2.Length();
				v2.Normalize();
				contactPos = vertices[ vtxIdx1].p +  v1*(v1.DotProduct(v2) * len);
			}

			Vector3 edgeNormal;
			{
				Vector3 v1 = vertices[ vtxIdx2].p - vertices[ vtxIdx1].p;
				v1.Normalize();

				Vector3 norm = v1.CrossProduct(vertices[ vtxIdx1].n);
				norm.Normalize();				
				
				Vector3 v2 = vertices[ remIdx].p - vertices[ vtxIdx1].p;
				v2.Normalize();

				if (v2.DotProduct(norm) > 0)
					norm = -norm;

				edgeNormal = norm;
			}

			// rotate
			Vector3 v1 = vertices[ selectVtxIdx].p  - contactPos;
			v1.Normalize();
			Quaternion quat;
			quat.SetRotationArc(v1, edgeNormal);
			Matrix44 mat = quat.GetMatrix();

			Vector3 mutaualPos = vertices[ selectVtxIdx].p - contactPos;
			Vector3 pos = mutaualPos * mat;
			pos += contactPos;
			//vertices[ selectVtxIdx].p = pos;

			g_ani = true;
			g_aniVtxIdx = selectVtxIdx;
			g_aniSrcPos = vertices[ selectVtxIdx].p;
			g_aniDestPos = pos;

			if (g_ani)
				break;
		}
		if (g_ani)
			break;
	}

	vtxBuff->Unlock();
	idxBuff->Unlock();
}