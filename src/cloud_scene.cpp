#include "cloud_scene.h"

#include "engine/universe/universe.h"
#include "engine/iallocator.h"
#include "engine/associative_array.h"
#include "engine/property_register.h"
#include "engine/serializer.h"
#include "engine/blob.h"
#include "engine/engine.h"
#include "engine/resource.h"
#include "engine/resource_manager_base.h"
#include "engine/resource_manager.h"

#include "renderer/render_scene.h"
#include "renderer/material.h"
#include "renderer/shader.h"
#include "engine/crc32.h"


#include "simulation/simulation.h"
#include "render/renderer.h"
#include "renderer/pipeline.h"

#include "engine/lua_wrapper.h"


namespace Lumix
{

static const ComponentType CLOUD_TYPE = PropertyRegister::getComponentType("cloud");
static const ResourceType MATERIAL_TYPE("material");


struct Cloud
{
	Entity entity;
	Vec3 cellSpace;
	CldSim::Simulation simulation;
	CldSim::CloudRenderer renderer;
	Material* material = nullptr;
	bool isSimulating = false;
	float timescale = 1.0f;
};


struct BaseVertex //TODO
{
	float x, y, z;
	u32 rgba;
	float u;
	float v;
};


	struct CloudSceneImpl LUMIX_FINAL : public CloudScene
	{
		enum class Version
		{
			CLOUDS = 0,

			LATEST
		};


		CloudSceneImpl(CloudSystem& system, Engine& engine, Universe& universe, IAllocator& allocator)
			: m_system(system)
			, m_engine(engine)
			, m_universe(universe)
			, m_allocator(allocator)
			, m_clouds(allocator)
		{
			//m_universe.entityTransformed().bind<NavigationSceneImpl, &NavigationSceneImpl::onEntityMoved>(this);
			universe.registerComponentType(CLOUD_TYPE, this, &CloudSceneImpl::serializeCloud, &CloudSceneImpl::deserializeCloud);

			createCloudBuffers();
			m_cloud_matrix_uniform = bgfx::createUniform("u_cloudMatrix", bgfx::UniformType::Mat4);
		}

		~CloudSceneImpl()
		{
			bgfx::destroyUniform(m_cloud_matrix_uniform);
			bgfx::destroyVertexBuffer(m_particle_vertex_buffer);
			bgfx::destroyIndexBuffer(m_particle_index_buffer);

			auto* manager = m_engine.getResourceManager().get(MATERIAL_TYPE);
			for(Cloud& cloud : m_clouds)
			{
				if(cloud.material != nullptr)
					manager->unload(*cloud.material);
			}
		}

		void clear() override
		{
			auto* manager = m_engine.getResourceManager().get(MATERIAL_TYPE);
			for(Cloud& cloud : m_clouds)
			{
				if(cloud.material != nullptr)
					manager->unload(*cloud.material);
			}

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
					u32 color =
						(u32(p.color.a * 0xff) << 24)
						+ (u32(p.color.r * 0xff) << 16)
						+ (u32(p.color.g * 0xff) << 8)
						+ (u32(p.color.b * 0xff));
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
				if(cloud.isSimulating)
				{
					cloud.simulation.Update(time_delta * cloud.timescale);
					cloud.renderer.CalcParticleColors(cloud.simulation.GetCloudSpace());
				}
			}
		}


		void serialize(OutputBlob& serializer) override
		{
			int count = m_clouds.size();
			serializer.write(count);
			for(const Cloud& cloud : m_clouds)
			{
				serializer.write(cloud.entity);
				serializer.write(cloud.cellSpace);
				serializer.write(Vec3((float)cloud.simulation.GetWidth(), (float)cloud.simulation.GetHeight(), (float)cloud.simulation.GetLength()));
				const Material* material = cloud.material;
				serializer.writeString(material ? material->getPath().c_str() : "");
				serializer.write(cloud.timescale);
			}
		};

		void deserialize(InputBlob& serializer) override
		{
			ResourceManager manager = m_engine.getResourceManager();
			auto material_manager = manager.get(MATERIAL_TYPE);

			int count = 0;
			serializer.read(count);
			m_clouds.reserve(count);
			for(int i = 0; i < count; ++i)
			{
				Entity entity;
				serializer.read(entity);

				Cloud& cloud = m_clouds.insert(entity);
				cloud.entity = entity;
				serializer.read(cloud.cellSpace);
				Vec3 dimension;
				serializer.read(dimension);
				cloud.simulation.Setup((CldSim::uint)dimension.x, (CldSim::uint)dimension.y, (CldSim::uint)dimension.z);
				cloud.renderer.Setup((CldSim::uint)dimension.x, (CldSim::uint)dimension.y, (CldSim::uint)dimension.z);
				char path[MAX_PATH_LENGTH];
				serializer.readString(path, lengthOf(path));
				if(path[0] != '\0')
					cloud.material = static_cast<Material*>(material_manager->load(Path(path)));
				serializer.read(cloud.timescale);

				ComponentHandle cmp = { entity.index };
				m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
			}
		};


		void serializeCloud(ISerializer& serializer, ComponentHandle cmp)
		{
			Cloud& cloud = m_clouds[{cmp.index}];
			serializer.write("cellSpace", cloud.cellSpace);
			serializer.write("size", Vec3((float)cloud.simulation.GetWidth(), (float)cloud.simulation.GetHeight(), (float)cloud.simulation.GetLength()));
			const Material* material = cloud.material;
			serializer.write("material", material ? material->getPath().c_str() : "");
			serializer.write("timescale", cloud.timescale);
		}


		void deserializeCloud(IDeserializer& serializer, Entity entity, int)
		{
			ResourceManagerBase* material_manager = m_engine.getResourceManager().get(MATERIAL_TYPE);

			Cloud& cloud = m_clouds.insert(entity);
			cloud.entity = entity;
			serializer.read(&cloud.cellSpace);
			Vec3 dimension;
			serializer.read(&dimension);
			cloud.simulation.Setup((CldSim::uint)dimension.x, (CldSim::uint)dimension.y, (CldSim::uint)dimension.z);
			cloud.renderer.Setup((CldSim::uint)dimension.x, (CldSim::uint)dimension.y, (CldSim::uint)dimension.z);
			char path[MAX_PATH_LENGTH];
			serializer.read(path, lengthOf(path));
			if(path[0] != '\0')
				cloud.material = static_cast<Material*>(material_manager->load(Path(path)));
			serializer.read(&cloud.timescale);

			ComponentHandle cmp = { cloud.entity.index };
			m_universe.addComponent(cloud.entity, CLOUD_TYPE, this, cmp);
		}


		void setCloudSize(ComponentHandle cmp, const Vec3& size) override
		{
			Entity entity = { cmp.index };
			Cloud& cloud = m_clouds[entity];

			cloud.simulation.Clear();
			cloud.simulation.Setup((CldSim::uint)size.x, (CldSim::uint)size.y, (CldSim::uint)size.z);
			cloud.renderer.Clear();
			cloud.renderer.Setup((CldSim::uint)size.x, (CldSim::uint)size.y, (CldSim::uint)size.z);
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


		void setViewSamplesCount(ComponentHandle cmp, const int count) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].renderer.SetViewSamplesCount(count);
		}

		int getViewSamplesCount(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].renderer.GetViewSamplesCount();
		}


		void setLightSamplesCount(ComponentHandle cmp, const int count) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].renderer.SetLightSamplesCount(count);
		}

		int getLightSamplesCount(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].renderer.GetLightSamplesCount();
		}


		void setSunPosition(ComponentHandle cmp, const Vec3& pos) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].renderer.SetSunPosition({ pos.x, pos.y, pos.z });
		}

		Vec3 getSunPosition(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			CldSim::Vec3 pos = m_clouds[entity].renderer.GetSunPosition();
			return Vec3(pos.x, pos.y, pos.z);
		}


		void setSunColor(ComponentHandle cmp, const Vec4& color) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].renderer.SetSunColor({ color.w, color.x, color.y, color.z });
		}

		Vec4 getSunColor(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			CldSim::Color col = m_clouds[entity].renderer.GetSunColor();
			return Vec4(col.r, col.g, col.b, col.a);
		}


		void setShadeColor(ComponentHandle cmp, const Vec4& color) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].renderer.SetShadeColor({ color.w, color.x, color.y, color.z });
		}

		Vec4 getShadeColor(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			CldSim::Color col = m_clouds[entity].renderer.GetShadeColor();
			return Vec4(col.r, col.g, col.b, col.a);
		}


		void setCloudMaterialPath(ComponentHandle cmp, const Path& path) override
		{
			Entity entity = { cmp.index };

			auto* manager = m_engine.getResourceManager().get(MATERIAL_TYPE);

			Material* material = m_clouds[entity].material;
			if(material != nullptr)
				manager->unload(*material);

			m_clouds[entity].material = static_cast<Material*>(manager->load(path));
		}


		Path getCloudMaterialPath(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };

			Cloud& cloud = m_clouds[entity];
			if (cloud.material == nullptr)
				return Path("");

			return cloud.material->getPath();
		}


		void restartSimulation(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].simulation.Restart();
		}


		void setIsSimulating(ComponentHandle cmp, const bool isSimulating) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].isSimulating = isSimulating;
		}

		bool getIsSimulating(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].isSimulating;
		}


		void setSimulationTimescale(ComponentHandle cmp, const float timescale) override
		{
			Entity entity = { cmp.index };
			m_clouds[entity].timescale = timescale;
		}

		float getSimulationTimescale(ComponentHandle cmp) override
		{
			Entity entity = { cmp.index };
			return m_clouds[entity].timescale;
		}


		void createCloudBuffers() //TODO
		{
			const BaseVertex vertices[] = {
				{ -1, -1, 1, 0xffffffff, 0, 0 },
				{ -1,  1, 1, 0xffffffff, 0, 1 },
				{ 1,  1, 1, 0xffffffff, 1, 1 },
				{ 1, -1, 1, 0xffffffff, 1, 0 },
			};

			bgfx::VertexDecl m_base_vertex_decl;
			m_base_vertex_decl.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.end();

			const bgfx::Memory* vertex_mem = bgfx::copy(vertices, sizeof(vertices));
			m_particle_vertex_buffer = bgfx::createVertexBuffer(vertex_mem, m_base_vertex_decl);

			const u16 indices[] = { 0, 1, 2, 0, 2, 3 };
			const bgfx::Memory* index_mem = bgfx::copy(indices, sizeof(indices));
			m_particle_index_buffer = bgfx::createIndexBuffer(index_mem);
		}


		void renderClouds(Pipeline* pipeline)
		{
			for(const Cloud& cloud : m_clouds)
			{
				Material* material = cloud.material;
				if(material == nullptr)
					continue;//TODO

				Matrix mtx = m_universe.getMatrix(cloud.entity);

				struct Instance
				{
					Vec4 pos;
					Vec4 color;
				};

				int count =
					cloud.simulation.GetWidth()
					* cloud.simulation.GetHeight()
					* cloud.simulation.GetLength();
				const CldSim::CloudRenderer::Particle* particle = cloud.renderer.GetParticles();

				const bgfx::InstanceDataBuffer* instance_buffer = bgfx::allocInstanceDataBuffer(count, sizeof(Instance));
				Instance* instance = (Instance*)instance_buffer->data;
				for(int i = 0, c = count; i < c; ++i)
				{
					instance->pos = Vec4(particle->position.x, particle->position.y, particle->position.z, 1.0f);
					instance->color = Vec4(particle->color.r, particle->color.g, particle->color.b, particle->color.a);
					++instance;
					++particle;
				}


				bgfx::setUniform(m_cloud_matrix_uniform, &mtx);
				pipeline->render(m_particle_vertex_buffer, m_particle_index_buffer, *instance_buffer, count, *material);
			}
		}


		IAllocator& m_allocator;
		Universe& m_universe;
		CloudSystem& m_system;
		Engine& m_engine;
		AssociativeArray<Entity, Cloud> m_clouds;

		bgfx::VertexBufferHandle m_particle_vertex_buffer; //TODO
		bgfx::IndexBufferHandle m_particle_index_buffer; //TODO
		bgfx::UniformHandle m_cloud_matrix_uniform;
	};


	CloudScene* CloudScene::createInstance(CloudSystem& system,
		Engine& engine,
		Universe& universe,
		class IAllocator& allocator)
	{
		return LUMIX_NEW(allocator, CloudSceneImpl)(system, engine, universe, allocator);
	}

	void CloudScene::destroyInstance(CloudScene* scene)
	{
		LUMIX_DELETE(static_cast<CloudSceneImpl*>(scene)->m_allocator, scene);
	}

	void CloudScene::registerLuaAPI(lua_State* L)
	{
		auto registerCFunction = [L](const char* name, lua_CFunction function)
		{
			lua_pushcfunction(L, function);
			lua_setglobal(L, name);
		};

		#define REGISTER_FUNCTION(name) \
		do {\
			auto f = &LuaWrapper::wrapMethod<CloudSceneImpl, decltype(&CloudSceneImpl::name), &CloudSceneImpl::name>; \
			registerCFunction(#name, f); \
		} while(false) \

		REGISTER_FUNCTION(renderClouds);

		#undef REGISTER_FUNCTION
	}


} // namespace Lumix

