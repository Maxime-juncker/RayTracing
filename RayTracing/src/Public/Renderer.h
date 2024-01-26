#pragma once

#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

namespace RayTracingApp
{
	class Renderer
	{
	public:
		struct Settings
		{
			bool Accumulate = true;
			bool Multithreading = true;
			bool SlowRandom = false;
		};

	public:
		Renderer() = default;

		void Render(const Scene& scene, const Camera& camera);
		void OnResize(uint32_t width, uint32_t height);
		std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

		void ResetFrameIndex() { frameIndex = 1; }
		Settings& GetSettings() { return settings; }
		int GetAccumulatedFrameIndex() { return frameIndex; }
	private:

		struct HitPayload
		{
			float HitDistance;
			glm::vec3 WorldPosition;
			glm::vec3 WorldNormal;

			uint32_t OjectIndex;
		};

		glm::vec4 PerPixel(uint32_t x, uint32_t y); // Ray gen

		HitPayload TraceRay(const Ray& ray);
		HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
		HitPayload Miss(const Ray& ray);
	private:
		const Scene* activeScene = nullptr;
		const Camera* activeCamera = nullptr;

		std::vector<uint32_t> imageHorizontalIter, imageVerticalIter;

		Settings settings;

		std::shared_ptr<Walnut::Image> finalImage;

		uint32_t* imageData = nullptr;
		glm::vec4* accumulationData = nullptr;

		uint32_t frameIndex = 1;
	};

}