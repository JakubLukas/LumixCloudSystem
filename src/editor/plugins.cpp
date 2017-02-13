#include "../cloud_system.h"

#include "engine/universe/component.h"
#include "editor/world_editor.h"
#include "editor/studio_app.h"
#include "editor/property_grid.h"
#include "engine/property_register.h"
#include "imgui/imgui.h"

#include "../cloud_scene.h"


static const Lumix::ComponentType CLOUD_TYPE = Lumix::PropertyRegister::getComponentType("cloud");


struct SimulatorPlugin LUMIX_FINAL : public PropertyGrid::IPlugin
{
	explicit SimulatorPlugin(StudioApp& app)
		: m_app(app)
		, m_simulation_updating(true)
		, m_simulation_timescale(1.0f)
	{
	}


	void onGUI(PropertyGrid& grid, Lumix::ComponentUID cmp) override
	{
		if(cmp.type != CLOUD_TYPE) return;

		ImGui::Separator();
		ImGui::Checkbox("Update", &m_simulation_updating);
		auto* scene = static_cast<Lumix::CloudScene*>(cmp.scene);
		ImGui::SameLine();
		if(ImGui::Button("Reset")) scene->restartSimulation(cmp.handle);

		/*if(m_simulation_updating)
		{
			ImGui::DragFloat("Timescale", &m_particle_emitter_timescale, 0.01f, 0.01f, 10000.0f);
			float time_delta = m_app.getWorldEditor()->getEngine().getLastTimeDelta();
			scene->updateEmitter(cmp.handle, time_delta * m_particle_emitter_timescale);
			scene->getParticleEmitter(cmp.handle)->drawGizmo(*m_app.getWorldEditor(), *scene);
		}*/
	}


	StudioApp& m_app;
	float m_simulation_timescale;
	bool m_simulation_updating;
};


LUMIX_STUDIO_ENTRY(LumixCloudSystem)
{
	auto& allocator = app.getWorldEditor()->getAllocator();

	app.registerComponent("cloud", "Cloud blob");

	auto& property_grid = *app.getPropertyGrid();
	property_grid.addPlugin(*LUMIX_NEW(allocator, SimulatorPlugin)(app));
}
