#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Public/Renderer.h"

using namespace Walnut;
using namespace RayTracingApp;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", lastRenderTime);
		ImGui::Text("width: %d", viewportWidth);
		ImGui::Text("height: %d", viewportHeight);
		if (ImGui::Button("Render"))
		{
			Render();
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
		renderer.Render();

		lastRenderTime = timer.ElapsedMillis();
	}
	
private:
	Renderer renderer;
	uint32_t viewportWidth = 0, viewportHeight = 0;

	float lastRenderTime = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Jen Tracing";
	spec.Height = 600;
	spec.Width = 800;

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