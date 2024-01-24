#pragma once

#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"

#include <memory>
#include <glm/glm.hpp>

namespace RayTracingApp
{
	class Renderer
	{
	public:
		Renderer() = default;

		void Render(const Camera& camera);
		void OnResize(uint32_t width, uint32_t height);
		std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
	private:

		glm::vec4 TraceRay(const Ray& ray);

		std::shared_ptr<Walnut::Image> finalImage;
		uint32_t* imageData = nullptr;

	};

}