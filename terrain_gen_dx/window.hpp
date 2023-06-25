#pragma once

#include<GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>
#include<d3d11.h>
#include<dxgi1_3.h>
#include<DirectXMath.h>
#include<wrl.h>
#include<unordered_map>
#include<string>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

class Window
{
public:
	Window();
	~Window();

	static void destroy();

	static void startFrame();
	static void endFrame();
	
	static double getDT();
	static bool shouldClose();
	static void setClose();
	static GLFWwindow* getRawWindow();
	static XMINT2 getSize();
	static bool isKeyPressed(int key);
	static bool isMouseButtonPressed(int button);
	static XMFLOAT2 getMouseDT();
	static bool isResized();
	static void setShouldClose();
	static ComPtr<ID3D11Device> getDevice();
	static ComPtr<ID3D11DeviceContext> getDeviceContext();
	static void setRendermodeWireframe();
	static void setRendermodeSolid();

private:
	//dx11
	static ComPtr<IDXGIFactory2> m_dxgi_factory;
	static ComPtr<ID3D11Device> m_device;
	static ComPtr<ID3D11DeviceContext> m_device_context;
	static ComPtr<IDXGISwapChain1> m_swap_chain;
	static ComPtr<ID3D11RenderTargetView> m_render_target;
	static ComPtr<ID3D11DepthStencilView> m_depth_stencil;
	static ComPtr<ID3D11DepthStencilState> m_depth_stencil_state;
	static ComPtr<ID3D11RasterizerState> m_rasterizer_state;
	static D3D11_VIEWPORT m_viewport;
	//window
	static GLFWwindow* m_window;
	static XMINT2 m_win_size;
	//misc
	static double m_time_counter;
	static double m_ms_dt;
	static double m_dt;
	static XMFLOAT2 m_mouse_prev;
	static XMFLOAT2 m_mouse_dt;
	static bool m_resized;

	static void resize_callback(GLFWwindow* window, int width, int height);

	static void createRTV();

	static void createDSV();
};