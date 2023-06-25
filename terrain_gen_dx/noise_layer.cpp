#include"noise_layer.hpp"
#include"imgui/imgui.h"

void* NoiseLayer::getTexture()
{
	return m_texture.getRawPointer();
}

void NoiseLayer::bindTexture(UINT unit)
{
	m_texture.bindCS(unit);
}

void NoiseLayer::unbindTexture(UINT unit)
{
	m_texture.unbindCS(unit);
}

void NoiseLayer::setName(char* name)
{
	memcpy(&m_name[0], name, 50);
}

char* NoiseLayer::getName()
{
	return &m_name[0];
}

void NoiseLayer::setSize(XMINT2 size)
{
	m_size = size;
	m_texture.destroy();
	m_texture = Texture2D(XMINT2(m_size.x * 32, m_size.y * 32));

	generate();
}

XMINT2 NoiseLayer::getSize()
{
	return m_size;
}

ComputeShader SimplexNoise::m_compute_shader=ComputeShader();
int SimplexNoise::m_counter = 0;

SimplexNoise::SimplexNoise(const char* name)
{
	memcpy(&m_name[0], name, 50);
	_id = m_counter;
	m_counter += 1;

	SNCB t{};
	t.ns[0] = m_noise_scale.x;
	t.ns[1] = m_noise_scale.y;
	t.no[0] = m_noise_offset.x;
	t.no[1] = m_noise_offset.y;
	t.seed = m_seed;

	m_cbuffer = ConstantBuffer(&t, 32);
	m_texture = Texture2D(XMINT2(m_size.x * 32, m_size.y * 32));

	SimplexNoise::generate();
}

void SimplexNoise::generate()
{
	m_cbuffer.useCS(0);
	m_texture.bindCS(0);
	m_compute_shader.dispatch(XMINT3(m_size.x, m_size.y, 1));
	Texture2D::unbindCS(0);
}
#include<iostream>
bool SimplexNoise::renderUI()
{
	ImGui::Begin(m_name);
	const bool should_close=ImGui::ArrowButton("", 0);
	const bool scale_changed = ImGui::InputFloat2("Scale", &m_noise_scale.x);
	const bool offset_changed = ImGui::InputFloat2("Offset", &m_noise_offset.x);
	const bool size_changed = ImGui::InputInt2("Size", &m_size.x);
	const bool seed_changed = ImGui::InputInt("Seed", &m_seed);

	ImGui::Image(m_texture.getRawPointer(),ImVec2(m_size.x*32,m_size.y*32));

	ImGui::End();

	if (size_changed)
	{
		m_texture.destroy();
		m_texture = Texture2D(XMINT2(m_size.x * 32, m_size.y * 32));

		generate();
	}

	if (scale_changed || offset_changed || seed_changed)
	{
		SNCB t{};
		t.ns[0] = m_noise_scale.x;
		t.ns[1] = m_noise_scale.y;
		t.no[0] = m_noise_offset.x;
		t.no[1] = m_noise_offset.y;
		t.seed = m_seed;

		m_cbuffer.updateData(&t, 32, 0, true);
		generate();
	}

	return should_close;
}

void SimplexNoise::init()
{
	m_compute_shader = ComputeShader(L"simplex_n.hlsl");
}