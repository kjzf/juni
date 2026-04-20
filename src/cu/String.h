typedef struct {
	const ubyte *data;
	usize size;
} String;

#define String_NULL ((String){.data=nullptr,.size=0})

void String_print(String this, OutStream os) {
	OutStream_write(os, this.data, this.size);
}

uhash String_hash(String this, uhash base) {
	return HASH_FN(this.data, this.size, base);
}

typedef struct {
	const ubyte *begin;
	const ubyte *end;
} StringSpan;

#define StringSpan_NULL ((StringSpan){.begin=nullptr,.end=nullptr})

void StringSpan_print(StringSpan this, OutStream os) {
	OutStream_write(os, this.begin, (usize)(this.end - this.begin));
}

uhash StringSpan_hash(StringSpan this, uhash base) {
	return HASH_FN(this.begin, (usize)(this.end - this.begin), base);
}

#define USTR(str) (const ubyte*)(str), __builtin_strlen(str)
#define STRING(str) ((String){.data=(const ubyte*)(str), .size=__builtin_strlen(str)})

#ifndef SmallString_PTRTAG
	#define SmallString_PTRTAG PTRTAG
#endif

#ifndef SmallString_SAFE
	#define SmallString_SAFE BUILD_SAFE
#endif

#if SmallString_PTRTAG

	#define SmallString_MAX PTRTAG_MAX

	typedef struct {
		Ptr value;
	} SmallString;

	usize SmallString_size(SmallString this) {
		return ptrread(this.value);
	}

	const ubyte *SmallString_data(SmallString this) {
		return ptrstrip(this.value);
	}

	SmallString SmallString_upcast(const ubyte *data, usize size) {
		#if SmallString_SAFE
			if (size > SmallString_MAX) PANIC("SmallString_upcast: size overflow");
		#endif

		return (SmallString){ptrtag((Ptr)data, (utag)size)};
	}

	#define SmallString_NULL ((SmallString){nullptr})
#else

	#define SmallString_MAX SIZE_MAX

	typedef struct {
		const ubyte *data;
		usize size;
	} SmallString;

	usize SmallString_size(SmallString this) {
		return this.size;
	}

	const ubyte *SmallString_data(SmallString this) {
		return this.data;
	}

	SmallString SmallString_upcast(const ubyte *data, usize size) {
		return (SmallString){.data=data,.size=size};
	}

	#define SmallString_NULL ((SmallString){.data=nullptr,.size=0})

#endif

void SmallString_print(SmallString this, OutStream os) {
	OutStream_write(os, SmallString_data(this), SmallString_size(this));
}

uhash SmallString_hash(SmallString this, uhash base) {
	return HASH_FN(SmallString_data(this), SmallString_size(this), base);
}
