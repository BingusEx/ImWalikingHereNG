#include "Events.h"
#include "Hooks.h"

using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kDataLoaded:
                Hooks::Install();
                Events::Install();
                break;
        }
    }
}

SKSEPluginLoad(const LoadInterface* skse) {

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);

    auto message = SKSE::GetMessagingInterface();

    if (!message->RegisterListener(MessageHandler)) {
        return false;
    }

    Settings::load();
    log::info("SKSEPluginLoad OK", plugin->GetName());

    return true;
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 7, 0, 0 },
	.Name = "ImWalkingHere",
	.Author = "BingusEx",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);
