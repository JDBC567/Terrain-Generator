#include"world.hpp"
#include"window.hpp"
#include"imgui/imgui.h"
#include<vector>

#define DTR 0.0174533f

World::World(XMINT2 world_size)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> input_desc = {
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_SINT,
			0,
			12,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},};

	m_vshader = VertexShader(L"terrain_vertex.hlsl", input_desc);
	m_pshader = PixelShader(L"terrain_pixel.hlsl");

	m_world_size = world_size;

	std::vector<Vertex> vertex_data;
	std::vector<UINT> index_data;

	UINT index_count=0;

	const int ws_x = m_world_size.x * 32;
	const int ws_y = m_world_size.y * 32;

	for (int i = 0; i < ws_x; i++)
	{
		for (int j = 0; j < ws_y; j++)
		{
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 1.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 1.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 1.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 1.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));

			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 0.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 0.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 0.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), -0.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			//top
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 3);
			index_data.emplace_back(index_count + 2);
			//front
			index_data.emplace_back(index_count + 4);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 4);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 5);

			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 3);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 4);


			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 6);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 3);


			index_data.emplace_back(index_count + 5);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 5);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 6);

			index_count += 8;
		}
	}
	m_mesh = Mesh::createMesh(vertex_data.data(), vertex_data.size() * sizeof(Vertex), index_data.data(), index_data.size(), sizeof(Vertex));

	
	makeViewMatrix();
	m_proj_mat = XMMatrixPerspectiveFovLH(70.0f* 0.0174533f,static_cast<float>(Window::getSize().x)/static_cast<float>(Window::getSize().y),0.1f,1024.0f);
	m_vp_mat = XMMatrixMultiply(m_view_mat, m_proj_mat);

	ShaderData data{};
	data.vp_mat = m_vp_mat;
	data.position[0].x = m_position.x;
	data.position[0].y = m_position.y;
	data.position[0].z=m_position.z;
	data.position[0].x = 1.0f;

	m_vp_buf = ConstantBuffer(&data, sizeof(ShaderData));
}

void World::destroy()
{
	m_vshader.destroy();
	m_pshader.destroy();
	m_mesh.destroy();
	m_vp_buf.destroy();
}

void World::start()
{
	while(!Window::shouldClose())
	{
		Window::startFrame();

		if(m_noise_generator.renderUI())
		{
			ShaderData data{};
			data.vp_mat = m_vp_mat;
			data.position[0].x = m_position.x;
			data.position[0].y = m_position.y;
			data.position[0].z = m_position.z;
			data.position[0].w = m_noise_generator.m_height_scale;

			m_vp_buf.updateData(&data, sizeof(ShaderData), 0, true);
		}
		getInput();
		renderGUI();
		render();

		Window::endFrame();
	}
}

void World::getInput()
{
	static bool toggle_movement = false;
	static bool m_pressed = false;

	if (Window::isKeyPressed(GLFW_KEY_ESCAPE))
	{
		Window::setClose();
	}

	if ((Window::isKeyPressed(GLFW_KEY_M) || Window::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) && !m_pressed)
	{
		toggle_movement = !toggle_movement;
		if (toggle_movement)
		{
			GLFWwindow* raw_window = Window::getRawWindow();
			glfwSetInputMode(raw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetInputMode(raw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			GLFWwindow* raw_window = Window::getRawWindow();
			glfwSetInputMode(raw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetInputMode(raw_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}
	}

	m_pressed = (Window::isKeyPressed(GLFW_KEY_M) || Window::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT));

	if (toggle_movement)
	{
		const XMFLOAT2 dt = Window::getMouseDT();
		const float tdt = static_cast<float>(Window::getDT());
		if (!((m_rotation.x >= 89.0f && dt.y > 0.0f) || (m_rotation.x <= -89.0f && dt.y < 0.0f)))
		{
			m_rotation.x += m_rot_spd.x * dt.y * tdt;
		}
		else
		{
			if (m_rotation.x < -89.0f)
			{
				m_rotation.x = -89.0f;
			}
			else if (m_rotation.x > 89.0f)
			{
				m_rotation.x = 89.0f;
			}
		}

		m_rotation.y += m_rot_spd.y * dt.x * tdt;

		XMFLOAT3 mov_vec = XMFLOAT3(0.0f,0.0f,0.0f);
		if (Window::isKeyPressed(GLFW_KEY_W))
		{
			mov_vec.z = m_mov_spd.z;
		}
		else if (Window::isKeyPressed(GLFW_KEY_S))
		{
			mov_vec.z = -m_mov_spd.z;
		}
		if (Window::isKeyPressed(GLFW_KEY_A))
		{
			mov_vec.x = -m_mov_spd.x;
		}
		else if (Window::isKeyPressed(GLFW_KEY_D))
		{
			mov_vec.x = m_mov_spd.x;
		}
		if (Window::isKeyPressed(GLFW_KEY_SPACE))
		{
			mov_vec.y = m_mov_spd.y;
		}
		else if (Window::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
		{
			mov_vec.y = -m_mov_spd.y;
		}

		m_position.x += mov_vec.x * tdt;
		m_position.y += mov_vec.y * tdt;
		m_position.z += mov_vec.z * tdt;

		makeViewMatrix();
		m_vp_mat =XMMatrixMultiply(m_view_mat,m_proj_mat);

		ShaderData data{};
		data.vp_mat = m_vp_mat;
		data.position[0].x = m_position.x;
		data.position[0].y = m_position.y;
		data.position[0].z = m_position.z;
		data.position[0].w = m_noise_generator.m_height_scale;

		m_vp_buf.updateData(&data, sizeof(ShaderData), 0, true);
	}
}

void World::renderGUI()
{
	bool changed_view = false;
	ImGui::Begin("World Settings");
	ImGui::InputFloat3("movement speed", &m_mov_spd.x);
	ImGui::InputFloat2("rotation speed", &m_rot_spd.x);
	ImGui::Text("");
	if (ImGui::InputFloat3("position", &m_position.x))
	{
		changed_view = true;
	}
	if (ImGui::InputFloat2("rotation", &m_rotation.x))
	{
		changed_view = true;
	}
	const bool size_changed=ImGui::InputInt2("world size", &m_world_size.x);
	static bool wireframe;
	const bool mode_change = ImGui::Checkbox("Wireframe Mode", &wireframe);
	ImGui::End();

	if (size_changed)
	{
		regenerateMesh();
		m_noise_generator.updateSize(m_world_size);
	}

	if (Window::isResized())
	{
		m_proj_mat = XMMatrixPerspectiveFovLH(70.0f* 0.0174533f, static_cast<float>(Window::getSize().x) / static_cast<float>(Window::getSize().y),
			0.01f, 1024.0f);
	}

	if (changed_view)
	{
		makeViewMatrix();
	}

	if (Window::isResized() || changed_view)
	{
		m_vp_mat = XMMatrixMultiply(m_view_mat, m_proj_mat);

		ShaderData data{};
		data.vp_mat = m_vp_mat;
		data.position[0].x = m_position.x;
		data.position[0].y = m_position.y;
		data.position[0].z = m_position.z;
		data.position[0].w = m_noise_generator.m_height_scale;

		m_vp_buf.updateData(&data, sizeof(ShaderData), 0, true);
	}

	if(mode_change)
	{
		if(wireframe)
		{
			Window::setRendermodeWireframe();
		}
		else
		{
			Window::setRendermodeSolid();
		}
	}
}

void World::render()
{
	m_noise_generator.m_main_tex.bindVS(0);
	m_vshader.use();
	m_pshader.use();
	m_vp_buf.use(0);
	m_mesh.render();
}

void World::regenerateMesh()
{
	std::vector<Vertex> vertex_data;
	std::vector<UINT> index_data;

	UINT index_count = 0;

	const int ws_x = m_world_size.x * 32;
	const int ws_y = m_world_size.y * 32;

	for (int i = 0; i < ws_x; i++)
	{
		for (int j = 0; j < ws_y; j++)
		{
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 1.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 1.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 1.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 1.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));

			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), 0.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 0.0f, static_cast<float>(j)), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i) + 1.0f, 0.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			vertex_data.emplace_back(XMFLOAT3(static_cast<float>(i), -0.0f, static_cast<float>(j) + 1.0f), XMINT2(i, j));
			//top
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 3);
			index_data.emplace_back(index_count + 2);
			//front
			index_data.emplace_back(index_count + 4);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 4);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 5);

			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 3);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count);
			index_data.emplace_back(index_count + 4);


			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 6);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 7);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 3);


			index_data.emplace_back(index_count + 5);
			index_data.emplace_back(index_count + 1);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 5);
			index_data.emplace_back(index_count + 2);
			index_data.emplace_back(index_count + 6);

			index_count += 8;
		}
	}
	m_mesh = Mesh::createMesh(vertex_data.data(), vertex_data.size() * sizeof(Vertex), index_data.data(), index_data.size(), sizeof(Vertex));
}

void World::makeViewMatrix(){
	const XMMATRIX rotation_x = XMMatrixRotationX(m_rotation.x * DTR);
	const XMMATRIX rotation_y = XMMatrixRotationY(m_rotation.y * DTR);
	const XMMATRIX translate = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	m_view_mat = XMMatrixInverse(nullptr,XMMatrixMultiply(XMMatrixMultiply(rotation_x, rotation_y),translate));
}