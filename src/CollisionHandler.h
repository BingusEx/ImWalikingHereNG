#pragma once

class ICollider
{
public:
	static constexpr auto COLLISION_FLAG = static_cast<std::uint32_t>(RE::CFilter::Flag::kNoCollision);

	ICollider() = default;
	virtual ~ICollider() = default;

	void PreCollide(util::not_null<RE::Actor*> a_actor, RE::TESObjectREFR* a_colRef)
	{
		if (ShouldIgnoreCollision(a_colRef)) {
			SetCollisionObject(a_actor);
			SetCollisionOnObject(false);
		}
	}

	void PostCollide()
	{
		if (_collisionObj) {
			SetCollisionOnObject(true);
			ClearCollisionObject();
		}
	}

protected:
	virtual bool ShouldIgnoreCollision(RE::TESObjectREFR* a_colRef) = 0;

private:
	void SetCollisionObject(util::not_null<RE::Actor*> a_actor)
	{
		const auto controller = a_actor->GetCharController();
		_collisionObj = controller->bumpedCharCollisionObject;
	}

	void ClearCollisionObject() { _collisionObj.reset(); }

	void SetCollisionOnObject(bool a_set)
	{
		auto& filter = _collisionObj->collidable.broadPhaseHandle.collisionFilterInfo;
		if (!a_set) {
			filter |= COLLISION_FLAG;
		} else {
			filter &= ~COLLISION_FLAG;
		}
	}

	RE::hkRefPtr<RE::hkpWorldObject> _collisionObj;
};

class DialogueCollider :
	public ICollider
{
	bool ShouldIgnoreCollision(RE::TESObjectREFR* a_colRef) override;
};

class AllyCollider :
	public ICollider
{
protected:
	bool ShouldIgnoreCollision(RE::TESObjectREFR* a_colRef) override
	{
		if (!a_colRef || a_colRef->IsNot(RE::FormType::ActorCharacter)) {
			return false;
		}

		const auto colActor = static_cast<RE::Actor*>(a_colRef);
		return colActor->IsPlayerTeammate() && !colActor->IsAMount();
	}
};

class SummonCollider :
	public ICollider
{
protected:
	bool ShouldIgnoreCollision(RE::TESObjectREFR* a_colRef) override
	{
		if (!a_colRef || a_colRef->IsNot(RE::FormType::ActorCharacter)) {
			return false;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto colActor = static_cast<RE::Actor*>(a_colRef);
		if (!player || colActor->IsAMount()) {
			return false;
		}

		const auto hCommander = colActor->GetCommandingActor();
		const auto hPlayer = player->CreateRefHandle();
		return hCommander && hCommander == hPlayer;
	}
};

class AllySummonCollider :
	public ICollider
{
protected:
	bool ShouldIgnoreCollision(RE::TESObjectREFR* a_colRef) override
	{
		if (!a_colRef || a_colRef->IsNot(RE::FormType::ActorCharacter)) {
			return false;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto colActor = static_cast<RE::Actor*>(a_colRef);
		if (!player | colActor->IsAMount()) {
			return false;
		}

		const auto commander = colActor->GetCommandingActor().get();
		return commander && commander->IsPlayerTeammate();
	}
};

class CollisionHandler
{
public:
	[[nodiscard]] static auto GetSingleton()
		-> util::not_null<CollisionHandler*>
	{
		static CollisionHandler singleton;
		return &singleton;
	}

	void Install()
	{
		if (!Init()) {
			logger::error("no collision handlers were enabled");
			return;
		}

		REL::Relocation<std::uintptr_t> target{ REL::ID(37350), 0xFB };
		REL::make_pattern<"E8 ?? ?? ?? ?? 48 8B 06 48 8B CE FF 90 ?? ?? ?? ?? 8B 86 ?? ?? ?? ?? C1 E8 02 33 DB A8 01 0F 85 ?? ?? ?? ?? 48 89 5D 40">()
			.match_or_fail(target.address());

		auto& trampoline = SKSE::GetTrampoline();
		_applyMovementDelta = trampoline.write_call<5>(target.address(), Hook_ApplyMovementDelta);

		logger::debug("Installed hooks for collision handler"sv);
	}

private:
	CollisionHandler() = default;
	CollisionHandler(const volatile CollisionHandler&) = delete;
	CollisionHandler& operator=(const volatile CollisionHandler&) = delete;

	static void Hook_ApplyMovementDelta(util::not_null<RE::Actor*> a_actor, float a_delta)
	{
		const auto proxy = CollisionHandler::GetSingleton();
		if (!proxy->CanProcess(a_actor, a_delta)) {
			proxy->_applyMovementDelta(a_actor, a_delta);
		}
	}

	bool Init()
	{
		if (*Settings::disableAllyCollision) {
			_colliders.push_back(std::make_unique<AllyCollider>());
		}

		if (*Settings::disableAllySummonCollision) {
			_colliders.push_back(std::make_unique<AllySummonCollider>());
		}

		if (*Settings::disableDialogueCollision) {
			_colliders.push_back(std::make_unique<DialogueCollider>());
		}

		if (*Settings::disableSummonCollision) {
			_colliders.push_back(std::make_unique<SummonCollider>());
		}

		return !_colliders.empty();
	}

	bool CanProcess(util::not_null<RE::Actor*> a_actor, float a_delta)
	{
		if (!a_actor->IsPlayerRef()) {
			return false;
		}

		const auto controller = a_actor->GetCharController();
		if (!controller) {
			return false;
		}

		const auto collisionObj = controller->bumpedCharCollisionObject.get();
		if (!collisionObj) {
			return false;
		}

		const auto& filter = collisionObj->collidable.broadPhaseHandle.collisionFilterInfo;
		if (filter & ICollider::COLLISION_FLAG) {
			return false;
		}

		const auto colRef = RE::TESHavokUtilities::FindCollidableRef(collisionObj->collidable);
		PreCollide(a_actor, colRef);

		_applyMovementDelta(a_actor, a_delta);

		PostCollide();

		return true;
	}

	void PreCollide(util::not_null<RE::Actor*> a_actor, RE::TESObjectREFR* a_colRef)
	{
		for (auto& collider : _colliders) {
			collider->PreCollide(a_actor, a_colRef);
		}
	}

	void PostCollide()
	{
		for (auto& collider : _colliders) {
			collider->PostCollide();
		}
	}

	REL::Relocation<decltype(Hook_ApplyMovementDelta)> _applyMovementDelta;
	std::vector<std::unique_ptr<ICollider>> _colliders;
};
