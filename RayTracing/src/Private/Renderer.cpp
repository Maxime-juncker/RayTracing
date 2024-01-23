#include "../Public/Renderer.h"
#include "Walnut/Random.h"
#include <iostream>

namespace RayTracingApp
{
	namespace Utils
	{
		uint32_t ConvertToRGBA(const glm::vec4& color)
		{
			glm::clamp(color, glm::vec4(0), glm::vec4(1));
			uint8_t r = (color.r * 255.0f);
			uint8_t g = (color.g * 255.0f);
			uint8_t b = (color.b * 255.0f);
			uint8_t a = (color.a * 255.0f);

			// Converting to uint32 (RBGA -> AGBR) ex : ffff0000 = blue
			uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
			return result;
		}
	}

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

				glm::vec4 color = PerPixel(coord);
				imageData[x + y * finalImage->GetWidth()] = Utils::ConvertToRGBA(color);
			}
		}

		finalImage->SetData(imageData);
	}

	glm::vec4 Renderer::PerPixel(glm::vec2 coord)
	{
		float aspectRatio = finalImage->GetWidth() / (float)finalImage->GetHeight();
		coord.x *= aspectRatio;

		glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
		glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
		float radius = 0.5f;

		//(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance

		float a = glm::dot(rayDirection, rayDirection);
		float b = 2.0f * glm::dot(rayOrigin, rayDirection);
		float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

		// descriminant (b^2 - 4ac)
		// t = (b^2 +- sqrt(det)) / (2a)
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			return glm::vec4(0, 0, 0, 1);


		// Getting instersections points
		float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;

		// Calculating the normal
		glm::vec3 normal = glm::normalize(hitPoint);

		glm::vec3 lighDir = glm::normalize(glm::vec3(-1, -1, -1));

		float d = glm::max(glm::dot(normal, -lighDir), 0.0f); // == cos(angle)

		// Outputing the color
		glm::vec3 sphereColor(1, 0, 1);
		sphereColor *= d;
		return glm::vec4(sphereColor,1);

	}
}
