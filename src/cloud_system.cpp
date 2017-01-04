#include "cloud_system.h"

#include "engine/engine.h"
#include "engine/iallocator.h"
#include "engine/property_register.h"
#include "engine/log.h"


namespace Lumix
{


	struct CloudSystemImpl LUMIX_FINAL : public CloudSystem
	{
		explicit CloudSystemImpl(Engine& engine)
			: m_engine(engine)
		{
		}


		~CloudSystemImpl()
		{
		}


		Engine& getEngine() override { return m_engine; }


		const char* getName() const override { return "cloud_system"; }


		void update(float deltaTime) override
		{
			//g_log_info.log("CloudSystem") << "update";
		}


		Engine& m_engine;
	};


	LUMIX_PLUGIN_ENTRY(LumixCloudSystem)
	{
		return LUMIX_NEW(engine.getAllocator(), Lumix::CloudSystemImpl)(engine);
	}


} // namespace Lumix

