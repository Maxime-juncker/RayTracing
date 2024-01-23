#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>

namespace RayTracingApp
{
	class Renderer
	{
	public:
		Renderer() = default;

		void Render();
		void OnResize(uint32_t width, uint32_t height);
		std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
	private:

		glm::vec4 PerPixel(glm::vec2 coord);

		std::shared_ptr<Walnut::Image> finalImage;
		uint32_t* imageData = nullptr;

	};

}