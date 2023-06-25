#pragma once
#include<d3d11.h>
#include<wrl.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class ConstantBuffer
{
public:
	ConstantBuffer()=default;
	ConstantBuffer(const void* data, UINT size);
	~ConstantBuffer();
	void destroy();

	void use(UINT unit);
	void useCS(UINT unit);
	void updateData(const void* data, UINT size, UINT offset, bool discard) const;

private:
	ComPtr<ID3D11Buffer> m_buffer=nullptr;
};