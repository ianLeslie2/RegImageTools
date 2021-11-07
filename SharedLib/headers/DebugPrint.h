#pragma once

#ifdef DEBUG
#define debug_printf(...) if(DebugPrint::enabled) printf(__VA_ARGS__);
#else
#define debug_printf(...)
#endif

class DebugPrint{
	public:
		static bool enabled;
};