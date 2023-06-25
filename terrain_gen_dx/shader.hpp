#pragma once
#include<d3d11.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<string>
#include<wrl.h>
#include<vector>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

class VertexShader {
public:
	VertexShader() = default;
	VertexShader(const std::wstring& shader_file,const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc);
	~VertexShader();
	void destroy();

	void use() const;

private:
	ComPtr<ID3D11VertexShader> m_vshader = nullptr;
	ComPtr<ID3D11InputLayout> m_input_layout = nullptr;

	static ComPtr<ID3DBlob> compileShader(const std::wstring& filename, const std::string& profile);
};

class PixelShader
{
public:
	PixelShader() = default;
	PixelShader(const std::wstring& shader_file);
	~PixelShader();
	void destroy();

	void use() const;

private:
	ComPtr<ID3D11PixelShader> m_pshader = nullptr;

	static ComPtr<ID3DBlob> compileShader(const std::wstring& filename, const std::string& profile);
};

class ComputeShader
{
public:
	ComputeShader() = default;
	ComputeShader(const std::wstring& shader_file);
	~ComputeShader();
	void destroy();

	void dispatch(XMINT3 num_threads) const;

private:
	ComPtr<ID3D11ComputeShader> m_cshader = nullptr;

	static ComPtr<ID3DBlob> compileShader(const std::wstring& filename, const std::string& profile);
};