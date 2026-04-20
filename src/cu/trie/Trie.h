typedef enum : u8 {
	// determines trie node type, if 1 -> branch else segment
	TrieFlag_BIT_BRANCH,
	TrieFlag_BIT_OCCUPIED,
	TrieFlag_BIT_CONST
} TrieFlag;

#define TrieFlag__BITS 3
#define TrieFlag__MASK (0b111ULL)

// whether to do micoroptimizations for conserving memory, e.g. resizing
// segment allocations when it is unnecessary

#ifndef Trie_CONSERVATIVE
	#define Trie_CONSERVATIVE true
#endif

typedef struct {
	Ptr value;
} Trie;

#define Trie_NULL ((Trie){(Ptr)0})

bool Trie_isnull(Trie this) {
	return this.value == (Ptr)0;
}

TrieFlag Trie_flags(Trie this) {
	return (TrieFlag)((usize)this.value & TrieFlag__MASK);
}

bool Trie_isbranch(Trie this) {
	return (usize)this.value & FLAG(TrieFlag, BRANCH);
}

bool Trie_isoccupied(Trie this) {
	return (usize)this.value & FLAG(TrieFlag, OCCUPIED);
}

bool Trie_isconst(Trie this) {
	return (usize)this.value & FLAG(TrieFlag, CONST);
}

Ptr Trie_data(Trie this) {
	return (Ptr)((usize)this.value & (~TrieFlag__MASK));
}

Trie Trie_upcast(Ptr data, TrieFlag flags) {
	return (Trie){(Ptr)((usize)data | flags)};
}

Trie Trie_const(Trie this) {
	return (Trie){(Ptr)((usize)this.value | FLAG(TrieFlag, CONST))};
}

Trie ZZTrie_orflags(Trie this, TrieFlag flags) {
	return (Trie){(Ptr)((usize)this.value | flags)};
}

bool ZZTrie_equal(Trie this, Trie other) {
	return (((usize)this.value ^ (usize)other.value) & FLAG_NOT(TrieFlag, CONST)) == 0;
}

typedef u16 TrieSize;

typedef struct {
	Ptr value;
	u64 map[4];
	Trie next[];
} TrieBranch;

static_assert(alignof(TrieBranch) > TrieFlag__MASK);

usize ZZTrieBranch_allocsize(uint size) {
	return offsetof(TrieBranch, next) + (size * sizeof(Trie));
}

typedef struct {
	Ptr value;
	Trie next;
	TrieSize size;
	u8 bytes[];
} TrieSegment;

static_assert(alignof(TrieSegment) > TrieFlag__MASK);

usize ZZTrieSegment_allocsize(TrieSize size) {
	return offsetof(TrieSegment, bytes) + size;
}

Trie Trie_createstub(Ptr value, Allocator alc) {
	TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(0));
	data->next = Trie_NULL;
	data->value = value;
	data->size = 0;

	return Trie_upcast(data, FLAG(TrieFlag, OCCUPIED));
}

Trie Trie_create(
	const u8 *segment, TrieSize segment_size,
	Ptr value, Allocator alc
) {
	if (segment_size == 0)
		return Trie_createstub(value, alc);

	TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(segment_size));
	data->next = Trie_createstub(value, alc);
	data->value = nullptr;
	data->size = segment_size;
	memcpy(data->bytes, segment, segment_size);
	return Trie_upcast(data, FLAG(TrieFlag));
}

Trie ZZTrie_set(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Ptr value,
	Allocator alc
);

Trie ZZTrie_unset(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Allocator alc
);

void Trie_destroy(
	Trie this,
	Allocator alc
);

void Trie_print(
	Trie this,
	TrieSize depth,
	OutStream os
);

#include "Branch.h"
#include "Segment.h"

Trie ZZTrie_set(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Ptr value,
	Allocator alc
) {
	if (Trie_isnull(this)) UNREACHABLE;

	if (Trie_isbranch(this)) {
		return TrieBranch_set(this, segment, segment_size, value, alc);
	} else {
		return TrieSegment_set(this, segment, segment_size, value, alc);
	}
}

Trie Trie_set(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Ptr value,
	Allocator alc
) {
	if (Trie_isnull(this)) {
		return Trie_create(segment, segment_size, value, alc);
	}

	return ZZTrie_set(this, segment, segment_size, value, alc);
}

Trie ZZTrie_unset(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Allocator alc
) {
	if (Trie_isnull(this)) UNREACHABLE;

	if (Trie_isbranch(this)) {
		return TrieBranch_unset(this, segment, segment_size, alc);
	} else {
		return TrieSegment_unset(this, segment, segment_size, alc);
	}
}

Trie Trie_unset(
	Trie this,
	const u8 *segment,
	TrieSize segment_size,
	Allocator alc
) {

	if (Trie_isnull(this)) {
		return this;
	}

	return ZZTrie_unset(this, segment, segment_size, alc);
}

void Trie_destroy(
	Trie this,
	Allocator alc
) {
	if (Trie_isnull(this))
		return;

	if (Trie_isbranch(this)) {
		return TrieBranch_destroy(this, alc);
	} else {
		return TrieSegment_destroy(this, alc);
	}
}

void Trie_print(
	Trie this,
	TrieSize depth,
	OutStream os
) {
	if (Trie_isnull(this))
		return;

	if (Trie_isbranch(this)) {
		return TrieBranch_print(this, depth, os);
	} else {
		return TrieSegment_print(this, depth, os);
	}
}
