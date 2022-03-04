#pragma once

#include "CollisionHandler.h"

namespace Hooks
{
	inline void Install()
	{
		CollisionHandler::GetSingleton()->Install();
		logger::debug("Installed all hooks"sv);
	}
}
