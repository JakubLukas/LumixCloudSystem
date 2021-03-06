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

static const ResourceType MATERIAL_TYPE("material");


static void registerProperties(Lumix::IAllocator& allocator)
{
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec3, CloudScene>)("Size",
			&CloudScene::getCloudSize,
			&CloudScene::setCloudSize));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec3, CloudScene>)("Cell Count",
			&CloudScene::getCloudCellSpace,
			&CloudScene::setCloudCellSpace));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, DecimalPropertyDescriptor<CloudScene>)("Humidity probability",
			&CloudScene::getCloudHumidityProbability,
			&CloudScene::setCloudHumidityProbability,
			0.0f, 0.1f, 0.005f));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, DecimalPropertyDescriptor<CloudScene>)("Active probability",
			&CloudScene::getCloudActiveProbability,
			&CloudScene::setCloudActiveProbability,
			0.0f, 0.1f, 0.005f));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, DecimalPropertyDescriptor<CloudScene>)("Extension probability",
			&CloudScene::getCloudExtinctionProbability,
			&CloudScene::setCloudExtinctionProbability,
			0.0f, 0.1f, 0.005f));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, DecimalPropertyDescriptor<CloudScene>)("Extinction Time",
			&CloudScene::getCloudExtinctionTime,
			&CloudScene::setCloudExtinctionTime,
			0.0f, 10.0f, 0.1f));
	auto* viewSampDesc = LUMIX_NEW(allocator, IntPropertyDescriptor<CloudScene>)("View samples",
		&CloudScene::getViewSamplesCount,
		&CloudScene::setViewSamplesCount);
	viewSampDesc->setLimit(0, 200);
	PropertyRegister::add("cloud", viewSampDesc);
	auto* lightSampDesc = LUMIX_NEW(allocator, IntPropertyDescriptor<CloudScene>)("Light samples",
		&CloudScene::getLightSamplesCount,
		&CloudScene::setLightSamplesCount);
	lightSampDesc->setLimit(0, 200);
	PropertyRegister::add("cloud", lightSampDesc);
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec3, CloudScene>)("Sun position",
			&CloudScene::getSunPosition,
			&CloudScene::setSunPosition));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec4, CloudScene>)("Sun color",
			&CloudScene::getSunColor,
			&CloudScene::setSunColor));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, SimplePropertyDescriptor<Vec4, CloudScene>)("Shade color",
			&CloudScene::getShadeColor,
			&CloudScene::setShadeColor));
	PropertyRegister::add("cloud",
		LUMIX_NEW(allocator, ResourcePropertyDescriptor<CloudScene>)("Material",
			&CloudScene::getCloudMaterialPath,
			&CloudScene::setCloudMaterialPath,
			"Material (*.mat)",
			MATERIAL_TYPE));
}


	struct CloudSystemImpl LUMIX_FINAL : public CloudSystem
	{
		explicit CloudSystemImpl(Engine& engine)
			: m_engine(engine)
		{
			registerProperties(engine.getAllocator());

			CloudScene::registerLuaAPI(m_engine.getState());
		}


		~CloudSystemImpl()
		{
		}


		Engine& getEngine() override { return m_engine; }


		const char* getName() const override { return "LumixCloudSystem"; }


		void createScenes(Universe& universe) override
		{
			auto* scene = CloudScene::createInstance(*this, m_engine, universe, m_engine.getAllocator());
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

