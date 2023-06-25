#include "generator.hpp"
#include"window.hpp"
#include"shader.hpp"
#include"mesh.hpp"
#include"Texture2D.hpp"
#include"World.hpp"
#include"noise_layer.hpp"
#include"imgui/imgui.h"

int main()
{
	Window window = Window();

	SimplexNoise::init();

	World world = World(XMINT2(8, 8));

	world.start();

	return 0;
}