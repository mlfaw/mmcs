/*{REPLACEMEWITHLICENSE}*/
#include "win32_ImagePainter.hpp"
#include <windowsx.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"
#include "win32_resource.h"
#include "win32_hinstance.h"
#include "msw_misc.hpp" // msw::SafeRelease()
#include "mmcs_MediaScaling.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite.h>


// TODO: D2DERR_RECREATE_TARGET

static unsigned char * ImageData = NULL;
static int ImageX = 0, ImageY = 0, ImageComp = 0;

static ID3D11Device * d3d11Device = NULL;
static ID3D11DeviceContext * d3d11DeviceContext = NULL;
static IDXGIDevice1 * dxgiDevice1 = NULL;
static ID2D1Factory1 * d2d1Factory1 = NULL;
static ID2D1Device * d2d1Device = NULL;
static ID2D1DeviceContext * d2d1DeviceContext = NULL;
//static ID2D1HwndRenderTarget * d2d1DeviceContext = NULL;
static IDXGIAdapter * dxgiAdapter = NULL;
static IDXGIFactory2 * dxgiFactory2 = NULL;
static IDXGISwapChain1 * dxgiSwapChain1 = NULL;
static IDXGISurface * dxgiBackBuffer = NULL;
static ID2D1Bitmap1 * d2d1BackBufferBitmap = NULL;
static ID2D1Bitmap * d2d1MasterChiefBitmap = NULL;

static IDWriteFactory * writeFactory = NULL;
static IDWriteTextFormat * writeTextFormat = NULL;

static ID2D1SolidColorBrush * pBlackBrush = NULL;

namespace win32 {

static int nxtidx = 0;
static const char * ff[] = {
	"C:\\code\\mmcs\\235KB.png",
	"C:\\code\\mmcs\\900KB_2.jpg",
	"C:\\code\\mmcs\\27MB.png",
};
static bool setupNextImage()
{
	int x, y, comp;
	unsigned char * data = stbi_load(
		ff[nxtidx],
		&x,
		&y,
		&comp,
		STBI_rgb_alpha // load as R8G8B8A8
	);
	if (!data) return false;
	nxtidx = !nxtidx;
	if (ImageData)
		stbi_image_free(ImageData);
	ImageData = data;
	ImageX = x;
	ImageY = y;
	ImageComp = comp;
	return true;
}

static BOOL setupSwapChain(HWND hwnd)
{
	HRESULT hr;
	DXGI_SCALING dxgiScalingMode = DXGI_SCALING_NONE;
	if (false) // Windows 7 doesn't support DXGI_SCALING_NONE
		dxgiScalingMode = DXGI_SCALING_ASPECT_RATIO_STRETCH;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
	swapChainDesc.Width = 0; // use automatic sizing
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // todo?
	swapChainDesc.Stereo = false; // todo?
	swapChainDesc.SampleDesc.Count = 1; // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // use double buffer to enable flip
	swapChainDesc.Scaling = dxgiScalingMode;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this swapeffect (UWP)?
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ?

	hr = dxgiFactory2->CreateSwapChainForHwnd(
		d3d11Device,
		hwnd,
		&swapChainDesc,
		NULL, // ?
		NULL,
		&dxgiSwapChain1
	);
	if (FAILED(hr)) {
		return FALSE;
	}
	// Ensure that DXGI doesn't queue more than one frame at a time.
	hr = dxgiDevice1->SetMaximumFrameLatency(1);
	if (FAILED(hr)) return FALSE;

	D2D1_PIXEL_FORMAT pixelFormat = {};
	pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	D2D1_BITMAP_PROPERTIES1 bitmapProperties1 = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		pixelFormat,
		96.0f,
		96.0f,
		NULL
	);

	hr = dxgiSwapChain1->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	if (FAILED(hr)) return FALSE;
	hr = d2d1DeviceContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer,
		&bitmapProperties1,
		&d2d1BackBufferBitmap
	);
	if (FAILED(hr)) return FALSE;
	d2d1DeviceContext->SetTarget(d2d1BackBufferBitmap);

	return TRUE;
}

static BOOL IpWmCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	HRESULT hr;

	if (!setupNextImage()) return FALSE;

	const D2D1_FACTORY_OPTIONS factoryOptions = { D2D1_DEBUG_LEVEL_INFORMATION };
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory1),
		&factoryOptions,
		(void **)&d2d1Factory1
	);
	if (FAILED(hr)) return FALSE;

	// This flag is required in order to enable compatibility with Direct2D.
	UINT d3d11CreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	d3d11CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the ordering of feature levels that D3D should attempt to create.
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL acceptedFeatureLevel;

	hr = D3D11CreateDevice(
		NULL, // pAdapter - uses first adapater from IDXGIFactory1::EnumAdapters
		D3D_DRIVER_TYPE_HARDWARE, // the hardware is the fast one..
		NULL, // Software rasterizer module
		d3d11CreationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&d3d11Device,
		&acceptedFeatureLevel,
		&d3d11DeviceContext
	);
	if (FAILED(hr)) return FALSE;

	hr = d3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice1);
	if (FAILED(hr)) return FALSE;
	hr = d2d1Factory1->CreateDevice(dxgiDevice1, &d2d1Device);
	if (FAILED(hr)) return FALSE;
	hr = d2d1Device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		&d2d1DeviceContext
	);
	if (FAILED(hr)) return FALSE;

	// Identify the physical adapter this devices runs on
	hr = dxgiDevice1->GetAdapter(&dxgiAdapter);
	if (FAILED(hr)) return FALSE;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory2));
	if (FAILED(hr)) return FALSE;

	if (!setupSwapChain(hwnd)) return FALSE;

	//RECT rc;
	//if (!GetClientRect(hwnd, &rc)) return FALSE;

	//D2D1_SIZE_U size = D2D1::SizeU(
	//	rc.right - rc.left,
	//	rc.bottom - rc.top
	//);

	//d2d1Factory1->CreateHwndRenderTarget(
	//	D2D1::RenderTargetProperties(),
	//	D2D1::HwndRenderTargetProperties(hwnd, size),
	//	&d2d1DeviceContext
	//);

	D2D1_PIXEL_FORMAT pixelFormat = {};
	pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	D2D1_BITMAP_PROPERTIES masterChiefBitmapProperties = {};
	masterChiefBitmapProperties.pixelFormat = pixelFormat;
	masterChiefBitmapProperties.dpiX = 96.0f;
	masterChiefBitmapProperties.dpiY = 96.0f;

	hr = d2d1DeviceContext->CreateBitmap(
		D2D1::SizeU(ImageX, ImageY),
		masterChiefBitmapProperties,
		&d2d1MasterChiefBitmap
	);
	if (FAILED(hr)) return FALSE;

	D2D1_RECT_U destRect = {0};
	destRect.right = ImageX;
	destRect.bottom = ImageY;
	d2d1MasterChiefBitmap->CopyFromMemory(
		&destRect,
		ImageData,
		4 * ImageX
	);
	stbi_image_free(ImageData);
	ImageData = NULL;
	if (FAILED(hr)) return FALSE;

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		(IUnknown**)&writeFactory
	);
	if (FAILED(hr)) return FALSE;
	
	hr = d2d1DeviceContext->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&pBlackBrush
	);
	if (FAILED(hr)) return FALSE;

	hr = writeFactory->CreateTextFormat(
		L"Calibre",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		72.0f,
		L"en-us",
		&writeTextFormat
	);
	if (FAILED(hr)) return FALSE;

	return TRUE;
}

static void IpWmDestroy(HWND hwnd)
{
	msw::SafeRelease(&pBlackBrush);
	msw::SafeRelease(&writeTextFormat);
	msw::SafeRelease(&writeFactory);
	msw::SafeRelease(&d2d1MasterChiefBitmap);
	msw::SafeRelease(&d2d1BackBufferBitmap);
	msw::SafeRelease(&dxgiBackBuffer);
	msw::SafeRelease(&dxgiSwapChain1);
	msw::SafeRelease(&dxgiFactory2);
	msw::SafeRelease(&dxgiAdapter);
	msw::SafeRelease(&d2d1DeviceContext);
	msw::SafeRelease(&d2d1Device);
	msw::SafeRelease(&d2d1Factory1);
	msw::SafeRelease(&dxgiDevice1);
	msw::SafeRelease(&d3d11DeviceContext);
	msw::SafeRelease(&d3d11Device);
	if (ImageData)
		stbi_image_free(ImageData);
}

static float g_Scale = 1.0f;
static void IpWmPaintInner(HWND hwnd)
{
	HRESULT hr;
	RECT rc;
	D2D1_TAG tag1, tag2;

	if (!GetClientRect(GetParent(hwnd), &rc)) return;

	D2D1_RECT_F srcRect = D2D1::RectF(
		0.0f,
		0.0f,
		(float)ImageX,
		(float)ImageY
	);

	float dw, dh, dx, dy;
	mmcs::CenterFit(
		(float)ImageX,
		(float)ImageY,
		(float)rc.right,
		(float)rc.bottom,
		dw,
		dh,
		dx,
		dy
	);

	mmcs::ScaleSizePos(
		g_Scale,
		dw,
		dh,
		dx,
		dy
	);

	D2D1_RECT_F destRect = D2D1::RectF(
		dx,
		dy,
		dx + dw,
		dh + dy
	);

	d2d1DeviceContext->BeginDraw();

	d2d1DeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));

#if 0
	// Apply the scale transform to the render target.
	d2d1DeviceContext->SetTransform(
		D2D1::Matrix3x2F::Scale(
			D2D1::Size(0.8f, 0.8f)
			//, D2D1::Point2F(438.0f, 80.5f)
		)
	);
#endif

	d2d1DeviceContext->DrawBitmap(
		d2d1MasterChiefBitmap,
		destRect,
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		NULL
	);

	hr = d2d1DeviceContext->EndDraw(&tag1, &tag2);
	if (hr != S_OK) {
		// D2DERR_RECREATE_TARGET?
		return;
	}

	DXGI_PRESENT_PARAMETERS presentParameters = {0};
	presentParameters.DirtyRectsCount = 0; // update whole frame
	presentParameters.pDirtyRects = NULL;
	presentParameters.pScrollRect = NULL;
	presentParameters.pScrollOffset = NULL;
	hr = dxgiSwapChain1->Present1(1, 0, &presentParameters);

	// todo with hr...
}

static void IpWmPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (!hdc) return;
	IpWmPaintInner(hwnd);
	(void)EndPaint(hwnd, &ps);
}

#if 0
static bool firstSize = true;
static void IpWmSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state == SIZE_MINIMIZED) return;
	if (firstSize) {
		firstSize = false;
		return;
	}
	// Stuff

	//RECT rc;
	//if (!GetClientRect(hwnd, &rc)) return;
	//D2D1_SIZE_U size = D2D1::SizeU(cx, cy);
	//d2d1DeviceContext->Resize(size);
	//(void)UpdateWindow(hwnd);

	d3d11DeviceContext->ClearState();
	d3d11DeviceContext->Flush();
	msw::SafeRelease(&d2d1BackBufferBitmap);
	msw::SafeRelease(&dxgiBackBuffer);
	msw::SafeRelease(&dxgiSwapChain1);

	if (!setupSwapChain(hwnd)) {
		MessageBoxA(NULL, "Failed to setup swap chain", "Fuck", MB_OK);
		ExitProcess(1);
	}
}

static void IpWmMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	if (!setupNextImage()) return;
	msw::SafeRelease(&d2d1MasterChiefBitmap);

	D2D1_PIXEL_FORMAT pixelFormat = {};
	pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

	D2D1_BITMAP_PROPERTIES masterChiefBitmapProperties = {};
	masterChiefBitmapProperties.pixelFormat = pixelFormat;
	masterChiefBitmapProperties.dpiX = 96.0f;
	masterChiefBitmapProperties.dpiY = 96.0f;

	HRESULT hr;
	hr = d2d1DeviceContext->CreateBitmap(
		D2D1::SizeU(ImageX, ImageY),
		masterChiefBitmapProperties,
		&d2d1MasterChiefBitmap
	);
	if (FAILED(hr)) return;

	D2D1_RECT_U destRect = {0};
	destRect.right = ImageX;
	destRect.bottom = ImageY;
	d2d1MasterChiefBitmap->CopyFromMemory(
		&destRect,
		ImageData,
		4 * ImageX
	);
	if (FAILED(hr)) return;
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
}
#endif

LRESULT CALLBACK IpWindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, IpWmCreate);
		HANDLE_MSG(hwnd, WM_DESTROY, IpWmDestroy);
		HANDLE_MSG(hwnd, WM_PAINT, IpWmPaint);
		case WM_ERASEBKGND:
			// TODO: Keep this?
			return 1;
#if 0
		HANDLE_MSG(hwnd, WM_SIZE, IpWmSize);
		HANDLE_MSG(hwnd, WM_MOUSEWHEEL, IpWmMouseWheel);
#endif
	case WM_MOUSEWHEEL:
	{
		float scaleAmount = 0.10f;
		short delta = HIWORD(wParam);
		if (delta > 0) { // scroll up
			g_Scale += scaleAmount;
		} else { // scroll down
			g_Scale -= scaleAmount;
			if (g_Scale < scaleAmount)
				g_Scale = scaleAmount;
		}
		InvalidateRect(hwnd, NULL, FALSE);
	}
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ImagePainter_Create(HWND hParent)
{
	HWND hwnd = CreateWindowExW(
		0, //WS_EX_ACCEPTFILES,
		L"ImagePainter",
		L"",
		WS_CHILD | WS_VISIBLE,
		0, // x
		0, // y
		1920, // width
		1080, // height
		hParent,
		(HMENU)ID_IMAGEPAINTER,
		HINST_THISCOMPONENT,
		NULL // lpParam
	);
	if (!hwnd)
		return false;

	return true;
}

}
