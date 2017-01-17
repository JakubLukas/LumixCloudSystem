#include "cloud_system.h"
#include "cloud_scene.h"

#include "engine/engine.h"
#include "engine/iallocator.h"
#include "engine/base_proxy_allocator.h"
#include "engine/property_register.h"
#include "engine/property_descriptor.h"
#include "engine/log.h"
#include "engine/universe/universe.h"


namespace Lumix
{

static void registerProperties(Lumix::IAllocator& allocator)
{
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec3, CloudScene>)("Size",
			&CloudScene::getCloudSize,
			&CloudScene::setCloudSize));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec3, CloudScene>)("Cell Count",
			&CloudScene::getCloudCellCount,
			&CloudScene::setCloudCellCount));
}


	struct CloudSystemImpl LUMIX_FINAL : public CloudSystem
	{
		explicit CloudSystemImpl(Engine& engine)
			: m_engine(engine)
		{
			registerProperties(engine.getAllocator());
		}


		~CloudSystemImpl()
		{
		}


		Engine& getEngine() override { return m_engine; }


		const char* getName() const override { return "LumixCloudSystem"; }


		void createScenes(Universe& universe) override
		{
			auto* scene = CloudScene::createInstance(*this, universe, m_engine.getAllocator());
			universe.addScene(scene);
		}


		void destroyScene(IScene* scene) override
		{
			CloudScene::destroyInstance(static_cast<CloudScene*>(scene));
		}


		Engine& m_engine;
	};


	LUMIX_PLUGIN_ENTRY(LumixCloudSystem)
	{
		return LUMIX_NEW(engine.getAllocator(), Lumix::CloudSystemImpl)(engine);
	}


} // namespace Lumix

