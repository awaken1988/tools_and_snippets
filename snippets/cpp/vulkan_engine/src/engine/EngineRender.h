#pragma once

#include "EngineTypes.h"

namespace engine
{
	class Render
	{
	public:
        static std::unique_ptr<Render> createInstance(RenderBackend);

		virtual ~Render();

		//-----------------------------------------
		//interface
		//-----------------------------------------
		virtual VertexHandle addVertex(const std::span<Vertex> vertex) = 0;
		virtual DrawableHandle addDrawable() = 0;
		virtual void setViewProjection(const glm::mat4& view, const glm::mat4& projection);
		virtual void setWorldTransform(const DrawableHandle& handle, const glm::mat4& transform) = 0;
		virtual void draw() = 0;

		//-----------------------------------------
		//helper
		//-----------------------------------------
		//virtual VertexHandle addVertex(const std::vector<Vertex>& vertex) final;
    };

    
}