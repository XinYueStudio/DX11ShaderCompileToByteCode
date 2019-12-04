// ShaderCompileFromFile.cpp : 定义应用程序的入口点。
//

#include "header.h"
#include "ShaderCompileFromFile.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void InitDevice();

void Resize();
 


#include <assert.h>
#include <locale>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include  <commdlg.h>
#include <shobjidl.h> 
using namespace std;


//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "D3dx9.lib")
#pragma comment(lib, "Dxgi.lib")
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include <d3d11.h>
#include <D3Dcompiler.h>
#include <d3dx11.h>
#include <d3d11_2.h>
#include <d3dx11tex.h>
#include <dxgi1_2.h>

#include <DirectXMath.h>
#include <D3DX10.h>
#include <D3dx9math.h>
#include <directxcolors.h>
 

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

struct	Size
{
	UINT Width;
	UINT Height;
};



// Check for SDK Layer support.
inline bool SdkLayersAvailable()
{
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
		0,
		D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
		nullptr,                    // Any feature level will do.
		0,
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		nullptr,                    // No need to keep the D3D device reference.
		nullptr,                    // No need to know the feature level.
		nullptr                     // No need to keep the D3D device context reference.
	);

	return SUCCEEDED(hr);
}


inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};


inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

 


// Pipeline objects.	
	// Direct3D 11.0
ComPtr < ID3D11Device> m_Device;                  // D3D11.1设备
ComPtr < ID3D11DeviceContext> m_DeviceContext; // D3D11.1设备上下文
ComPtr < IDXGISwapChain> m_SwapChain;                // D3D11.1交换链
ComPtr<IDXGIDevice> dxgiDevice;
HWND										  m_Hwnd;
D3D_FEATURE_LEVEL                m_FeatureLevel;
bool                                           m_StereoEnabled;
static const UINT						  m_FrameCount = 2;
Size											  m_Resolution;
Size											  m_RenderTargetSize;
ComPtr < ID3D11RenderTargetView> m_RenderTargetView;
ComPtr < ID3D11RenderTargetView> m_RenderTargetViewRight;
ComPtr < ID3D11DepthStencilView>  m_DepthStencilView;

bool show_demo_window = false;
bool show_Shader_window = false;
bool compile_Shader = false;
bool show_another_window = false;

PWSTR shaderReources;

int vertexShaderBufferSize = 0;
int pixelShaderBufferSize = 0;
void* vertexShaderBufferPtr = nullptr;
void* pixelShaderBufferPtr = nullptr;

string vertexShaderBufferStr;
string pixelShaderBufferStr;

char* vertexShaderBufferCtr;
char*  pixelShaderBufferCtr;
int  vertexShaderBufferCtrLen;
int  pixelShaderBufferCtrLen;

void InitDevice()
{
	RECT rect;
	if (GetWindowRect(m_Hwnd, &rect))
	{
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		m_Resolution.Width = width;
		m_Resolution.Height = height;
	}


	// This flag adds support for surfaces with a different color channel ordering
  // than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;


#if defined(_DEBUG)
	if (SdkLayersAvailable())
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ThrowIfFailed(
		D3D11CreateDevice(
			nullptr,                    // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			creationFlags,              // Set debug and Direct2D compatibility flags.
			featureLevels,              // List of feature levels this app can support.
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device,                    // Returns the Direct3D device created.
			&m_FeatureLevel,            // Returns feature level of device created.
			&context                    // Returns the device immediate context.
		)
	);
	// Get the Direct3D 11.1 API device and context interfaces.
	ThrowIfFailed(
		device.As(&m_Device)
	);

	ThrowIfFailed(
		context.As(&m_DeviceContext)
	);

	// Get the underlying DXGI device of the Direct3D device.
	ThrowIfFailed(
		m_Device.As(&dxgiDevice)
	);





	Resize();



}

void Resize()
{
 

	// m_swapChain is nullptr either because it has never been created or because it has been
	// invalidated. Make sure that the dependent objects are also released.
	m_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_RenderTargetView = nullptr;
	m_DepthStencilView = nullptr;
	m_RenderTargetViewRight = nullptr;
	m_DeviceContext->Flush();

	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_Resolution.Width;          // Swapchain needs to be 2x sized for direct stereo.
	sd.BufferDesc.Height = m_Resolution.Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 120;	// Needs to be 120Hz for 3D Vision 
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	sd.OutputWindow = m_Hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;


	// Identify the physical adapter (GPU or card) this device is running on.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(
		dxgiDevice->GetAdapter(&dxgiAdapter)
	);

	// And obtain the factory object that created it.
	ComPtr<IDXGIFactory1> dxgiFactory;
	ThrowIfFailed(
		dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
	);

	// Obtain the final swap chain for this window from the DXGI factory.
	ThrowIfFailed(
		dxgiFactory->CreateSwapChain(
			m_Device.Get(),
			&sd,
			&m_SwapChain
		)
	);


	dxgiFactory->MakeWindowAssociation(m_Hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_PRINT_SCREEN | DXGI_MWA_VALID);



	// Obtain the backbuffer for this window which will be the final 3D rendertarget.
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(
		m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
	);

	// Create a descriptor for the RenderTargetView.
	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
		D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		0,
		1
	);

	// Create a view interface on the rendertarget to use on bind for mono or left eye view.
	ThrowIfFailed(
		m_Device->CreateRenderTargetView(
			backBuffer.Get(),
			&renderTargetViewDesc,
			&m_RenderTargetView
		)
	);

	// Stereo swapchains have an arrayed resource, so create a second Render Target
	// for the right eye buffer.
	if (m_StereoEnabled)
	{
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewRightDesc(
			D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			0,
			0,
			1
		);

		ThrowIfFailed(
			m_Device->CreateRenderTargetView(
				backBuffer.Get(),
				&renderTargetViewRightDesc,
				&m_RenderTargetViewRight
			)
		);
	}

	// Cache the rendertarget dimensions in our helper class for convenient use.
	D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
	backBuffer->GetDesc(&backBufferDesc);
	m_RenderTargetSize.Width = backBufferDesc.Width;
	m_RenderTargetSize.Height = backBufferDesc.Height;

	// Create a descriptor for the depth/stencil buffer.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		backBufferDesc.Width,
		backBufferDesc.Height,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	// Allocate a 2-D surface as the depth/stencil buffer.
	ComPtr<ID3D11Texture2D> depthStencil;
	ThrowIfFailed(
		m_Device->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&depthStencil
		)
	);

	// Create a DepthStencil view on this surface to use on bind.
	auto viewDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
	ThrowIfFailed(
		m_Device->CreateDepthStencilView(
			depthStencil.Get(),
			&viewDesc,
			&m_DepthStencilView
		)
	);

	// Create a viewport descriptor of the full window size.
	CD3D11_VIEWPORT viewport(
		0.0f,
		0.0f,
		static_cast<float>(backBufferDesc.Width),
		static_cast<float>(backBufferDesc.Height)
	);

	// Set the current viewport using the descriptor.
	m_DeviceContext->RSSetViewports(1, &viewport);
}

 



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
	ImGui_ImplWin32_EnableDpiAwareness();

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SHADERCOMPILEFROMFILE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SHADERCOMPILEFROMFILE));
	
    MSG msg;

 
	InitDevice();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingAlwaysTabBar = true;
	//io.ConfigDockingTransparentPayload = true;
#if 1
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI
#endif

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(m_Hwnd);
	ImGui_ImplDX11_Init(m_Device.Get(), m_DeviceContext.Get());

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		if (show_Shader_window)
		{
			

			if (compile_Shader)
			{
				compile_Shader = false;

				HRESULT result;
				ID3D10Blob* errorMessage;
				ID3D10Blob* vertexShaderBuffer;
				ID3D10Blob* pixelShaderBuffer;
				D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
				unsigned int numElements;
				D3D11_BUFFER_DESC matrixBufferDesc;
			 


				// Initialize the pointers this function will use to null.
				errorMessage = 0;
				vertexShaderBuffer = 0;
				pixelShaderBuffer = 0;

				// Compile the vertex shader code.
				result = D3DX11CompileFromFile(shaderReources, NULL, NULL, "vert", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
					&vertexShaderBuffer, &errorMessage, NULL);
				if (FAILED(result))
				{
					MessageBox(NULL, shaderReources, L"Missing Shader File", MB_OK);
				}

				// Compile the pixel shader code.
				result = D3DX11CompileFromFile(shaderReources, NULL, NULL, "frag", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
					&pixelShaderBuffer, &errorMessage, NULL);
				if (FAILED(result))
				{

					MessageBox(NULL, shaderReources, L"Missing Shader File", MB_OK);

				}

				
		   	  vertexShaderBufferSize=	vertexShaderBuffer->GetBufferSize();		 
			  pixelShaderBufferSize = pixelShaderBuffer->GetBufferSize();
			  vertexShaderBufferPtr = vertexShaderBuffer->GetBufferPointer();
			  pixelShaderBufferPtr = pixelShaderBuffer->GetBufferPointer();

			  {
				  char* data = static_cast<char*>(vertexShaderBufferPtr);
				  std::string tempName(data, vertexShaderBufferSize);
				  vertexShaderBufferStr = tempName;
				  int Count = 0;
				  int LineCount = 50;
				  ofstream myfile;
				  remove("VertexShader.txt");
				  myfile.open("VertexShader.txt");
				  for (int i = 0; i < vertexShaderBufferSize; i++)
				  {
					  myfile << (int)data[i];
					  if(i< vertexShaderBufferSize-1)
					  myfile << ",";
					  
					  Count++;
					  if (Count >= LineCount)
					  {
						  Count = 0;
						  myfile << "\r";
					  }
				  }


				  myfile.close();
			
				  ifstream in("VertexShader.txt", ios::in | ios::binary);
				 
				  //获取文件的大小
				  in.seekg(0, in.end);
				  int length = in.tellg();
				  in.seekg(0, in.beg);

				  char* temp = new char[length];

				  if (in.is_open()) {
					 
					  in.read(temp, length);
				  }
				  vertexShaderBufferStr = temp;
				  vertexShaderBufferCtr = temp;
				  vertexShaderBufferCtrLen = length;
				  in.close();
			  }


			  {
				  char*	  data = static_cast<char*>(pixelShaderBufferPtr);
				  std::string   tempName(data, pixelShaderBufferSize);
				  pixelShaderBufferStr = tempName;
				  ofstream myfile;
				  remove("PixelShader.txt");
				  myfile.open("PixelShader.txt");
				  int Count = 0;
				  int LineCount = 50;
				  for (int i = 0; i < pixelShaderBufferSize; i++)
				  {
					  myfile << (int)data[i];
					  if (i < pixelShaderBufferSize - 1)
						  myfile << ",";
					  Count++;
					  if (Count >= LineCount)
					  {
						  Count = 0;
						  myfile << "\r";
					  }
				  }
			 
				  myfile.close();

				  ifstream in("PixelShader.txt", ios::in | ios::binary);

				  //获取文件的大小
				  in.seekg(0, in.end);
				  int length = in.tellg();
				  in.seekg(0, in.beg);

				  char* temp = new char[length];

				  if (in.is_open()) {

					  in.read(temp, length);
				  }
				  pixelShaderBufferStr = temp;
				  pixelShaderBufferCtr = temp;
				  pixelShaderBufferCtrLen = length;
				  in.close();
			  
			  }
			 
			
		
			}
		
			if (vertexShaderBufferSize != 0 &&
				pixelShaderBufferSize != 0 &&
				vertexShaderBufferPtr != nullptr &&
				pixelShaderBufferPtr != nullptr)
			{
				ImGui::Begin("VertexShader Buffer  vert vs_5_0");                          // Create a window called "Hello, world!" and append into it.
				ImGui::SameLine();
				ImGui::InputTextMultiline("VertexShader", vertexShaderBufferCtr, vertexShaderBufferCtrLen+2,ImVec2(300,300), ImGuiInputTextFlags_Multiline| ImGuiInputTextFlags_NoMarkEdited,0, vertexShaderBufferCtr);
				ImGui::SameLine();
			 
				ImGui::End();
			}
			if (vertexShaderBufferSize != 0 &&
				pixelShaderBufferSize != 0 &&
				vertexShaderBufferPtr != nullptr &&
				pixelShaderBufferPtr != nullptr)
			{
				ImGui::Begin("PixelShader Buffer  frag ps_5_0");                          // Create a window called "Hello, world!" and append into it.
				ImGui::SameLine();
				ImGui::InputTextMultiline("PixelShader", pixelShaderBufferCtr, pixelShaderBufferCtrLen+2, ImVec2(300, 300), ImGuiInputTextFlags_Multiline | ImGuiInputTextFlags_NoMarkEdited);
				ImGui::SameLine(); 
				 
				ImGui::End();
			}
		}



		// Rendering
		ImGui::Render();

		// Clear both the render target and depth stencil to default values.
		const float ClearColor[4] = { 0x38 / 255.0f, 0x38 / 255.0f, 0x38 / 255.0f, 1.0f };
		m_DeviceContext->OMSetRenderTargets(
			1,
			m_RenderTargetView.GetAddressOf(),
			m_DepthStencilView.Get());

		m_DeviceContext->ClearRenderTargetView(
			m_RenderTargetView.Get(),
			ClearColor);

		m_DeviceContext->ClearDepthStencilView(
			m_DepthStencilView.Get(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		m_SwapChain->Present(1, 0); // Present with vsync
    }
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();



    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHADERCOMPILEFROMFILE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SHADERCOMPILEFROMFILE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   m_Hwnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_OPENFILE:
			{
				IFileOpenDialog *pFileOpen;

				HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
					COINIT_DISABLE_OLE1DDE);
				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
					IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));


				if (SUCCEEDED(hr))
				{
					// Show the Open dialog box.
					hr = pFileOpen->Show(NULL);

					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem *pItem;
						hr = pFileOpen->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

							// Display the file name to the user.
							if (SUCCEEDED(hr))
							{
								//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);

								show_Shader_window = true;
								shaderReources = pszFilePath;
								compile_Shader= true;
								CoTaskMemFree(pszFilePath);
							}
							pItem->Release();
						}
					}
					pFileOpen->Release();
				}
				CoUninitialize();
			}
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_SIZE:
		if (m_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			if (m_RenderTargetView) { m_RenderTargetView= nullptr; }
			m_SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			Resize();
		}	
		break;
	
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
 