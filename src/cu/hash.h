typedef u64 uhash;

#define FNV1a_BASE 0xcbf29ce484222325ULL
#define FNV1a_PRIME 0x100000001b3ULL

uhash FNV1a(const ubyte *bytes, usize size, uhash base) {
	for (usize i = 0; i < size; i++) {
		base = (base ^ bytes[i]) * FNV1a_PRIME;
	}
	return base;
}

uhash HASH_u8(u8 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash HASH_u16(u16 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash HASH_u32(u32 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash HASH_u64(u64 value, uhash base) {
	return (base ^ value) * FNV1a_PRIME;
}

uhash HASH_Ptr(Ptr value, uhash base) {
	return (base ^ (usize)value) * FNV1a_PRIME;
}

#define HASH_FN FNV1a
#define HASH_BASE FNV1a_BASE
#define HASH(value, base) _Generic((value), \
	u8 : HASH_u8, \
	u16 : HASH_u16, \
	u32 : HASH_u32, \
	u64 : HASH_u64, \
	Ptr : HASH_Ptr, \
	String : String_hash \
)(value, base)

#define HASH_COMBINE HASH_u64
