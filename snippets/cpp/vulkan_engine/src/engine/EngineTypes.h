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
#include <cmath>


namespace engine
{
	enum class RenderBackend {
		VULKAN,
	};

	struct HandleBase
	{
		explicit HandleBase(size_t index)
			: index{ index } {}

		HandleBase() = default;
		HandleBase(const HandleBase&) = default;


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

	//some math helper
	template<typename T>
	bool betweenStartEnd(T start, T end, T value) {
		return value >= start && value < end;
	}

	template<typename T>
	bool betweenStartSize(T start, T size, T value) {
		return betweenStartEnd(start, start + size, value);
	}

	namespace primitive
	{
		const auto l = -0.5f;
		const auto h = 0.5f;

		static std::vector<engine::Vertex> trianglePrimitive()
		{
			return {
				{{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, -0.5f, 1.0f},  {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.5f, 1.0f},   {1.0f, 1.0f, 1.0f}},
			};
		}

		static std::vector<engine::Vertex> rectanglePrimitive()
		{
			return {
				 {{l, l, 1.0f}, {1.0f, 0.0f, 0.0f}},
				 {{h, l, 1.0f}, {0.0f, 1.0f, 0.0f}},
				 {{h, h, 1.0f}, {1.0f, 1.0f, 1.0f}},

				 {{h, h, 1.0f}, {1.0f, 1.0f, 1.0f}},
				 {{l, h, 1.0f}, {1.0f, 1.0f, 1.0f}},
				 {{l, l, 1.0f}, {1.0f, 0.0f, 0.0f}},
			};
		}
	}
}