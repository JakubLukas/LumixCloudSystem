#pragma once

#include "engine/iplugin.h"
#include "cloud_system.h"
#include "engine/vec.h"


namespace Lumix
{


	class CloudScene : public IScene
	{
	public:

		static CloudScene* createInstance(CloudSystem& system,
			Universe& universe,
			class IAllocator& allocator);

		static void destroyInstance(CloudScene* scene);

		virtual void setCloudSize(ComponentHandle cmp, const Vec3& size) = 0;
		virtual Vec3 getCloudSize(ComponentHandle cmp) = 0;
		virtual void setCloudCellCount(ComponentHandle cmp, const Vec3& count) = 0;
		virtual Vec3 getCloudCellCount(ComponentHandle cmp) = 0;
		virtual void setEvolutionSpeed(ComponentHandle cmp, const float speed) = 0;
		virtual float getEvolutionSpeed(ComponentHandle cmp) = 0;

	};


} // namespace Lumix
