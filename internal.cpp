#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"detours.lib")


#include<Windows.h>
#include<iostream>
#include<string>
#include<Detours.h>
#include<imgui.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui.h_impl_win32.h"
#include<d3dx9.h>
#include<d3d9.h>


LPDIRECT3D9 gçpD3D =  NULL ;
LPDIRECT3DDEVICE9 g_p3dDevice = NULL ;
D3DPRESENT_PARAMETERS g_d3dpp = {} ;
IDirect3DPixelShader9* shaderback ;
IDirect3DPixelShader9* shaderfront ;

UNIT stride ;
D3DVERTEXELEMENT9 decl[MAX3DDECLLENGTH] ;
UNIT num_Elements , mStartRegister , mVectorCount , vSize , pSize ;
IDirect3DPixelShader9* vShader ;
IDirect3DPixelShader9* pShader ;


LPDIRECT3DDEVICE9   g_pd3Device= NULL ;
IDirect3D9 *        g_pD3D =  NULL ;
HWND 			    window = NULL ;
WINDPROC		  	wndproc_orig = NULL ;


bool show_menu = false ;
bool chams = true ;


typedef HRESULT (_stdcall* EndScene)(LPDIRECT3DDEVICE9 pDevice) ;
HRESULT _stdcall hkEndScene (LPDIRECT3DDEVICE9 pDevice) ;
EndScene oEndScene

typedef HRESULT(APIENTRY* SetStreamSource ) (IDirect3DDevice9* , UNIT , IDirect3DVertexBuffer9 , UNIT , UNIT) ;
SetStreamSource SetStreamSource_orig = 0 ;


typedef HRESULT(APIENTRY* SetVertexDeclaration ) (IDirect3DDevice9* ,  IDirect3DSetVertexDeclaration9) ;
SetStreamSource SetVertexDeclaration_orig = 0 ;

typedef HRESULT(APIENTRY* SetVertexShaderConstantF ) (IDirect3DDevice9*  ,  UNIT  , const float*  , UNIT  ) ;
SetStreamSource SetVertexShaderConstantF_orig = 0 ;

typedef HRESULT(APIENTRY* SetVertexShader ) (IDirect3DDevice9* ,  IDirect3DVertexShader9) ;
SetStreamSource SetVertexShader_orig = 0 ;

typedef HRESULT(APIENTRY* SetPixelShader ) (IDirect3DDevice9* ,  IDirect3DPixelShader9) ;
SetStreamSource SetPixelShader_orig = 0 ;

typedef HRESULT(APIENTRY* DrawIndexedPrimitive ) (IDirect3DDevice9* ,  D3DPRIMITIVETYPE , INT , UNIT , UNIT , UNIT , UNIT) ;
SetStreamSource DrawIndexedPrimitive_orig = 0 ;

HRESULT GenerateShader (IDirect3DDevice9* pD3Ddev , IDirect3DPixelShader9** pShader , float r , float g , float b)
{
	char sz_Shader[256] ;
	ID3DXBuffer* pShaderBuf = NULL ;
	sprintf_s(sz_Shader," ps.1.1\ndef c0 ,%f , %f, %f ,%f\nmov r0, c0 ",  r,g,b,1.0f) ;
	D3DXAssembleShader(sz_Shader,sizeof(sz_Shader),NULL , NULL , 0 , &pShaderBuf , NULL ) ;
	if (FAILED (pD3Ddev -> CreatePixelShader (  (const DWORD*)pShaderBuf-> GetBufferPointer() ,pShader))) return E_FAIL ;

	return D3D_OK
}

BOOL CALLBACK EnumWindowsCallback (HWND handle , LPARAM lParam)
{

	DWORD wndProcID ;
	GetWindowThreadProcessId(HWND handle , &wndProcID);


	if (GetCurrentProcessId () != wndProcID) {

		return TRUE ;
	}

	window  = handle ;
	return FALSE ;


}

HWND GetProcessWindow() {
	EnumWindows(EnumWindowsCallback , NULL) ;
	return window ;

}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd , UINT msg , WPARAM wparam , LPARAM lParam) ;
LRESULT WINAPI WndProc(HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam)
{

	if (show_menu && ImGui_ImplWin32_WndProcHandler(hWnd ,  msg ,  wparam ,  lParam))
	{

		return true ;
	}

	return CallWindowProc(wndproc_orig , hWnd , msg , wParam , lParam);

}


bool GetD3D9Device(void ** pTable , size_t size)
{

	if (!pTable) {
		return false ;
	}

	g_pD3D = Direct3DCreate(D3D_SDK_VERSION);
	if (!g_pD3D) {

		return false ;
	}
	D3DPRESENT_PARAMETERS d3dpp = {} ;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.SwapEffect = GetProcessWindow() ;
	d3dpp.Windowed = false ;

	g_pD3D-> CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,d3dpp.hDeviceWindow , 
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp , &g_pd3dDevice);
	if (!g_pd3dDevice)
	{

		g_pD3D->Release();
		return false ;

	}

	memcpy(pTable , *reinterpret_cast<void***>(g_pd3dDevice) , size ) ;

	g_pd3dDevice->Release();
	g_pD3D->Release();
	return true ;
}

void CleanupDeviceD3D() {

	if (g_pd3dDevice) {

		g_pd3dDevice->Release();
		g_pd3dDevice = NULL ;

	}
	if (g_pD3D) {

		g_pD3D->Release();
		g_pD3D = NULL ;
	}
}

HRESULT __stdcall hkEndScene (LPDIRECT3DDEVICE9  pDevice) 
{
	static bool init = false ;
	if (GetAsyncKeyState(VK_INSERT))
	{
		show_menu = !show_menu ;

	}	
	if (!init ) {
		wndproc_orig = (WNDPROC)SetWindowProcLongPtr(window , GWLP_WNDPROC , (LONG_PTR)WndProc) ;
		IMGUI_CHECKVERSION() ;
		ImGui::CreateContext() ;
		ImGuiIO& io = ImGui::GetIO() ;

		ImGui::styleColorsDark ;
		ImGui_ImplWin32_Init(window) ;
		ImGui_ImplDX9_Init(pDevice) ;


		if (!shaderback) {

			GenerateShader(pDevice ,&shaderback , 221.0f /255.0f, 177.0f/255.0f , 31.0f/255.0f)


		}

		if (!shaderfront) {

			GenerateShader(pDevice ,&shaderfront , 31.0f /255.0f, 99.0f/255.0f , 155.0f/255.0f)


		}


		init = true ; 
	}
	if (init) {
		if( show_menu ) {
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame() ;
			ImGui::Begin("Menu" , &show_menu) ;
			{

				ImGui::Checkbox('Chams' ,&chams);

			}
			ImGui::Render() ;
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData())


		}

	}
	return oEndScene(pDevice) ;
}

HRESULT APIENTRY SetStreamSource_hook  (IDirect3DDevice9 pDevice , UNIT StreamNumber , 
	IDirect3DVertexBuffer9 pStreamData , UNIT offsetInBytes , UNIT sStride) 
{
	if (StramNumber == 0)
	{
		stride = sStride ;


	}
	return SetStreamSource_orig(pDevice, StreamNumber , pStreamData ,offsetInBytes , sStride ) ;
}



HRESULT APIENTRY SetVertexDeclaration_hook  (IDirect3DDevice9 pDevice , IDirect3DSetVertexDeclaration9* pdecl ){
	if (pdecl != NULL)
	{
		pdecl -> GetDeclaration(decl ,&num_Elements) ;



	}
	return SetVertexDeclaration_orig(pDevice, pdecl ) ;
}


HRESULT APIENTRY SetVertexShaderConstantF_hook  (IDirect3DDevice9 pDevice ,  UNIT StartRegister , const float* pConstantData , UNIT Vector4fCount){
	if (pConstantData != NULL)
	{
		mStartRegister = StartRegister ;
		mVectorCount = Vector4fCount ;




	}
	return SetVertexShaderConstantF_orig(pDevice, StartRegister , pConstantData , Vector4fCount ) ;
}



HRESULT APIENTRY SetVertexShader_hook  (IDirect3DDevice9 pDevice ,  IDirect3DVertexShader9* veShader) {


	if (veShader != NULL) {
		vShader = veShader ;
		vShader -> GetFunction(NULL , &vSize) ;


	}


	return SetVertexShader_orig(pDevice ,veShader ) ;
}


 HRESULT APIENTRY SetPixelShader_hook  (IDirect3DDevice9 pDevice ,  IDirect3DPixelShader9* piShader) {

	if (piShader != NULL) {
		pShader = piShader ;
		pShader -> GetFunction(NULL , &pSize) ;


	}



	return SetPixelShader_orig(pDevice , piShader) ;
}


 HRESULT APIENTRY DrawIndexedPrimitive_hook  (IDirect3DDevice9* pDevice ,  D3DPRIMITIVETYPE Type , INT BaseVertexIndex , UNIT MinVertexIndex ,
 		 UNIT NumVertices  , UNIT startIndex , UNIT  primCount) {

 	if (chams) {

 		if (T_Models || CT_Models ) {

 			pDevice -> SetRenderState(D3DRS_ZENABLE , D3DZB_FALSE) ;
 			pDevice -> SetPixelShader(shaderback ) ;
 			DrawIndexedPrimitive_orig(pDevice  , Type ,BaseVertexIndex ,  MinVertexIndex ,NumVertices , startIndex , primCount );
 			pDevice -> SetRenderState(D3DRS_ZENABLE ,  D3DZB_TRUE) ;
 			pDevice -> SetPixelShader(shaderfront) ;

 		}
 	}

 	return  DrawIndexedPrimitive_orig(pDevice  , Type ,BaseVertexIndex ,  MinVertexIndex ,NumVertices , startIndex , primCount );

 }

DWORD WINAPI mainThread (PVOID base ) {

	void*  d3d9Device[119] ;
	if (GetD3D9Device(d3d9Device , sizeof(d3d9Device) ) ) {

		oEndScene = (EndScene)Detours::X86::DetourFunction((Detours::unit8_t*) d3d9Device[42] ,(Detours::unit8_t*)  hkEndScene) ;


		SetStreamSource_orig = (SetStreamSource)Detours::X86::DetourFunction( (Detours::unit8_t*) d3d9Device[]   , (Detours::unit8_t*)SetStreamSource_hook ) ;
		SetVertexDeclaration_orig = (SetVertexDeclaration)Detours::X86::DetourFunction( (Detours::unit8_t*) d3d9Device[100]    , (Detours::unit8_t*)SetVertexDeclaration_hook ) ;
		SetVertexShaderConstantF_orig  = (SetVertexShaderConstantF)Detours::X86::DetourFunction( (Detours::unit8_t*) d3d9Device[87]     , (Detours::unit8_t*) SetVertexShaderConstantF_hook)
		SetVertexShader_orig  =  (SetVertexShader)Detours::X86::DetourFunction( (Detours::unit8_t*) d3d9Device[94]     , (Detours::unit8_t*) SetVertexShader_hook ) ;
		SetPixelShader_orig = (SetPixelShader)Detours::X86::DetourFunction( (Detours::unit8_t*) d3d9Device[92]    , (Detours::unit8_t*)SetPixelShader_hook ) ;
		DrawIndexedPrimitive_orig  = (DrawIndexedPrimitive)Detours::X86::DetourFunction(  (Detours::unit8_t*) d3d9Device[107]    , (Detours::unit8_t*)DrawIndexedPrimitive_hook );


		while (true){
			if (GetAsyncKeyState(VK_F10))
			{
				CleanupDeviceD3D();
				FreeLibraryAndExitThread(static_cast<HMODULE>(base) , 1 );

			}


		}
	}

	FreeLibraryAndExitThread(static_cast<HMODULE>(base) , 1 );

}



BOOL APIENTRY DllMain ( HMODULE hModule ,
	DWORD ul_reason_for_call ,
	LPVOID lpReserved )

{

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH :
		CreateThread(nullptr , NULL , mainThread ,hModule ,  NULL , nullptr );
		case DLL_THREAD_ATTACH :
		case DLL_PROCESS_DETACH :
		case DLL_THREAD_DETACH :
			break ;
	}

	return true ;
}