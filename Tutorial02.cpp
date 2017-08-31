//--------------------------------------------------------------------------------------
// File: Tutorial02.cpp
//
// This application displays a triangle using Direct3D 11
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729719.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include <array>

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT2 Pos;
};

const float data[] = {
	1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
};

struct BoundaryBuffer
{
	XMINT4 mBoundary;
}BoundaryBuffer;


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           g_pVertexBuffer = nullptr;
ID3D11Buffer*           g_pVertexBuffer2 = nullptr;
ID3D11Buffer*           g_pConstantBuffer = nullptr;
ID3D11Buffer*           g_vConstantBuffer = nullptr;
float(*mapPS)[4] = nullptr;
int width = 2;
int height = 2;
D3D11_QUERY_DESC queryDesc;
ID3D11Query * pQuery;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 2: Rendering a Triangle",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	g_driverType = driverTypes[0];
	hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, nullptr);

	if (FAILED(hr))
		return hr;

	g_pd3dDevice->GetImmediateContext(&g_pImmediateContext);

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = 16;
		sd.Height = 16;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.Stereo = 0;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.BufferUsage = 112;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Flags = 0;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = 16;
		sd.BufferDesc.Height = 16;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlobSetup = nullptr;
	ID3D11VertexShader *g_pVertexShaderSetup;
	hr = CompileShaderFromFile(L"Tutorial02.fx", "VS_Passthrough2D", "vs_4_0", &pVSBlobSetup);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlobSetup->GetBufferPointer(), pVSBlobSetup->GetBufferSize(), nullptr, &g_pVertexShaderSetup);
	if (FAILED(hr))
	{
		pVSBlobSetup->Release();
		return hr;
	}

	g_pImmediateContext->VSSetShader(g_pVertexShaderSetup, {}, 0);

	ID3D11InputLayout *g_pVertexLayoutSetup;
	D3D11_INPUT_ELEMENT_DESC layoutsetup[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElementsSetup = ARRAYSIZE(layoutsetup);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layoutsetup, numElementsSetup, pVSBlobSetup->GetBufferPointer(),
		pVSBlobSetup->GetBufferSize(), &g_pVertexLayoutSetup);
	pVSBlobSetup->Release();
	if (FAILED(hr))
		return hr;

	// Create rasterizer sate set up
	ID3D11RasterizerState *g_pRasterizerStateSetup;
	D3D11_RASTERIZER_DESC rasterizerDescSetup;
	rasterizerDescSetup.FillMode = D3D11_FILL_SOLID;
	rasterizerDescSetup.CullMode = D3D11_CULL_NONE;
	rasterizerDescSetup.FrontCounterClockwise = FALSE;
	rasterizerDescSetup.DepthBias = 0;
	rasterizerDescSetup.SlopeScaledDepthBias = 0.0f;
	rasterizerDescSetup.DepthBiasClamp = 0.0f;
	rasterizerDescSetup.DepthClipEnable = TRUE;
	rasterizerDescSetup.ScissorEnable = FALSE;
	rasterizerDescSetup.MultisampleEnable = FALSE;
	rasterizerDescSetup.AntialiasedLineEnable = FALSE;

	g_pd3dDevice->CreateRasterizerState(&rasterizerDescSetup, &g_pRasterizerStateSetup);
	g_pImmediateContext->RSSetState(g_pRasterizerStateSetup);

	// create sampler state set up
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.MaxLOD = 3.402823466e+38F;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MipLODBias = 0.0f;
	ID3D11SamplerState *samplerState;
	g_pd3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
	g_pImmediateContext->PSSetSamplers(0, 1, &samplerState);

	ID3D11ShaderResourceView * g_pShaderResourceView = nullptr;
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pShaderResourceView);

	g_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Create Vertex buffer set up
	ID3D11Buffer *g_pVertexBUfferSetup;
	D3D11_BUFFER_DESC bufferDescSetup;
	bufferDescSetup.ByteWidth = 64;
	bufferDescSetup.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescSetup.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescSetup.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDescSetup.MiscFlags = 0;
	bufferDescSetup.StructureByteStride = 0;

	g_pd3dDevice->CreateBuffer(&bufferDescSetup, nullptr, &g_pVertexBUfferSetup);

	D3D11_MAPPED_SUBRESOURCE mapsetup = { 0 };
	hr = g_pImmediateContext->Map(g_pVertexBUfferSetup, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapsetup);
	if (FAILED(hr))
		return hr;
	uint8_t *mapVertexsetup = static_cast<uint8_t *>(mapsetup.pData);
	const float datasetup[] = { -1,-1,0,0,-1,1,0,1,1,-1,1,0,1,1,1,1 };
	const void* tempsetup = datasetup;
	memcpy(mapVertexsetup, static_cast<const uint8_t *>(tempsetup), 64);
	g_pImmediateContext->Unmap(g_pVertexBUfferSetup, 0);

	std::array<ID3D11Buffer*, 16> mCurrentVertexBuffers = {};
	mCurrentVertexBuffers[0] = g_pVertexBUfferSetup;
	std::array<UINT, 16> mCurrentVertexStrides;
	mCurrentVertexStrides[0] = 16;
	std::array<UINT, 16> mCurrentVertexOffsets;
	mCurrentVertexOffsets[0] = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 16, &mCurrentVertexBuffers[0], &mCurrentVertexStrides[0], &mCurrentVertexOffsets[0]);

	// Set Blend State set up
	static const float blendFactorsetup[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	g_pImmediateContext->OMSetBlendState(nullptr, blendFactorsetup, 0xFFFFFFF);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)16;
	vp.Height = (FLOAT)16;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the pixel shader set up
	ID3DBlob* pPSBlobSetup = nullptr;
	hr = CompileShaderFromFile(L"Tutorial02.fx", "PS_PassthroughRGBA2D", "ps_4_0", &pPSBlobSetup);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	ID3D11PixelShader* g_pPixelShaderSetup;
	hr = g_pd3dDevice->CreatePixelShader(pPSBlobSetup->GetBufferPointer(), pPSBlobSetup->GetBufferSize(), nullptr, &g_pPixelShaderSetup);
	pPSBlobSetup->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->PSSetShader(g_pPixelShaderSetup, {},0);


	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	//g_pImmediateContext->UpdateSubresource(pBackBuffer, 0, nullptr, addr, 64, 1024);

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// set rendertarget
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial02_VS.hlsl", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// create texture2d
	D3D11_TEXTURE2D_DESC texturedesc;
	texturedesc.Width = 16;
	texturedesc.Height = 16;
	texturedesc.MipLevels = 1;
	texturedesc.ArraySize = 1;
	texturedesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;
	texturedesc.SampleDesc.Count = 1;
	texturedesc.SampleDesc.Quality = 0;
	texturedesc.Usage = D3D11_USAGE_DEFAULT;
	texturedesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texturedesc.CPUAccessFlags = 0;
	texturedesc.MiscFlags = 0;
	g_pd3dDevice->CreateTexture2D(&texturedesc,NULL, &pBackBuffer);

	//create shader resource view set up
	ID3D11ShaderResourceView* srvsetup;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvsetupdesc;
	srvsetupdesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;
	srvsetupdesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvsetupdesc.Texture2D.MipLevels = 1;
	srvsetupdesc.Texture2D.MostDetailedMip = 0;
	g_pd3dDevice->CreateShaderResourceView(pBackBuffer,&srvsetupdesc, &srvsetup);

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(float) * 12;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	hr = g_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;
   
	D3D11_MAPPED_SUBRESOURCE map1 = { 0 };
	hr = g_pImmediateContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE, 0, &map1);
	if (FAILED(hr))
		return hr;
	uint8_t *mapVertex = static_cast<uint8_t *>(map1.pData);
	const void* temp = data;
	memcpy(mapVertex, static_cast<const uint8_t *>(temp), sizeof(float) * 12);
	g_pImmediateContext->Unmap(g_pVertexBuffer, 0);

	// create query
	queryDesc.Query = D3D11_QUERY_OCCLUSION;
	queryDesc.MiscFlags = 0;
	g_pd3dDevice->CreateQuery(&queryDesc, &pQuery);
	g_pImmediateContext->Begin(pQuery);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial02_PS.hlsl", "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set primitive topology
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set shaders
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->GSSetShader(nullptr, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	// set rendertarget
	std::array<ID3D11RenderTargetView *,8>  rtvarray = { {} };
	g_pImmediateContext->OMSetRenderTargets(0, rtvarray.data(), nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp_init;
	vp_init.Width = (FLOAT)width;
	vp_init.Height = (FLOAT)height;
	vp_init.MinDepth = 0.0f;
	vp_init.MaxDepth = 1.0f;
	vp_init.TopLeftX = 0;
	vp_init.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp_init);

	// Set rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	ID3D11RasterizerState *rasterizerState;
	g_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	g_pImmediateContext->RSSetState(rasterizerState);

	// Set Blend State
	static const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_pImmediateContext->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFF);
	
	// create DepthStencil State
	D3D11_DEPTH_STENCIL_DESC depthstencilDesc;
	depthstencilDesc.DepthEnable = 0;
	depthstencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthstencilDesc.StencilEnable = 0;
	depthstencilDesc.StencilReadMask = 255;
	depthstencilDesc.StencilWriteMask = 255;
	depthstencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthstencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ID3D11DepthStencilState *depthstencilState;
	g_pd3dDevice->CreateDepthStencilState(&depthstencilDesc, &depthstencilState);
	g_pImmediateContext->OMSetDepthStencilState(depthstencilState,0);

	// Create the ps constant buffer
	D3D11_BUFFER_DESC constantBufferDescription = { 0 };

	constantBufferDescription.ByteWidth = sizeof(float) * 4;
	constantBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescription.MiscFlags = 0;
	constantBufferDescription.StructureByteStride = 0;

	hr = g_pd3dDevice->CreateBuffer(&constantBufferDescription, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	D3D11_MAPPED_SUBRESOURCE map2 = { 0 };
	HRESULT hr2 =
		g_pImmediateContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map2);
	if (FAILED(hr2))
		return hr2;
	mapPS = (float(*)[4])map2.pData;

	XMINT4 size = XMINT4(width - 1, height, 0, 1);
	memcpy(&mapPS[0][0], &size, sizeof(float) * 4);
	g_pImmediateContext->Unmap(g_pConstantBuffer, 0);

	ID3D11Buffer *appliedVertexConstants = NULL;
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &appliedVertexConstants);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	// Create the vs constant buffer
	D3D11_BUFFER_DESC vsconstantBufferDescription = { 0 };
	vsconstantBufferDescription.ByteWidth = 320;
	vsconstantBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	vsconstantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsconstantBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsconstantBufferDescription.MiscFlags = 0;
	vsconstantBufferDescription.StructureByteStride = 0;

	hr = g_pd3dDevice->CreateBuffer(&vsconstantBufferDescription, nullptr, &g_vConstantBuffer);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_vConstantBuffer);

	// Create the ps constant buffer
	D3D11_BUFFER_DESC psconstantBufferDescription = { 0 };

	psconstantBufferDescription.ByteWidth = 320;
	psconstantBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	psconstantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	psconstantBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	psconstantBufferDescription.MiscFlags = 0;
	psconstantBufferDescription.StructureByteStride = 0;

	hr = g_pd3dDevice->CreateBuffer(&psconstantBufferDescription, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pConstantBuffer);

	// map vs buffer
	D3D11_MAPPED_SUBRESOURCE mapvs = { 0 };
	HRESULT hrvs =
		g_pImmediateContext->Map(g_vConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapvs);
	if (FAILED(hrvs))
		return hrvs;
	float ShaderConstants[16] =
	{ 0,1,1,-431602080,
		-431602080,-431602080, -431602080, -431602080,
		1,1,1,1,
		1, -1,1,1 };
	memcpy(mapvs.pData, ShaderConstants, sizeof(ShaderConstants));
	int samplerMetaData[64] = { 0 };
	memcpy(mapvs.pData, samplerMetaData, sizeof(samplerMetaData));

	g_pImmediateContext->Unmap(g_vConstantBuffer, 0);
	
	//map ps buffer
	D3D11_MAPPED_SUBRESOURCE mapps = { 0 };
	HRESULT hrps =
		g_pImmediateContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapps);
	if (FAILED(hrps))
		return hrps;
	float ShaderConstants2[16] =
	{ 0,1,1,-431602080,
		1,1, 1, 1,
		0.5,0.5,-431602080,-431602080,
		1, -1,1,1 };
	memcpy(mapps.pData, ShaderConstants2, sizeof(ShaderConstants2));
	memcpy(mapps.pData, samplerMetaData, sizeof(samplerMetaData));

	g_pImmediateContext->Unmap(g_pConstantBuffer, 0);

	// get device context type
	auto type = g_pImmediateContext->GetType();

	g_pImmediateContext->SOSetTargets(0, nullptr, nullptr);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// copy to a new Vertex Buffer
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = g_pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &g_pVertexBuffer2);
	if (FAILED(hr))
		return hr;

	D3D11_BOX srcBox;
	srcBox.left = 0;
	srcBox.right = static_cast<unsigned int>(12 * sizeof(float));
	srcBox.top = 0;
	srcBox.bottom = 1;
	srcBox.front = 0;
	srcBox.back = 1;

	g_pImmediateContext->CopySubresourceRegion(g_pVertexBuffer2, 0, 0, 0,
		0, g_pVertexBuffer, 0, &srcBox);

	std::array<ID3D11Buffer*, 16> mCurrentVertexBuffers2 = {};
	mCurrentVertexBuffers[0] = g_pVertexBuffer2;
	std::array<UINT, 16> mCurrentVertexStrides2;
	mCurrentVertexStrides2[0] = 8;
	std::array<UINT, 16> mCurrentVertexOffsets2;
	mCurrentVertexOffsets2[0] = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 16, &mCurrentVertexBuffers[0], &mCurrentVertexStrides2[0], &mCurrentVertexOffsets2[0]);

	// Render a triangle
	g_pImmediateContext->Draw(6, 0);

	g_pImmediateContext->End(pQuery);

	UINT64 queryData = 0; // This data type is different depending on the query type
	while (S_OK != g_pImmediateContext->GetData(pQuery, &queryData, sizeof(UINT64), 0))
	{
		g_pd3dDevice->GetDeviceRemovedReason();
	}

	// Present the information rendered to the back buffer to the front buffer (the screen)
	g_pSwapChain->Present(0, 0);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// Note that this tutorial does not handle resizing (WM_SIZE) requests,
		// so we created the window without the resize border.

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

