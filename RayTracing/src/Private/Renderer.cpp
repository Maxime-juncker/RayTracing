#include "../Public/Renderer.h"
#include "Walnut/Random.h"
#include <iostream>
#include <execution>

namespace RayTracingApp
{
	namespace Utils
	{
		uint32_t ConvertToRGBA(const glm::vec4& color)
		{
			uint8_t r = (uint8_t)(color.r * 255.0f);
			uint8_t g = (uint8_t)(color.g * 255.0f);
			uint8_t b = (uint8_t)(color.b * 255.0f);
			uint8_t a = (uint8_t)(color.a * 255.0f);

			// Converting to uint32 (RBGA -> AGBR) ex : ffff0000 = blue
			uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
			return result;
		}

		// Random number generator
		static uint32_t PCG_Hash(uint32_t input)
		{
			uint32_t state = input * 747796405u + 2891336453u;
			uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
			return (word >> 22u) ^ word;
		}

		static float RandomFloat(uint32_t& seed)
		{
			seed = PCG_Hash(seed);
			return (float)seed / (float)UINT32_MAX;
		}

		static glm::vec3 InUnitSphere(uint32_t& seed)
		{
			return glm::normalize(glm::vec3(
				RandomFloat(seed) * 2.0f - 1.0f, 
				RandomFloat(seed) * 2.0f - 1.0f, 
				RandomFloat(seed) * 2.0f - 1.0f));
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

		delete[] accumulationData;
		accumulationData = new glm::vec4[width * height];

		imageHorizontalIter.resize(width);
		imageVerticalIter.resize(height);

		for (uint32_t i = 0; i < width; i++)
			imageHorizontalIter[i] = i;
		
		for (uint32_t i = 0; i < height; i++)
			imageVerticalIter[i] = i;
		
	}

	void Renderer::Render(const Scene& scene, const Camera& camera)
	{
		activeScene = &scene;
		activeCamera = &camera;
		if (frameIndex == 1)
		{
			memset(accumulationData, 0, 
				finalImage->GetWidth() * finalImage->GetHeight() * sizeof(glm::vec4));
		}

		// Render every pixels
		if (settings.Multithreading)
		{
			std::for_each(std::execution::par, imageVerticalIter.begin(), imageVerticalIter.end(),
				[this](uint32_t y)
				{
					for (uint32_t x = 0; x < finalImage->GetWidth(); x++)
					{
						glm::vec4 color = PerPixel(x, y);
						accumulationData[x + y * finalImage->GetWidth()] += color;

						glm::vec4 accumulatedColor = accumulationData[x + y * finalImage->GetWidth()];
						accumulatedColor /= (float)frameIndex;

						accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0), glm::vec4(1));
						imageData[x + y * finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
					}
				});
		}
		else
		{
			for (uint32_t y = 0; y < finalImage->GetHeight(); y++)
			{
				for (uint32_t x = 0; x < finalImage->GetWidth(); x++)
				{
					glm::vec4 color = PerPixel(x,y);
					accumulationData[x + y * finalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = accumulationData[x + y * finalImage->GetWidth()];
					accumulatedColor /= (float)frameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0), glm::vec4(1));
					imageData[x + y * finalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
				}
			}

		}

		finalImage->SetData(imageData);

		if (settings.Accumulate)
			frameIndex++;
		else
			frameIndex = 1;
	}

	glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
	{
		Ray ray;
		ray.Origin = activeCamera->GetPosition();
		ray.Direction = activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()];

		glm::vec3 light(0.0f);
		glm::vec3 contribution(1.0f);

		uint32_t seed = x + y * finalImage->GetWidth();
		seed *= frameIndex;

		int bounces = 5;
		for (int i = 0; i < bounces; i++)
		{
			seed += i;

			Renderer::HitPayload payload = TraceRay(ray);
			if (payload.HitDistance < 0) // Sky color
			{
				glm::vec3 skyColor(0.6f, 0.7f, 0.9f);
				light += skyColor * contribution;
				break;
			}

			glm::vec3 lighDir = glm::normalize(glm::vec3(-1, -1, -1));
			float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lighDir), 0.0f); // == cos(angle)

			const Sphere& sphere = activeScene->Spheres[payload.OjectIndex];
			const Material& material = activeScene->Materials[sphere.MaterialIndex];

			// Outputing the color
			contribution *= material.Albedo;
			light += material.GetEmission();

			ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
			if (settings.SlowRandom)
				ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
			else
				ray.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));
		}
		return glm::vec4(light, 1);
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
