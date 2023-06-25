#include"mesh.hpp"
#include"window.hpp"

Mesh Mesh::createMesh(const void* vertex_data,UINT vertex_data_size,const void* index_data,UINT index_count,UINT stride)
{
	ComPtr<ID3D11Buffer> vertex_buffer;
	ComPtr<ID3D11Buffer> index_buffer;

	const UINT _index_count = index_count;

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.ByteWidth = vertex_data_size;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vertex_data;

	Window::getDevice()->CreateBuffer(&buffer_desc, &data, &vertex_buffer);

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.ByteWidth = index_count * 4;

	data.pSysMem = index_data;

	Window::getDevice()->CreateBuffer(&buffer_desc, &data, &index_buffer);

	return Mesh(vertex_buffer, index_buffer, stride, 0, index_count);
}

Mesh::~Mesh()
{
	if (m_vertex_buffer != nullptr)
	{
		destroy();
	}
}

void Mesh::destroy()
{
	m_vertex_buffer.Reset();
	m_index_buffer.Reset();
}

void Mesh::render()
{
	Window::getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Window::getDeviceContext()->IASetVertexBuffers(0, 1, m_vertex_buffer.GetAddressOf(), &m_stride, &m_offset);
	Window::getDeviceContext()->IASetIndexBuffer(m_index_buffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT,0);
	Window::getDeviceContext()->DrawIndexed(m_index_count, 0, 0);
}

Mesh::Mesh(const ComPtr<ID3D11Buffer>& vertex_buffer, const ComPtr<ID3D11Buffer>& index_buffer, UINT stride, UINT offset, UINT index_count)
{
	m_vertex_buffer = vertex_buffer;
	m_index_buffer = index_buffer;
	m_stride = stride;
	m_offset = offset;
	m_index_count = index_count;
}