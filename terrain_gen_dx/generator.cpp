#include"generator.hpp"
#include <iostream>
#include <ranges>
#include<format>
#include"imgui/imgui.h"

Generator::Generator()
{
	m_main_tex = Texture2D(XMINT2(256, 256));
}

bool Generator::renderUI()
{
	ImGui::Begin("Noise Settings");

	const bool reload=ImGui::Button("Reload User Program");
	ImGui::SameLine();
	static char prog_buf[1000]="";
	ImGui::InputText("User Program Path", &prog_buf[0], 1000);
	if(reload)
	{
		m_user_program = ComputeShader(std::wstring(&prog_buf[0], &prog_buf[strlen(prog_buf)]));
		update();
	}

	if(ImGui::Button("Regenerate"))
	{
		update();
	}

	bool changed_height=ImGui::InputFloat("Height Scale", &m_height_scale);

	const bool add=ImGui::Button("Add Simplex Layer: ");
	ImGui::SameLine();
	static char buf[50]="";
	ImGui::InputText("##layer name", &buf[0], 50);
	if(add)
	{
		addLayerSimplex(&buf[0]);
		buf[0] = '\0';
	}

	for(auto& val : m_simplex_layers | std::views::values)
	{
		const bool remove=ImGui::ArrowButton(std::format("##{:d}_",val.first._id).c_str(), 0);
		ImGui::SameLine();
		ImGui::Text(val.first.getName());
		ImGui::SameLine();
		const bool open=ImGui::ArrowButton(std::format("##{:d}",val.first._id).c_str(), val.second ? 0 : 1);
		if(open && val.second)
		{
			val.second = false;
		}
		else if(open)
		{
			val.second = true;
		}
		if (val.second)
		{
			val.second = !val.first.renderUI();
		}
		if(remove)
		{
			m_simplex_layers.erase(val.first._id);
		}
	}

	ImGui::Image(m_main_tex.getRawPointer(), ImVec2(m_size.x * 32, m_size.y * 32));

	ImGui::End();

	return changed_height;
}

void Generator::update()
{
	m_main_tex.bindCS(0);
	int count = 1;
	for(auto& val : m_simplex_layers | std::views::values)
	{
		val.first.bindTexture(count++);
	}
	m_user_program.dispatch(XMINT3(m_size.x,m_size.y,1));
	Texture2D::unbindCS(0);
	for (int i = 1; i < count; i++)
	{
		Texture2D::unbindCS(i);
	}
}

void Generator::addLayerSimplex(const char* name)
{
	auto layer = SimplexNoise(name);
	m_simplex_layers.insert(std::pair(layer._id,std::pair(layer,false)));
}

void Generator::removeLayerSimplex(int id)
{
	m_simplex_layers.erase(id);
	m_main_tex.bindCS(0);
	int count = 1;
	for (auto& val : m_simplex_layers | std::views::values)
	{
		val.first.bindTexture(count++);
	}
	m_user_program.dispatch(XMINT3(m_size.x, m_size.y, 1));
	Texture2D::unbindCS(0);
	for (int i = 1; i < count;i++)
	{
		Texture2D::unbindCS(i);
	}
}

void Generator::updateSize(XMINT2 size)
{
	m_main_tex.destroy();
	m_main_tex = Texture2D(XMINT2(size.x*32,size.y*32));
	m_size = size;
	for(auto& val:m_simplex_layers | std::views::values)
	{
		val.first.setSize(size);
	}
	m_main_tex.bindCS(0);
	int count = 1;
	for (auto& val : m_simplex_layers | std::views::values)
	{
		val.first.bindTexture(count++);
	}
	m_user_program.dispatch(XMINT3(m_size.x, m_size.y, 1));
	Texture2D::unbindCS(0);
	for (int i = 1; i < count; i++)
	{
		Texture2D::unbindCS(i);
	}
}