#define UNIQUEPTR(name) struct {char name##_UNIQUEPTR__;} *name
#define UNIQUENUM(T, name) enum : T {name##_UNIQUENUM__} name
#define STRUCTDECL(name) struct name name

#define LITERAL(T,...) ((T){__VA_ARGS__})

#ifdef __GNUC__
	#define GCC_ERROR_MAX_DEPTH_REACHED _Pragma("GCC error \"max depth reached\"")

	#define GCC_DIAG_PUSH _Pragma("GCC diagnostic push")
	#define GCC_DIAG_POP _Pragma("GCC diagnostic pop")

	#define GCC_DIAG_IGNORE_WTYPELIMITS "GCC diagnostic ignored \"-Wtype-limits\""


	#define GCC_PRAGMA_3(P, N, ...) _Pragma(P##N) __VA_OPT__(GCC_ERROR_MAX_DEPTH_REACHED)
	#define GCC_PRAGMA_2(P, N, ...) _Pragma(P##N) __VA_OPT__(GCC_PRAGMA_3(P, __VA_ARGS__))
	#define GCC_PRAGMA_1(P, N, ...) _Pragma(P##N) __VA_OPT__(GCC_PRAGMA_2(P, __VA_ARGS__))

	#define GCC_DIAG_IGNORE(...) __VA_OPT__(GCC_PRAGMA_1(GCC_DIAG_IGNORE_, __VA_ARGS__))

#else

	#define GCC_DIAG_PUSH
	#define GCC_DIAG_POP
	#define GCC_IGNORE(...)

#endif
