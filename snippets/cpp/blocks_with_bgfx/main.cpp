#include <iostream>
#include <cstdint>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
      
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;


static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};



constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;

int main()
{
    //glfw window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_WIDTH, "Vulkan", nullptr, nullptr);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	bgfx::Init init;
	init.platformData.nwh = glfwGetWin32Window(window);
	init.resolution.width = (uint32_t)width;
	init.resolution.height = (uint32_t)height;
	init.resolution.reset = BGFX_RESET_VSYNC;
	init.type = bgfx::RendererType::OpenGL; 
	if (!bgfx::init(init))
		return 1;

	const bgfx::ViewId main_view_id = 0;
	bgfx::setViewClear(main_view_id, BGFX_CLEAR_COLOR, 0x000000FF);

	// Create vertex stream declaration.
	PosColorVertex::init();

	// Create static vertex buffer.
	bgfx::VertexBufferHandle m_vbh = bgfx::createVertexBuffer(
		bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)) // Static data can be passed with bgfx::makeRef
		, PosColorVertex::ms_layout
	);

	while (!glfwWindowShouldClose(window )) {
		std::cout << "next" << std::endl;
	}

    return 0; 
}