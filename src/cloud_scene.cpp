#include "cloud_scene.h"

#include "engine/universe/universe.h"
#include "engine/iallocator.h"
#include "engine/hash_map.h"
#include "engine/property_register.h"
#include "engine/serializer.h"
#include "engine/blob.h"

#include "renderer/render_scene.h"
#include "engine/crc32.h"


//////////////////////////
#include "lucky_cloud/VolumetricCloud.h"
#include "lucky_cloud/CloudParticle.h"


namespace Lumix
{

static const ComponentType CLOUD_TYPE = PropertyRegister::getComponentType("cloud");


struct Node
{
	u8 value;
};


static const Node EMPTY_NODE = { 0 };


struct Cloud
{
	Entity entity;
	Vec3 size;
	int cell_count_x;
	int cell_count_y;
	int cell_count_z;
	Array<Node> nodes;
	CVolumetricCloud luckyCloud;

	Cloud(IAllocator& allocator)
		: nodes(allocator)
	{}
};


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
			, m_clouds(allocator)
		{
			//m_universe.entityTransformed().bind<NavigationSceneImpl, &NavigationSceneImpl::onEntityMoved>(this);
			universe.registerComponentType(CLOUD_TYPE, this, &CloudSceneImpl::serializeCloud, &CloudSceneImpl::deserializeCloud);
		}

		void clear() override
		{
			m_clouds.clear();
		};


		ComponentHandle createComponent(ComponentType type, Entity entity) override
		{
			if(type == CLOUD_TYPE)
			{
				m_clouds.insert(entity, Cloud(m_allocator));

				Cloud& cloud = m_clouds[entity];
				cloud.entity = entity;
				cloud.size = Vec3(10, 5, 10);
				cloud.cell_count_x = 10;
				cloud.cell_count_y = 5;
				cloud.cell_count_z = 10;
				setupCloudNodeCount(cloud);

				Environment enviroment{ Lumix::Vec3(0.01f, 0, 0) };
				CloudProperties cloudProps{
					10,
					10,
					5,
					1,
					0.1f,
					Lumix::Vec3(0, 0, 0)
				};
				cloud.luckyCloud.Setup(&enviroment, &cloudProps);

				ComponentHandle cmp = { entity.index };
				m_universe.addComponent(entity, type, this, cmp);
				return cmp;
			}
			return INVALID_COMPONENT;
		};

		void destroyComponent(ComponentHandle component, ComponentType type) override
		{
			if(type == CLOUD_TYPE)
			{
				Entity entity = { component.index };
				auto iter = m_clouds.find(entity);
				Cloud& cloud = iter.value();
				m_clouds.erase(iter);
				m_universe.destroyComponent(entity, type, this, component);
			}
			else
			{
				ASSERT(false);
			}
		};

		ComponentHandle getComponent(Entity entity, ComponentType type) override
		{
			if(type == CLOUD_TYPE)
			{
				auto iter = m_clouds.find(entity);
				const Cloud& cloud = iter.value();
				return{ entity.index };
			}
			return INVALID_COMPONENT;
		}


		int getVersion() const override { return (int)Version::LATEST; }
		IPlugin& getPlugin() const override { return m_system; };
		Universe& getUniverse() override { return m_universe; }


		void startGame() override { } //TODO
		void stopGame() override { } //TODO

		void debugDraw()
		{
			auto render_scene = static_cast<RenderScene*>(m_universe.getScene(crc32("renderer")));
			if(!render_scene) return;

			Entity camera = m_universe.getFirstEntity();
			do
			{
				if(strcmp(m_universe.getEntityName(camera), "editor_camera") == 0)
					break;
				camera = m_universe.getNextEntity(camera);
			}
			while(isValid(camera));
			Vec3 dir = m_universe.getRotation(camera).rotate(Vec3(0, 0, 1));

			for(auto iter = m_clouds.begin(), end = m_clouds.end(); iter != end; ++iter)
			{
				Cloud& cloud = iter.value();
				/*Vec3 pos = m_universe.getPosition(cloud.entity);
				Vec3 cell_size(
					cloud.size.x / cloud.cell_count_x,
					cloud.size.y / cloud.cell_count_y,
					cloud.size.z / cloud.cell_count_z
				);
				float sphere_radius = Math::minimum(Math::minimum(cell_size.x, cell_size.y), cell_size.z) * 0.2f;
				for(int z = 0, cz = cloud.cell_count_z; z < cz; ++z)
					for(int y = 0, cy = cloud.cell_count_y; y < cy; ++y)
						for(int x = 0, cx = cloud.cell_count_x; x < cx; ++x)
						{
							Vec3 nodePos = pos + Vec3(x*cell_size.x, y*cell_size.y, z*cell_size.z);
							render_scene->addDebugCircle(nodePos, dir, sphere_radius, 0x0f00aaff, 0);
						}*/


				CParticleEnumerator Enumerator(&cloud.luckyCloud.m_ParticlePool);
				CloudParticle *pCurParticle = Enumerator.NextParticle();
				while (pCurParticle)
				{
					u32 color = 0xff000000
						+ (u32(pCurParticle->m_cScatteringColor.x * 0xff) << 16)
						+ (u32(pCurParticle->m_cScatteringColor.y * 0xff) << 8)
						+ (u32(pCurParticle->m_cScatteringColor.z * 0xff));
					render_scene->addDebugCircle(*pCurParticle->GetPosition(), dir, 0.4f, color, 0);
					pCurParticle = Enumerator.NextParticle();
				}
			}
		}

		void update(float time_delta, bool paused) override
		{
			for(auto iter = m_clouds.begin(), end = m_clouds.end(); iter != end; ++iter)
				iter.value().luckyCloud.AdvanceTime(time_delta, 0);
			debugDraw();
		}


		void setupCloudNodeCount(Cloud& cloud)
		{
			cloud.nodes.resize(((int)cloud.cell_count_x + 1) * ((int)cloud.cell_count_y + 1) * ((int)cloud.cell_count_z + 1));
			for(int i = 0, c = cloud.nodes.size(); i < c; ++i)
				cloud.nodes[i] = EMPTY_NODE;
		}


		void serialize(OutputBlob& serializer) override
		{
			int count = m_clouds.size();
			serializer.write(count);
			for(auto iter = m_clouds.begin(), end = m_clouds.end(); iter != end; ++iter)
			{
				serializer.write(iter.key());
				serializer.write(iter.value().size);
				serializer.write(iter.value().cell_count_x);
				serializer.write(iter.value().cell_count_y);
				serializer.write(iter.value().cell_count_z);
			}
		};

		void deserialize(InputBlob& serializer) override
		{
			int count = 0;
			serializer.read(count);
			m_clouds.rehash(count);
			for(int i = 0; i < count; ++i)
			{
				Cloud cloud(m_allocator);
				serializer.read(cloud.entity);
				serializer.read(cloud.size);
				serializer.read(cloud.cell_count_x);
				serializer.read(cloud.cell_count_y);
				serializer.read(cloud.cell_count_z);
				setupCloudNodeCount(cloud);

				m_clouds.insert(cloud.entity, cloud);
				ComponentHandle cmp = { cloud.entity.index };
				m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
			}
		};


		void serializeCloud(ISerializer& serializer, ComponentHandle cmp)
		{
			Cloud& cloud = m_clouds[{cmp.index}];
			serializer.write("size", cloud.size);
			serializer.write("cell_count_x", cloud.cell_count_x);
			serializer.write("cell_count_y", cloud.cell_count_y);
			serializer.write("cell_count_z", cloud.cell_count_z);
		}


		void deserializeCloud(IDeserializer& serializer, Entity entity)
		{
			Cloud cloud(m_allocator);
			cloud.entity = entity;
			serializer.read(&cloud.size);
			serializer.read(&cloud.cell_count_x);
			serializer.read(&cloud.cell_count_y);
			serializer.read(&cloud.cell_count_z);
			setupCloudNodeCount(cloud);

			m_clouds.insert(cloud.entity, cloud);
			ComponentHandle cmp = { cloud.entity.index };
			m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
		}


		void setCloudSize(ComponentHandle cmp, const Vec3& size) override
		{
			m_clouds[{cmp.index}].size = size;
		}

		Vec3 getCloudSize(ComponentHandle cmp) override
		{
			return m_clouds[{cmp.index}].size;
		}


		void setCloudCellCount(ComponentHandle cmp, const Vec3& count) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].cell_count_x = (int)count.x;
			m_clouds[entity].cell_count_y = (int)count.y;
			m_clouds[entity].cell_count_z = (int)count.z;
		}
		
		Vec3 getCloudCellCount(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			Vec3 count(
				(float)m_clouds[entity].cell_count_x,
				(float)m_clouds[entity].cell_count_y,
				(float)m_clouds[entity].cell_count_z);
			return count;
		}



		IAllocator& m_allocator;
		Universe& m_universe;
		CloudSystem& m_system;
		HashMap<Entity, Cloud> m_clouds;
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

