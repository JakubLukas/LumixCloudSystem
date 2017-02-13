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
		virtual void setCloudCellSpace(ComponentHandle cmp, const Vec3& count) = 0;
		virtual Vec3 getCloudCellSpace(ComponentHandle cmp) = 0;
		virtual void setCloudHumidityProbability(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudHumidityProbability(ComponentHandle cmp) = 0;
		virtual void setCloudActiveProbability(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudActiveProbability(ComponentHandle cmp) = 0;
		virtual void setCloudExtensionProbability(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudExtensionProbability(ComponentHandle cmp) = 0;
		virtual void setCloudExtinctionTime(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudExtinctionTime(ComponentHandle cmp) = 0;
		virtual void restartSimulation(ComponentHandle cmp) = 0;

	};


} // namespace Lumix
