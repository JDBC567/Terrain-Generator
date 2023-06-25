#include"Texture2D.hpp"
#include"window.hpp"

Texture2D::Texture2D(XMINT2 size)
{
	ComPtr<ID3D11Texture2D> texture;

	D3D11_TEXTURE2D_DESC texture_desc{};
	texture_desc.Width = size.x;
	texture_desc.Height = size.y;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32_FLOAT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	Window::getDevice()->CreateTexture2D(&texture_desc, nullptr, texture.GetAddressOf());

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uav_desc.Texture2D.MipSlice = 0;
	Window::getDevice()->CreateUnorderedAccessView(texture.Get(), &uav_desc, m_uav.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;
	Window::getDevice()->CreateShaderResourceView(texture.Get(), &srv_desc, m_srv.GetAddressOf());

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Window::getDevice()->CreateSamplerState(&sampler_desc, m_sampler.GetAddressOf());

	texture->Release();
}

Texture2D::~Texture2D()
{
	if(m_srv!=nullptr)
	{
		destroy();
	}
}

void Texture2D::destroy()
{
	m_uav.Reset();
	m_srv.Reset();
	m_sampler.Reset();
}



void Texture2D::bind(UINT unit)
{
	Window::getDeviceContext()->PSSetShaderResources(unit, 1, m_srv.GetAddressOf());
	Window::getDeviceContext()->PSSetSamplers(unit, 1, m_sampler.GetAddressOf());
}

void Texture2D::unbind(UINT unit)
{
	static const ComPtr<ID3D11ShaderResourceView> tsrv = nullptr;
	Window::getDeviceContext()->PSSetShaderResources(unit, 1, tsrv.GetAddressOf());
}

void Texture2D::bindVS(UINT unit)
{
	Window::getDeviceContext()->VSSetShaderResources(unit, 1, m_srv.GetAddressOf());
	Window::getDeviceContext()->VSSetSamplers(unit, 1, m_sampler.GetAddressOf());
}

void Texture2D::unbindVS(UINT unit)
{
	static const ComPtr<ID3D11ShaderResourceView> tsrv = nullptr;
	Window::getDeviceContext()->VSSetShaderResources(unit, 1, tsrv.GetAddressOf());
}


void Texture2D::bindCS(UINT unit)
{
	Window::getDeviceContext()->CSSetUnorderedAccessViews(unit, 1, m_uav.GetAddressOf(), nullptr);
}

void Texture2D::unbindCS(UINT unit)
{
	static const ComPtr<ID3D11UnorderedAccessView> tuav = nullptr;
	Window::getDeviceContext()->CSSetUnorderedAccessViews(unit, 1, tuav.GetAddressOf(), nullptr);
}

void* Texture2D::getRawPointer() const
{
	return m_srv.Get();
}
