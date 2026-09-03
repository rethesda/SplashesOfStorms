#pragma once

#include "RE/Skyrim.h"
#include "REX/REX.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;

namespace stl
{
	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
	    auto& trampoline = REL::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}
}

#include "Version.h"

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#else
#	define OFFSET(se, ae) se
#endif
