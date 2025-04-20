#pragma once

namespace Hooks
{	
	class Hook
	{
	public:
		static void SetupHook()
		{
#if _DEBUG || defined(_FULL_LOGGING)
			REL_MESSAGE("Hooking...");
			REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Actor[0] };
			_UpdateInDialogue_Actor = ActorVtbl.write_vfunc(0x4c, UpdateInDialogue_Actor);
			REL::Relocation<std::uintptr_t> CharacterVtbl{ RE::VTABLE_Character[0] };
			_UpdateInDialogue_Character = CharacterVtbl.write_vfunc(0x4c, UpdateInDialogue_Character);
			REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };
			_UpdateInDialogue_PlayerCharacter = PlayerCharacterVtbl.write_vfunc(0x4c, UpdateInDialogue_PlayerCharacter);
			REL_MESSAGE("...success");
#endif
		}

	private:
		static bool SetCurrentScene_Actor(RE::Actor* a_this, RE::BGSScene* a_scene);                                                                         // 04C
		static inline REL::Relocation<decltype(SetCurrentScene_Actor)> _SetCurrentScene_Actor;
		static bool UpdateInDialogue_Actor(RE::Actor* a_this, RE::DialogueResponse* a_response, bool a_unused);                                                                         // 04C
		static inline REL::Relocation<decltype(UpdateInDialogue_Actor)> _UpdateInDialogue_Actor;
		static bool UpdateInDialogue_Character(RE::Character* a_this, RE::DialogueResponse* a_response, bool a_unused);                                                                         // 04C
		static inline REL::Relocation<decltype(UpdateInDialogue_Character)> _UpdateInDialogue_Character;
		static bool UpdateInDialogue_PlayerCharacter(RE::PlayerCharacter* a_this, RE::DialogueResponse* a_response, bool a_unused);                                                                         // 04C
		static inline REL::Relocation<decltype(UpdateInDialogue_PlayerCharacter)> _UpdateInDialogue_PlayerCharacter;
	};

	void Install();
}