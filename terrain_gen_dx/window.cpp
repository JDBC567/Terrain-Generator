#include"window.hpp"
#include<iostream>
#include<format>
#include"imgui/imgui_impl_dx11.h"
#include"imgui/imgui_impl_glfw.h"
#include"imgui/imgui.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

ComPtr<IDXGIFactory2> Window::m_dxgi_factory = nullptr;
ComPtr<ID3D11Device> Window::m_device=nullptr;
ComPtr<ID3D11DeviceContext> Window::m_device_context = nullptr;
ComPtr<IDXGISwapChain1> Window::m_swap_chain = nullptr;
ComPtr<ID3D11RenderTargetView> Window::m_render_target = nullptr;
ComPtr<ID3D11DepthStencilView> Window::m_depth_stencil = nullptr;
ComPtr<ID3D11DepthStencilState> Window::m_depth_stencil_state = nullptr;
ComPtr<ID3D11RasterizerState> Window::m_rasterizer_state = nullptr;
D3D11_VIEWPORT Window::m_viewport = D3D11_VIEWPORT{};

GLFWwindow* Window::m_window = nullptr;
XMINT2 Window::m_win_size = XMINT2(0, 0);

double Window::m_time_counter = 0;
double Window::m_ms_dt = 0;
double Window::m_dt = 0;
XMFLOAT2 Window::m_mouse_prev = XMFLOAT2(0, 0);
XMFLOAT2 Window::m_mouse_dt = XMFLOAT2(0, 0);
bool Window::m_resized = false;

Window::Window()
{
	if (!glfwInit())
	{
		std::cerr << "failed to initialize glfw";
		exit(-1);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);

	m_win_size= XMINT2(vidmode->width,vidmode->height);

	m_viewport.Width = static_cast<float>(vidmode->width);
	m_viewport.Height = static_cast<float>(vidmode->height);
	m_viewport.MaxDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_window = glfwCreateWindow(vidmode->width, vidmode->height, "Terrain Generator", nullptr, nullptr);

	if(m_window==nullptr)
	{
		std::cerr << "failed to initialize window";
		exit(-1);
	}

	glfwSetFramebufferSizeCallback(m_window, resize_callback);

	if(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgi_factory))))
	{
		std::cerr << "failed to create DXGIFactory";
		exit(-1);
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&deviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&m_device,
		nullptr,
		&m_device_context)))
	{
		std::cerr << "D3D11: Failed to create device and device Context\n";
		exit(-1);
	}


	if(m_device_context==nullptr)
	{
		std::cerr << "aaa";
	}
	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
	swapChainDescriptor.Width = m_win_size.x;
	swapChainDescriptor.Height = m_win_size.y;
	swapChainDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescriptor.SampleDesc.Count = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = 2;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.Scaling = DXGI_SCALING_STRETCH;
	swapChainDescriptor.Flags = {};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
	swapChainFullscreenDescriptor.Windowed = true;

	if (FAILED(m_dxgi_factory->CreateSwapChainForHwnd(
		m_device.Get(),
		glfwGetWin32Window(m_window),
		&swapChainDescriptor,
		&swapChainFullscreenDescriptor,
		nullptr,
		&m_swap_chain)))
	{
		std::cerr << "DXGI: Failed to create swapchain\n";
		exit(-1);
	}

	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	if (FAILED(m_swap_chain->GetBuffer(
		0,
		IID_PPV_ARGS(&backBuffer))))
	{
		std::cerr << "D3D11: Failed to get Back Buffer from the SwapChain\n";
		exit(-1);
	}

	if (FAILED(m_device->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&m_render_target)))
	{
		std::cerr << "D3D11: Failed to create RTV from Back Buffer\n";
		exit(-1);
	}

	D3D11_TEXTURE2D_DESC tex_desc{};
	tex_desc.Width = m_win_size.x;
	tex_desc.Height = m_win_size.y;
	tex_desc.ArraySize = 1;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.MipLevels = 1;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	ComPtr<ID3D11Texture2D> tex = nullptr;
	m_device->CreateTexture2D(&tex_desc, nullptr, tex.GetAddressOf());
	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	m_device->CreateDepthStencilView(tex.Get(), &dsv_desc, m_depth_stencil.GetAddressOf());
	tex->Release();

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	m_device->CreateDepthStencilState(&depth_stencil_desc, &m_depth_stencil_state);

	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;

	m_device->CreateRasterizerState(&rasterizer_desc, m_rasterizer_state.GetAddressOf());

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplDX11_Init(m_device.Get(),m_device_context.Get());
	ImGui_ImplGlfw_InitForOther(m_window, true);
}

Window::~Window()
{
	if(m_device_context!=nullptr)
	{
		destroy();
	}
}

void Window::destroy()
{
	m_device_context->Flush();
	m_render_target.Reset();
	m_depth_stencil.Reset();
	m_depth_stencil_state.Reset();
	m_rasterizer_state.Reset();
	m_swap_chain.Reset();
	m_dxgi_factory.Reset();
	m_device_context.Reset();
	m_device.Reset();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::startFrame()
{
	m_resized = false;
	constexpr float clear_color[] = { 0.2274f, 0.5372f, 0.7294f,1.0f };
	m_device_context->ClearRenderTargetView(m_render_target.Get(), clear_color);
	m_device_context->ClearDepthStencilView(m_depth_stencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_device_context->RSSetViewports(1, &m_viewport);
	m_device_context->RSSetState(m_rasterizer_state.Get());
	m_device_context->OMSetRenderTargets(1, m_render_target.GetAddressOf(), m_depth_stencil.Get());
	m_device_context->OMSetDepthStencilState(m_depth_stencil_state.Get(), 0);

	glfwPollEvents();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Time Data");
	if(m_time_counter>0.5)
	{
		m_time_counter = 0.0;
		m_ms_dt = m_dt * 1000;
	}
	ImVec4 ms_color;
	if (m_ms_dt > 32.0)
	{
		ms_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (m_ms_dt > 16.0)
	{
		ms_color = ImVec4(0.98f, 0.24f, 0.04f, 1.0f);
	}
	else
	{
		ms_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	ImGui::TextColored(ms_color, std::format("frame time: {:.1f}ms", m_ms_dt).c_str());
	ImGui::TextColored(ms_color, std::format("fps: {:d}", static_cast<int>(1000.0 / (m_ms_dt + 0.000001))).c_str());
	ImGui::End();

	double cpos_x, cpos_y;
	glfwGetCursorPos(m_window, &cpos_x, &cpos_y);
	m_mouse_dt.x = static_cast<float>(cpos_x) - m_mouse_prev.x;
	m_mouse_dt.y = static_cast<float>(cpos_y) - m_mouse_prev.y;
	m_mouse_prev.x = static_cast<float>(cpos_x);
	m_mouse_prev.y = static_cast<float>(cpos_y);
}

void Window::endFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_swap_chain->Present(0,0);

	static double prev_frame_time;
	const double current = glfwGetTime();
	m_dt = current - prev_frame_time;
	prev_frame_time = current;
	m_time_counter += m_dt;
}

double Window::getDT()
{
	return m_dt;
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void Window::setClose()
{
	glfwSetWindowShouldClose(m_window, true);
}

GLFWwindow* Window::getRawWindow()
{
	return m_window;
}

XMINT2 Window::getSize()
{
	return m_win_size;
}

bool Window::isKeyPressed(int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button)
{
	return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

XMFLOAT2 Window::getMouseDT()
{
	return m_mouse_dt;
}

bool Window::isResized()
{
	return m_resized;
}

void Window::setShouldClose()
{
	glfwSetWindowShouldClose(m_window, true);
}

ComPtr<ID3D11Device> Window::getDevice()
{
	return m_device;
}

ComPtr<ID3D11DeviceContext> Window::getDeviceContext()
{
	return m_device_context;
}

void Window::setRendermodeWireframe()
{
	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;

	m_device->CreateRasterizerState(&rasterizer_desc, m_rasterizer_state.GetAddressOf());
}

void Window::setRendermodeSolid()
{
	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;

	m_device->CreateRasterizerState(&rasterizer_desc, m_rasterizer_state.GetAddressOf());
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
	m_win_size = XMINT2(width, height);

	m_render_target.Reset();

	m_swap_chain->ResizeBuffers(
		0,
		width,
		height,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		0);

	createRTV();
	createDSV();

	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
}

void Window::createRTV()
{
	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	m_device->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&m_render_target);
}

void Window::createDSV()
{
	D3D11_TEXTURE2D_DESC tex_desc{};
	tex_desc.Width = m_win_size.x;
	tex_desc.Height = m_win_size.y;
	tex_desc.ArraySize = 1;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.MipLevels = 1;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;

	ComPtr<ID3D11Texture2D> texture = nullptr;
	m_device->CreateTexture2D(&tex_desc, nullptr, texture.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	m_device->CreateDepthStencilView(texture.Get(), &dsv_desc, m_depth_stencil.GetAddressOf());

	texture->Release();
}
