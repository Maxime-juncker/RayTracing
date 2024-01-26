#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Public/Renderer.h"
#include "Public/Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;
using namespace RayTracingApp;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : camera(45.0f, 0.1f, 100.0f) 
	{
		// Creating materials
		Material& basicSphere = scene.Materials.emplace_back();
		basicSphere.Albedo = { 0.0f, 1.0f, 0.2f };
		basicSphere.Roughness = 0.6f;

		Material& pinkSphere = scene.Materials.emplace_back();
		pinkSphere.Albedo = { 0.6f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.6f;

		Material& ground = scene.Materials.emplace_back();
		ground.Albedo = { 0.7f, 0.7f, 0.7f };
		ground.Roughness = 0.1f;

		Material& sun = scene.Materials.emplace_back();
		sun.Albedo = { 0.8f, 0.5f, 0.2f };
		sun.Roughness = 0.1f;
		sun.EmissionColor = sun.Albedo;
		sun.EmissionPower = 3.5f;

		// Instantiating spheres
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;

			scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { -3.3f, 0.1f, 1.2f };
			sphere.Radius = 1.4f;
			sphere.MaterialIndex = 1;

			scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 2;

			scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 42.0f, 4.0f, 2.0f };
			sphere.Radius = 20.0f;
			sphere.MaterialIndex = 3;

			scene.Spheres.push_back(sphere);
		}

	}

	virtual void OnUpdate(float ts) override
	{
		
		if (camera.OnUpdate(ts))
			renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", lastRenderTime);
		ImGui::Text("current accumulated frame %d", renderer.GetAccumulatedFrameIndex());
		ImGui::Text("width: %d", viewportWidth);
		ImGui::Text("height: %d", viewportHeight);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Acculumate", &renderer.GetSettings().Accumulate);
		ImGui::Checkbox("Multithrading", &renderer.GetSettings().Multithreading);
		ImGui::Checkbox("Slow random", &renderer.GetSettings().SlowRandom);

		if (ImGui::Button("Reset"))
		{
			renderer.ResetFrameIndex();
		}

		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 
				1.0f, 0.0f, (int)scene.Materials.size() - 1);


			ImGui::Separator();
			ImGui::PopID();
		}
		for (size_t i = 0; i < scene.Materials.size(); i++)
		{
			ImGui::PushID(i);

			Material& material = scene.Materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);

			ImGui::ColorEdit3("Emision color", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emision Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		viewportWidth = ImGui::GetContentRegionAvail().x;
		viewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = renderer.GetFinalImage();
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(),
				{ (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0,1), ImVec2(1,0) );
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		renderer.OnResize(viewportWidth, viewportHeight);
		camera.OnResize(viewportWidth, viewportHeight);
		renderer.Render(scene, camera);

		lastRenderTime = timer.ElapsedMillis();
	}
	
private:
	Renderer renderer;
	uint32_t viewportWidth = 0, viewportHeight = 0;
	Camera camera;
	Scene scene;
	float lastRenderTime = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Jen Tracing";
	spec.Height = 700;
	spec.Width = 950;

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}