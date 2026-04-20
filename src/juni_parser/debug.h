#ifndef PDBG_ENABLE
	#define PDBG_ENABLE BUILD_DEBUG
#endif

#ifndef PDBG_INDENTWIDTH
	#define PDBG_INDENTWIDTH 3
#endif

#if PDBG_ENABLE

	_Thread_local OutStream PDBG_os;
	_Thread_local uint PDBG_depth = 0;

	#define PDBG_INIT(os) PDBG_os = os
	#define PDBG_PUSH PDBG_depth++
	#define PDBG_POP PDBG_depth--

	#define PDBG(...) PRINT(PDBG_os, STRING_WHITESPACE(PDBG_depth * PDBG_INDENTWIDTH),__VA_ARGS__,"\n")

#else

	#define PDBG_PUSH
	#define PDBG_POP
	#define PDBG_INIT(...)
	#define PDBG(...)

#endif
