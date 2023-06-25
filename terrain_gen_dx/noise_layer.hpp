#pragma once
#include"texture2d.hpp"
#include"shader.hpp"
#include"buffer.hpp"
#include<DirectXMath.h>

using namespace DirectX;

class NoiseLayer
{
public:
	virtual ~NoiseLayer() = default;
	virtual void generate()=0;
	virtual bool renderUI()=0;
	virtual void* getTexture();
	virtual void bindTexture(UINT unit);
	virtual void unbindTexture(UINT unit);

	virtual void setName(char* name);
	virtual char* getName();

	virtual void setSize(XMINT2 size);
	virtual XMINT2 getSize();
protected:
	Texture2D m_texture;
	ConstantBuffer m_cbuffer;
	char m_name[50]="";
	XMINT2 m_size = { 8,8 };
};

class SimplexNoise : public NoiseLayer
{
public:
	struct SNCB {         //DirectX constant buffers require data to be padded on 16 byte boundaries
		float ns[2];
		float no[2];
		int seed;
	};

	SimplexNoise() = default;
	SimplexNoise(const char* name);
	~SimplexNoise() override = default;

	void generate() override;
	bool renderUI() override;

	static void init();

	int _id;
private:
	static ComputeShader m_compute_shader;
	static int m_counter;


	XMFLOAT2 m_noise_scale = { 1.0f,1.0f };
	XMFLOAT2 m_noise_offset = { 0.0f,0.0f };
	int m_seed = 1;
};