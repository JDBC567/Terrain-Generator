#pragma once
#include"mesh.hpp"
#include"shader.hpp"
#include"buffer.hpp"
#include"generator.hpp"
#include<DirectXMath.h>

using namespace DirectX;

class World
{
public:
	World(XMINT2 world_size);
	~World() = default;
	void destroy();

	void start();

private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMINT2 texture_index;
	};

	struct ShaderData
	{
		XMMATRIX vp_mat;
		XMFLOAT4 position[4];
	};


	XMFLOAT2 m_rotation{};
	XMFLOAT2 m_rot_spd = XMFLOAT2(10.0f, 10.0f);
	XMFLOAT3 m_position{};
	XMFLOAT3 m_mov_spd = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMMATRIX m_view_mat{};
	XMMATRIX m_proj_mat{};
	XMMATRIX m_vp_mat{};
	XMINT2 m_world_size{};

	VertexShader m_vshader;
	PixelShader m_pshader;
	Mesh m_mesh;
	ConstantBuffer m_vp_buf;
	Generator m_noise_generator;

	void getInput();
	void renderGUI();
	void render();

	void regenerateMesh();
	void makeViewMatrix();
};