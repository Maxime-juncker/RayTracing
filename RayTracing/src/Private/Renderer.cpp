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

	void Renderer::Render(const Scene& scene, const Camera& camera)
	{
		activeScene = &scene;
		activeCamera = &camera;

		// Render every pixels
		for (uint32_t y = 0; y < finalImage->GetHeight(); y++)
		{
			for (uint32_t x = 0; x < finalImage->GetWidth(); x++)
			{
				glm::vec4 color = PerPixel(x,y);
				imageData[x + y * finalImage->GetWidth()] = Utils::ConvertToRGBA(color);
			}
		}

		finalImage->SetData(imageData);
	}

	glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
	{
		Ray ray;
		ray.Origin = activeCamera->GetPosition();
		ray.Direction = activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()];

		glm::vec3 color(0.0f);
		float multiplier = 1.0f;

		int bounces = 2;
		for (int i = 0; i < bounces; i++)
		{
			Renderer::HitPayload payload = TraceRay(ray);
			if (payload.HitDistance < 0) // Sky color
			{
				glm::vec3 skyColor(0.0f, 0.0f, 0.0f);
				color += skyColor * multiplier;
				break;
			}

			glm::vec3 lighDir = glm::normalize(glm::vec3(-1, -1, -1));
			float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lighDir), 0.0f); // == cos(angle)

			const Sphere& sphere = activeScene->Spheres[payload.OjectIndex];
			// Outputing the color
			glm::vec3 sphereColor = sphere.Albedo;
			sphereColor *= lightIntensity;
			color += sphereColor;

			multiplier *= .7f;

			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
			ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
		}
		return glm::vec4(color, 1);
	}

	Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
	{
		//(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance

		int closestSphere = -1;
		float hitDistance = FLT_MAX;

		for (size_t i = 0; i < activeScene->Spheres.size(); i++)
		{
			const Sphere& sphere = activeScene->Spheres[i];
			glm::vec3 origin = ray.Origin - sphere.Position;

			float a = glm::dot(ray.Direction, ray.Direction);
			float b = 2.0f * glm::dot(origin, ray.Direction);
			float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

			// descriminant (b^2 - 4ac)
			// t = (b^2 +- sqrt(det)) / (2a)
			float discriminant = b * b - 4.0f * a * c;
			if (discriminant < 0.0f)
				continue;

			// Getting instersections points
			//float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
			float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
			if (closestT < hitDistance && closestT > 0.0f)
			{
				hitDistance = closestT;
				closestSphere = (int)i;
			}
		}

		if (closestSphere < 0) // Didn't hit any sphere
			return Miss(ray);

		return ClosestHit(ray, hitDistance, closestSphere);
	}

	Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
	{
		const Sphere& closestSphere = activeScene->Spheres[objectIndex];
		Renderer::HitPayload payload;

		payload.HitDistance = hitDistance;
		payload.OjectIndex = objectIndex;

		glm::vec3 origin = ray.Origin - closestSphere.Position;
		payload.WorldPosition = origin + ray.Direction * hitDistance;
		payload.WorldNormal = glm::normalize(payload.WorldPosition);

		payload.WorldPosition += closestSphere.Position;

		return payload;
	}

	Renderer::HitPayload Renderer::Miss(const Ray& ray)
	{
		Renderer::HitPayload payload;
		payload.HitDistance = -1;
		return payload;
	}
}
