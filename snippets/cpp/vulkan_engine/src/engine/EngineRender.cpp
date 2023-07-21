#include "EngineRender.h"

#include "render/vulkan/VulRender.h"


namespace engine
{
	std::unique_ptr<Render> createInstance(RenderBackend backend)
	{
		switch (backend) {
		case RenderBackend::VULKAN: {
			vulk::Device::Settings deviceSettings = {
			  .deviceIndex = 0,
			  .layer = {"VK_LAYER_KHRONOS_validation"},
			  .swapchain_image_count = 3,
			};
			vulk::Render::Settings rendererSettings{};
			auto device = std::make_unique<vulk::Device>(deviceSettings);
			
			return std::make_unique<vulk::Render>(std::move(device), rendererSettings);
		}
		default: throw std::string{"cannot crate render backend"};
		}
	}

	//VertexHandle addVertex(const std::vector<Vertex>& vertex)
	//{
	//	return addVertex(std::span<Vertex>(vertex.cbegin(), vertex.cend()));
	//}
}