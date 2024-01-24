#include "PrecompiledHeaders.h"

// #pragma warning(disable: 4702)
// #include <xbyak/xbyak.h>

namespace Hooks
{
	void Install()
	{
		Hook::SetupHook();
	}

	bool Hook::UpdateInDialogue_Actor(RE::Actor* a_this, RE::DialogueResponse* a_response, bool a_unused)
	{
		std::string actorName;
		if (a_this->GetActorBase() && a_this->GetActorBase()->GetFullNameLength())
		{
			actorName = std::string(a_this->GetActorBase()->GetFullName(), a_this->GetActorBase()->GetFullNameLength());
		}
		REL_MESSAGE("Actor::UpdateInDialogue intercepted: '{}'", actorName);
		auto result(_UpdateInDialogue_Actor(a_this, a_response, a_unused));
		REL_MESSAGE("Actor::UpdateInDialogue completed");
		return result;
	}

	bool Hook::UpdateInDialogue_Character(RE::Character* a_this, RE::DialogueResponse* a_response, bool a_unused)
	{
		std::string actorName;
		if (a_this->GetActorBase() && a_this->GetActorBase()->GetFullNameLength())
		{
			actorName = std::string(a_this->GetActorBase()->GetFullName(), a_this->GetActorBase()->GetFullNameLength());
		}
		REL_MESSAGE("Character::UpdateInDialogue intercepted {}", actorName);
		auto result(_UpdateInDialogue_Character(a_this, a_response, a_unused));
		REL_MESSAGE("Character::UpdateInDialogue completed");
		return result;
	}

	bool Hook::UpdateInDialogue_PlayerCharacter(RE::PlayerCharacter* a_this, RE::DialogueResponse* a_response, bool a_unused)
	{
		REL_MESSAGE("PlayerCharacter::UpdateInDialogue intercepted");
		auto result(_UpdateInDialogue_PlayerCharacter(a_this, a_response, a_unused));
		REL_MESSAGE("PlayerCharacter::UpdateInDialogue completed");
		return result;
	}
}