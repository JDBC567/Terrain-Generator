#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
#include<wrl.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

class Texture2D
{
public:
	Texture2D()=default;
	Texture2D(XMINT2 size);
	~Texture2D();
	void destroy();

	void bind(UINT unit);
	static void unbind(UINT unit);
	void bindVS(UINT unit);
	static void unbindVS(UINT unit);
	void bindCS(UINT unit);
	static void unbindCS(UINT unit);

	void* getRawPointer() const;
private:
	ComPtr<ID3D11ShaderResourceView> m_srv=nullptr;
	ComPtr<ID3D11UnorderedAccessView> m_uav=nullptr;
	ComPtr<ID3D11SamplerState> m_sampler = nullptr;
};