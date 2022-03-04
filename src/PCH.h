#pragma once

#include <atomic>
#include <memory>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

#pragma warning(push)
#include <AutoTOML.hpp>
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
using namespace std::literals;

namespace util
{
	using SKSE::stl::not_null;
	using SKSE::stl::report_and_fail;
}

#include "Plugin.h"
#include "Settings.h"
