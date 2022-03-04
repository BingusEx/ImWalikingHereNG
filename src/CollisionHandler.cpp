#include "CollisionHandler.h"

#include "Events.h"

bool DialogueCollider::ShouldIgnoreCollision(RE::TESObjectREFR*)
{
	const auto menuHandler = Events::MenuOpenCloseHandler::GetSingleton();
	return menuHandler->IsInDialogue();
}
