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
		virtual void setCloudExtinctionProbability(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudExtinctionProbability(ComponentHandle cmp) = 0;
		virtual void setCloudExtinctionTime(ComponentHandle cmp, const float value) = 0;
		virtual float getCloudExtinctionTime(ComponentHandle cmp) = 0;

		virtual void setViewSamplesCount(ComponentHandle cmp, const unsigned int count) = 0;
		virtual unsigned int getViewSamplesCount(ComponentHandle cmp) = 0;
		virtual void setLightSamplesCount(ComponentHandle cmp, const unsigned int count) = 0;
		virtual unsigned int getLightSamplesCount(ComponentHandle cmp) = 0;
		virtual void setSunPosition(ComponentHandle cmp, const Vec3& pos) = 0;
		virtual Vec3 getSunPosition(ComponentHandle cmp) = 0;
		virtual void setSunColor(ComponentHandle cmp, const Vec4& color) = 0;
		virtual Vec4 getSunColor(ComponentHandle cmp) = 0;
		virtual void setShadeColor(ComponentHandle cmp, const Vec4& color) = 0;
		virtual Vec4 getShadeColor(ComponentHandle cmp) = 0;

		virtual void restartSimulation(ComponentHandle cmp) = 0;

	};


} // namespace Lumix
