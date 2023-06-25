#pragma once
#include<unordered_map>
#include"imgui/imgui.h"
#include"noise_layer.hpp"


class Generator
{
public:
	Generator();
	~Generator()=default;

	bool renderUI();
	void update();

	void addLayerSimplex(const char* name);
	void removeLayerSimplex(int num);

	void updateSize(XMINT2 size);

	Texture2D m_main_tex;
	float m_height_scale=1.0f;
private:
	std::unordered_map<int,std::pair<SimplexNoise,bool>> m_simplex_layers;
	ComputeShader m_user_program;
	XMINT2 m_size={8,8};
};