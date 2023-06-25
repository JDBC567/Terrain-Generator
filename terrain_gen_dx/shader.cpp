#include"shader.hpp"
#include"window.hpp"
#include <iostream>

VertexShader::VertexShader(const std::wstring& shader_file,const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc)
{
	const auto shader_blob = compileShader(shader_file, "vs_5_0");

    Window::getDevice()->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &m_vshader);

    Window::getDevice()->CreateInputLayout(input_element_desc.data(), input_element_desc.size(), shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), &m_input_layout);
}

VertexShader::~VertexShader()
{
    if(m_vshader!=nullptr)
    {
        destroy();
    }
}

void VertexShader::destroy()
{
    if(m_vshader!=nullptr)
    {
        m_vshader.Reset();
        m_input_layout.Reset();
    }
}

void VertexShader::use() const
{
    Window::getDeviceContext()->VSSetShader(m_vshader.Get(), nullptr, 0);
    Window::getDeviceContext()->IASetInputLayout(m_input_layout.Get());
    if(m_input_layout==nullptr)
    {
        exit(-100);
    }
}

ComPtr<ID3DBlob> VertexShader::compileShader(const std::wstring& filename, const std::string& profile)
{
	constexpr unsigned int compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;

	ComPtr<ID3DBlob> shader_blob = nullptr;
	ComPtr<ID3DBlob> error_blob = nullptr;

    if(FAILED(D3DCompileFromFile(filename.data(), nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        profile.data(),
        compile_flags,
        0,
        &shader_blob,
        &error_blob)))
    {
	    if(error_blob!=nullptr)
	    {
            std::cerr << "failed to compile shader : " << static_cast<const char*>(error_blob->GetBufferPointer());
            exit(-1);
	    }
    }

    return shader_blob;
}

PixelShader::PixelShader(const std::wstring& shader_file)
{
	const auto shader_blob = compileShader(shader_file, "ps_5_0");

    Window::getDevice()->CreatePixelShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &m_pshader);
}

PixelShader::~PixelShader()
{
	if(m_pshader!=nullptr)
	{
        destroy();
	}
}

void PixelShader::destroy()
{
    m_pshader.Reset();
}

void PixelShader::use() const
{
    Window::getDeviceContext()->PSSetShader(m_pshader.Get(), nullptr, 0);
}

ComPtr<ID3DBlob> PixelShader::compileShader(const std::wstring& filename, const std::string& profile)
{
    constexpr unsigned int compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> shader_blob = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;

    if (FAILED(D3DCompileFromFile(filename.data(), nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        profile.data(),
        compile_flags,
        0,
        &shader_blob,
        &error_blob)))
    {
        if (error_blob != nullptr)
        {
            std::cerr << "failed to compile shader : " << static_cast<const char*>(error_blob->GetBufferPointer());
            exit(-1);
        }
    }

    return shader_blob;
}

ComputeShader::ComputeShader(const std::wstring& shader_file)
{
    const auto shader_blob = compileShader(shader_file, "cs_5_0");
    if(shader_blob==nullptr)
    {
        std::cout << "invalid filename";
        exit(-1);
    }
    Window::getDevice()->CreateComputeShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, &m_cshader);
}

ComputeShader::~ComputeShader()
{
	if(m_cshader!=nullptr)
	{
        destroy();
	}
}

void ComputeShader::destroy()
{
    m_cshader.Reset();
}

void ComputeShader::dispatch(XMINT3 num_threads) const
{
    Window::getDeviceContext()->CSSetShader(m_cshader.Get(), nullptr, 0);
    Window::getDeviceContext()->Dispatch(num_threads.x, num_threads.y, num_threads.z);
}

ComPtr<ID3DBlob> ComputeShader::compileShader(const std::wstring& filename, const std::string& profile)
{
    constexpr unsigned int compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> shader_blob = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;

    if (FAILED(D3DCompileFromFile(filename.data(), nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        profile.data(),
        compile_flags,
        0,
        &shader_blob,
        &error_blob)))
    {
        if (error_blob != nullptr)
        {
            std::cerr << "failed to compile shader : " << static_cast<const char*>(error_blob->GetBufferPointer());
            exit(-1);
        }
    }

    return shader_blob;
}