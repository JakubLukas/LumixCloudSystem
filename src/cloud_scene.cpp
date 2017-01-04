#include "cloud_scene.h"

#include "engine/iallocator.h"


namespace Lumix
{


	/*struct CloudSceneImpl LUMIX_FINAL : public CloudScene
	{

		ComponentHandle createComponent(ComponentType, Entity) override { }; //TODO
		void destroyComponent(ComponentHandle component, ComponentType type) override { }; //TODO
		void serialize(OutputBlob& serializer) override { }; //TODO
		void serialize(ISerializer& serializer) override { }
		void deserialize(IDeserializer& serializer) override { }
		void deserialize(InputBlob& serializer) override { }; //TODO

		IPlugin& getPlugin() const override { return m_system; };

		void update(float time_delta, bool paused) override { }; //TODO
		ComponentHandle getComponent(Entity entity, ComponentType type) override { };////////////////////////////////////////////

		Universe& getUniverse() override { return m_universe; }

		void startGame() override { }
		void stopGame() override { }
		int getVersion() const override { return -1; }
		void clear() override { }; //TODO

		Universe& m_universe;
		CloudSystem& m_system;
	};


	CloudScene* CloudScene::createInstance(CloudSystem& system,
		Universe& universe,
		class IAllocator& allocator)
	{
		return LUMIX_NEW(allocator, CloudSceneImpl)(system, universe, allocator);
	}

	void CloudScene::destroyInstance(CloudScene* scene)
	{
		LUMIX_DELETE(static_cast<CloudSceneImpl*>(scene)->m_allocator, scene);
	}*/


} // namespace Lumix

