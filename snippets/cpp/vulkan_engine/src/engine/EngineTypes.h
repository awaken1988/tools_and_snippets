#pragma once

//include all base types we need
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <string>
#include <format>
#include <vector>
#include <array>
#include <set>
#include <span>
#include <optional>
#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <format>
#include <cstring>
#include <fstream>


namespace engine
{
	enum class RenderBackend {
		VULKAN,
	};

	struct HandleBase
	{
		explicit HandleBase(size_t index)
			: index{ index } {}

		auto isValid(){
			return std::numeric_limits<size_t>::max();
		}

		size_t index = std::numeric_limits<size_t>::max();
	};

	struct VertexHandle : public HandleBase {
		using HandleBase::HandleBase;
	};
	struct DrawableHandle : public HandleBase {
		using HandleBase::HandleBase;
	};
	struct TextureHandle : public HandleBase {
		using HandleBase::HandleBase;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;	//TODO: later we only use Textures 
	};
}