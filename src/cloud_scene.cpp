#include "cloud_scene.h"

#include "engine/universe/universe.h"
#include "engine/iallocator.h"
#include "engine/associative_array.h"
#include "engine/property_register.h"
#include "engine/serializer.h"
#include "engine/blob.h"

#include "renderer/render_scene.h"
#include "engine/crc32.h"


#include "simulation/simulation.h"
#include "render/renderer.h"


namespace Lumix
{

static const ComponentType CLOUD_TYPE = PropertyRegister::getComponentType("cloud");


struct Cloud
{
	Entity entity;
	Vec3 cellSpace;
	CldSim::Simulation simulation;
	CldSim::CloudRenderer renderer;
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
				Cloud& cloud = m_clouds.insert(entity);

				cloud.entity = entity;
				cloud.cellSpace = Vec3(10, 10, 10);
				cloud.simulation.Setup(30, 10, 30);
				cloud.renderer.Setup(30, 10, 30);

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
				m_clouds.erase(entity);
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
				if (m_clouds.find(entity) < 0)
					return INVALID_COMPONENT;
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

			Vec3 pos;

			for (const Cloud& cloud : m_clouds)
			{
				/*const bool* humSpace = cloud.simulation.GetHumiditySpace();
				const bool* actSpace = cloud.simulation.GetActiveSpace();
				const bool* cldSpace = cloud.simulation.GetCloudSpace();
				for(CldSim::uint x = 0; x < cloud.simulation.GetWidth(); ++x)
				{
					nodePos.x = x * cloud.cellSpace.x;
					for(CldSim::uint y = 0; y < cloud.simulation.GetHeight(); ++y)
					{
						nodePos.y = y * cloud.cellSpace.y;
						for(CldSim::uint z = 0; z < cloud.simulation.GetLength(); ++z)
						{
							nodePos.z = z * cloud.cellSpace.z;

							bool hum = humSpace[cloud.simulation.GetIndex(x, y, z)];
							bool act = actSpace[cloud.simulation.GetIndex(x, y, z)];
							bool cld = cldSpace[cloud.simulation.GetIndex(x, y, z)];
							u32 color = 0xff000000
								+ (u32(cld * 0xff) << 16)
								+ (u32(cld * 0xff) << 8)
								+ (u32(cld * 0xff));
							render_scene->addDebugPoint(nodePos, color, 0);
						}
					}
				}*/
				CldSim::uint size =
					cloud.simulation.GetWidth()
					* cloud.simulation.GetHeight()
					* cloud.simulation.GetLength();
				for(CldSim::uint i = 0; i < size; ++i)
				{
					const auto& p = cloud.renderer.GetParticles()[i];
					pos.x = p.position.x;
					pos.y = p.position.y;
					pos.z = p.position.z;
					u32 color = 0xff000000
						+ (u32(1 * 0xff) << 16)
						+ (u32(p.color.y * 0xff) << 8)
						+ (u32(p.color.z * 0xff));
					render_scene->addDebugPoint(pos, color, 0);
				}
			}
		}

		void update(float time_delta, bool paused) override
		{
			if(paused)
				return;

			for(Cloud& cloud : m_clouds)
			{
				cloud.simulation.Update(time_delta);
				cloud.renderer.CalcParticleColors(cloud.simulation.GetCloudSpace());
			}

			debugDraw();
		}


		void serialize(OutputBlob& serializer) override
		{
			int count = m_clouds.size();
			serializer.write(count);
			for(const Cloud& cloud : m_clouds)
			{
				serializer.write(cloud.entity);
				//serializer.write(cloud.size);
				serializer.write(cloud.cellSpace);
			}
		};

		void deserialize(InputBlob& serializer) override
		{
			int count = 0;
			serializer.read(count);
			m_clouds.reserve(count);
			for(int i = 0; i < count; ++i)
			{
				Cloud cloud;
				serializer.read(cloud.entity);
				//serializer.read(cloud.size);
				serializer.read(cloud.cellSpace);

				m_clouds.insert(cloud.entity, cloud);
				ComponentHandle cmp = { cloud.entity.index };
				m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
			}
		};


		void serializeCloud(ISerializer& serializer, ComponentHandle cmp)
		{
			Cloud& cloud = m_clouds[{cmp.index}];
			//serializer.write("size", cloud.size);
			serializer.write("cell_count_x", cloud.cellSpace.x);
			serializer.write("cell_count_y", cloud.cellSpace.y);
			serializer.write("cell_count_z", cloud.cellSpace.z);
		}


		void deserializeCloud(IDeserializer& serializer, Entity entity, int)
		{
			Cloud cloud;
			cloud.entity = entity;
			//serializer.read(&cloud.size);
			serializer.read(&cloud.cellSpace.x);
			serializer.read(&cloud.cellSpace.y);
			serializer.read(&cloud.cellSpace.z);

			m_clouds.insert(cloud.entity, cloud);
			ComponentHandle cmp = { cloud.entity.index };
			m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
		}


		void setCloudSize(ComponentHandle cmp, const Vec3& size) override
		{
			Entity entity = { cmp.index };
			//m_clouds[entity].size = size;
			//m_clouds[entity].luckyCloud.m_fWidth = size.x;
			//m_clouds[entity].luckyCloud.m_fHigh = size.y;
			//m_clouds[entity].luckyCloud.m_fLength = size.z;
		}

		Vec3 getCloudSize(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return Vec3(
				(float)m_clouds[entity].simulation.GetWidth(),
				(float)m_clouds[entity].simulation.GetHeight(),
				(float)m_clouds[entity].simulation.GetLength());
		}


		void setCloudCellSpace(ComponentHandle cmp, const Vec3& count) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].cellSpace.x = count.x;
			m_clouds[entity].cellSpace.y = count.y;
			m_clouds[entity].cellSpace.z = count.z;
		}
		
		Vec3 getCloudCellSpace(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			Vec3 count(
				m_clouds[entity].cellSpace.x,
				m_clouds[entity].cellSpace.y,
				m_clouds[entity].cellSpace.z);
			return count;
		}


		void setCloudHumidityProbability(ComponentHandle cmp, const float value) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].simulation.SetHumidityProbability(value);
		}

		float getCloudHumidityProbability(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.GetHumidityProbability();
		}


		void setCloudActiveProbability(ComponentHandle cmp, const float value) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].simulation.SetActiveProbability(value);
		}

		float getCloudActiveProbability(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.GetActiveProbability();
		}


		void setCloudExtinctionProbability(ComponentHandle cmp, const float value) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].simulation.SetExtinctionProbability(value);
		}

		float getCloudExtinctionProbability(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.GetExtinctionProbability();
		}


		void setCloudExtinctionTime(ComponentHandle cmp, const float value) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].simulation.SetExtinctionTime(value);
		}

		float getCloudExtinctionTime(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.GetExtinctionTime();
		}


		void restartSimulation(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.Restart();
		}


		IAllocator& m_allocator;
		Universe& m_universe;
		CloudSystem& m_system;
		AssociativeArray<Entity, Cloud> m_clouds;
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

