#pragma once
#include<d3d11.h>
#include<wrl.h>
#include<vector>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class Mesh
{
public:
	Mesh()=default;

	static Mesh createMesh(const void* vertex_data,UINT vertex_data_size, const void* index_data,UINT index_count,UINT stride);
	~Mesh();
	void destroy();

	void render();

private:
	Mesh(const ComPtr<ID3D11Buffer>& vertex_buffer, const ComPtr<ID3D11Buffer>& index_buffer, UINT stride, UINT offset, UINT index_count);

	ComPtr<ID3D11Buffer> m_vertex_buffer=nullptr;
	ComPtr<ID3D11Buffer> m_index_buffer=nullptr;
	UINT m_stride = 0;
	UINT m_offset = 0;
	UINT m_index_count=0;
};