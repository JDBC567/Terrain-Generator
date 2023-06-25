#include"buffer.hpp"
#include"window.hpp"

ConstantBuffer::ConstantBuffer(const void* data,const UINT size)
{
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.ByteWidth = size;

	D3D11_SUBRESOURCE_DATA buffer_data{};
	buffer_data.pSysMem = data;

	Window::getDevice()->CreateBuffer(&buffer_desc, &buffer_data, m_buffer.GetAddressOf());
}

ConstantBuffer::~ConstantBuffer()
{
	if(m_buffer!=nullptr)
	{
		destroy();
	}
}

void ConstantBuffer::destroy()
{
	m_buffer.Reset();
}


void ConstantBuffer::use(UINT unit)
{
	Window::getDeviceContext()->VSSetConstantBuffers(unit, 1, m_buffer.GetAddressOf());
}

void ConstantBuffer::useCS(UINT unit)
{
	Window::getDeviceContext()->CSSetConstantBuffers(unit, 1, m_buffer.GetAddressOf());
}

void ConstantBuffer::updateData(const void* data, UINT size,UINT offset,bool discard) const
{
	D3D11_MAPPED_SUBRESOURCE mapped_res;
	if(discard)
	{
		Window::getDeviceContext()->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	}
	else
	{
		Window::getDeviceContext()->Map(m_buffer.Get(), 0,D3D11_MAP_WRITE, 0, &mapped_res);
	}
	memcpy(static_cast<char*>(mapped_res.pData)+offset, data, size);
	Window::getDeviceContext()->Unmap(m_buffer.Get(), 0);
}
