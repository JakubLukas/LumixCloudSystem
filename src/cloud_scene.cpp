#include "cloud_scene.h"

#include "engine/iallocator.h"


namespace Lumix
{


	struct CloudSceneImpl LUMIX_FINAL : public CloudScene
	{
		enum class Version
		{
			CLOUDS = 0,

			LATEST
		};

		CloudSceneImpl(CloudSystem& system, Universe& universe, IAllocator& allocator)
			: m_system(system)
			, m_universe(universe)
			, m_allocator(allocator)
		{

		}

		ComponentHandle createComponent(ComponentType, Entity) override { return INVALID_COMPONENT; }; //TODO
		void destroyComponent(ComponentHandle component, ComponentType type) override { }; //TODO
		void serialize(OutputBlob& serializer) override { }; //TODO
		void serialize(ISerializer& serializer) override { }
		void deserialize(IDeserializer& serializer) override { }
		void deserialize(InputBlob& serializer) override { }; //TODO

		IPlugin& getPlugin() const override { return m_system; };

		void update(float time_delta, bool paused) override { }; //TODO
		ComponentHandle getComponent(Entity entity, ComponentType type) override { return INVALID_COMPONENT; };////////////////////////////////////////////

		Universe& getUniverse() override { return m_universe; }

		void startGame() override { }
		void stopGame() override { }
		int getVersion() const override { return (int)Version::LATEST; }
		void clear() override { }; //TODO

		IAllocator& m_allocator;
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
	}


} // namespace Lumix

