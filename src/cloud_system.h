#pragma once

#include "engine/iplugin.h"


namespace Lumix
{


	class CloudSystem : public IPlugin
	{
	public:
		virtual Engine& getEngine() = 0;
	};


} // namespace Lumix
