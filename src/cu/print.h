#define PRINT_GENERATE_PRIMITIVE(N, T, Fmt) \
	void PRINT_##N(T value, OutStream os) { \
		char buf[32]; \
		auto sz = snprintf(buf, 32, Fmt, value); \
		OutStream_write(os, (ubyte*)buf, (usize)sz); \
	}

PRINT_GENERATE_PRIMITIVE(short, short, "%hi")
PRINT_GENERATE_PRIMITIVE(ushort, unsigned short, "%hu")
PRINT_GENERATE_PRIMITIVE(int, int, "%i")
PRINT_GENERATE_PRIMITIVE(uint, unsigned int, "%u")
PRINT_GENERATE_PRIMITIVE(long, long, "%li")
PRINT_GENERATE_PRIMITIVE(ulong, unsigned long, "%lu")
PRINT_GENERATE_PRIMITIVE(llong, long long, "%lli")
PRINT_GENERATE_PRIMITIVE(ullong, unsigned long long, "%llu")
PRINT_GENERATE_PRIMITIVE(ptr, void*, "%p")

void PRINT_byte(unsigned char value, OutStream os) {
	static const char hex_digits[] = "0123456789ABCDEF";
	char buf[2] = {
		hex_digits[(value >> 4)],
		hex_digits[value & 0b1111]
	};

	OutStream_write(os, (ubyte*)buf, 2);
}

void PRINT_bool(bool value, OutStream os) {
	if (value)
		OutStream_write(os, USTR("true"));
	else
		OutStream_write(os, USTR("false"));
}

void PRINT_char(char value, OutStream os) {
	OutStream_write(os, (ubyte*)&value, 1);
}

void PRINT_cstring(Str cstr, OutStream os) {
	if (!cstr) {
		OutStream_write(os, USTR("(nullstr)"));
	}

	OutStream_write(os, (const ubyte*)cstr, strlen(cstr));
}

#define PRINT_ITEM(S, A) _Generic((A), \
	char : PRINT_char, \
	unsigned char : PRINT_byte, \
	short : PRINT_short, \
	unsigned short : PRINT_ushort, \
	int : PRINT_int, \
	unsigned int : PRINT_uint, \
	long : PRINT_long, \
	unsigned long : PRINT_ulong, \
	long long : PRINT_llong, \
	unsigned long long : PRINT_ullong, \
	bool : PRINT_bool, \
	char* : PRINT_cstring, \
	Str : PRINT_cstring, \
	Ptr : PRINT_ptr, \
	String : String_print, \
	StringSpan : StringSpan_print, \
	SmallString : SmallString_print, \
	VString : VString_print, \
	Printable : Printable_print \
)((A), (S))

#define PRINT_X(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(MAX_PRINT_DEPTH_REACHED)
#define PRINT_9(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_X(S, __VA_ARGS__))
#define PRINT_8(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_9(S, __VA_ARGS__))
#define PRINT_7(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_8(S, __VA_ARGS__))
#define PRINT_6(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_7(S, __VA_ARGS__))
#define PRINT_5(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_6(S, __VA_ARGS__))
#define PRINT_4(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_5(S, __VA_ARGS__))
#define PRINT_3(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_4(S, __VA_ARGS__))
#define PRINT_2(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_3(S, __VA_ARGS__))
#define PRINT_1(S, A, ...) PRINT_ITEM(S, A); __VA_OPT__(PRINT_2(S, __VA_ARGS__))

#define PRINT(stream, ...) { \
	const OutStream PRINT__stream = (stream); \
	PRINT_1(PRINT__stream, __VA_ARGS__) \
}

#define FPRINT(file, ...) PRINT(FileOutStream_upcast(file), __VA_ARGS__)
