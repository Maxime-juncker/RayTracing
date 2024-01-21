#include "../Public/Renderer.h"
#include "Walnut/Random.h"
#include <iostream>

namespace RayTracingApp
{
	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		if (finalImage)
		{
			// No resize needed
			if (finalImage->GetWidth() == width && finalImage->GetHeight() == height)
				return;

			finalImage->Resize(width, height);
		}
		else
		{
			finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
		}

		delete[] imageData;
		imageData = new uint32_t[width * height];
	}

	void Renderer::Render()
	{
		// Render every pixels
		for (uint32_t y = 0; y < finalImage->GetHeight(); y++)
		{
			for (uint32_t x = 0; x < finalImage->GetWidth(); x++)
			{
				glm::vec2 coord =
				{ 
					(float)x / (float)finalImage->GetWidth(), 
					(float)y / (float)finalImage->GetHeight() 
				};
				coord = coord * 2.0f - 1.0f; // Remap from 0 -> 1 to -1 -> 1;

				imageData[x + y * finalImage->GetWidth()] = PerPixel(coord);
			}
		}

		finalImage->SetData(imageData);
	}

	uint32_t Renderer::PerPixel(glm::vec2 coord)
	{
		glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
		glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
		float radius = 0.5f;
		//rayDirection = glm::normalize(rayDirection);

		//(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance

		float a = glm::dot(rayDirection, rayDirection);
		float b = 2.0f * glm::dot(rayOrigin, rayDirection);
		float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

		// descriminant (b^2 - 4ac)
		float discriminant = b * b - 4.0f * a * c;

		if (discriminant >= 0.0f)
			return 0xffff00ff;

		return 0xff000000;
	}
}
