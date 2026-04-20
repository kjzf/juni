Trie TrieSegment_set(
	Trie vthis,
	const u8 *segment, TrieSize segment_size,
	Ptr value, Allocator alc
) {
	const TrieFlag isoccupied = Trie_flags(vthis) & FLAG(TrieFlag, OCCUPIED);
	const TrieFlag isconst = Trie_flags(vthis) & FLAG(TrieFlag, CONST);
	TrieSegment *const this = Trie_data(vthis);

	if (segment_size == 0) {
		#if Trie_CONSERVATIVE
			if (value == this->value)
				return Trie_upcast(this, isoccupied | isconst);
		#endif

		TrieSegment *data;

		if (isconst) {
			const TrieSize this_size = this->size;
			data = Allocator_new(alc, ZZTrieSegment_allocsize(this_size));
			data->next = Trie_const(this->next);
			data->size = this_size;
			memcpy(data->bytes, this->bytes, this_size);
		} else {
			data = this;
		}

		data->value = value;
		return Trie_upcast(data, FLAG(TrieFlag, OCCUPIED));
	}

	const TrieSize this_size = this->size;

	if (Trie_isnull(this->next)) {
		// validate this is a stub

		if (this->size == 0) UNREACHABLE;
		if (!isoccupied) UNREACHABLE;

		TrieSegment *data;
		if (isconst) {
			data = Allocator_new(alc, ZZTrieSegment_allocsize(segment_size));
			data->value = this->value;
		} else {
			data = Allocator_resize(alc, this, ZZTrieSegment_allocsize(segment_size));
		}

		data->size = segment_size;
		memcpy(data->bytes, segment, segment_size);
		data->next = Trie_createstub(value, alc);

		return Trie_upcast(data, FLAG(TrieFlag, OCCUPIED));
	}

	TrieSize idx = 0;
	if (segment_size < this_size) {
		for (; idx < segment_size; idx++) {
			if (this->bytes[idx] != segment[idx]) goto found_split;
		}

		// entirety of string fits into this segment, need to split
		// in the middle and create two segments - no branch

		TrieSegment *left;
		Trie right_next;

		if (isconst) {
			right_next = Trie_const(this->next);
			left = Allocator_new(alc, ZZTrieSegment_allocsize(idx));
			left->value = this->value;
			memcpy(left->bytes, this->bytes, idx);
		} else {
			right_next = this->next;
			#if Trie_CONSERVATIVE
				left = Allocator_resize(alc, this, ZZTrieSegment_allocsize(idx));
			#else
				left = this;
			#endif
		}

		left->size = idx;

		const TrieSize right_size = this_size - idx;
		TrieSegment *right = Allocator_new(alc, ZZTrieSegment_allocsize(right_size));
		right->size = right_size;
		right->value = value;
		right->next = right_next;
		memcpy(right->bytes, &this->bytes[idx], right_size);

		left->next = Trie_upcast(right, FLAG(TrieFlag, OCCUPIED));

		return Trie_upcast(left, isoccupied);

	} else {
		for (; idx < this_size; idx++) {
			if (this->bytes[idx] != segment[idx]) goto found_split;
		}

		// segment fits inside of new string, modify next node

		if (isconst) {

			#if Trie_CONSERVATIVE
				// if conservative mode is enabled, first modify next node, then
				// allocate the copy of this node if the modified node changed

				Trie next = ZZTrie_set(Trie_const(this->next),
					segment + idx, segment_size - idx, value, alc
				);

				if (ZZTrie_equal(next, this->next))
					return Trie_upcast(this, FLAG(TrieFlag, CONST) | isoccupied);

				TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(this_size));

				data->next = next;
			#else
				// else first allocate this node and then modify next

				TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(this_size));

				data->next = ZZTrie_set(Trie_const(this->next),
					segment + idx, segment_size - idx, value, alc
				);
			#endif

			data->size = this_size;
			data->value = this->value;
			memcpy(data->bytes, this->bytes, this_size);
			return Trie_upcast(data, isoccupied);
		} else {
			this->next = ZZTrie_set(this->next,
				segment + idx, segment_size - idx, value, alc
			);
			return Trie_upcast(this, isoccupied);
		}
	}

	found_split:;

	const u8 a = this->bytes[idx];
	const u8 b = segment[idx];

	if (idx == 0) {
		// if the first character didn't match, there is no need to create
		// a left side segment, and this segment can be reused for the right side

		TrieBranch *data = Allocator_new(alc, ZZTrieBranch_allocsize(2));
		memset(data->map, 0, sizeof(data->map));
		data->value = this->value;
		data->map[a / 64] |= (u64)1 << (a % 64);
		data->map[b / 64] |= (u64)1 << (b % 64);

		const TrieSize rest_size = this_size - 1;
		TrieSegment *rest;

		if (isconst) {
			rest = Allocator_new(alc, ZZTrieSegment_allocsize(rest_size));
			rest->size = rest_size;
			rest->next = Trie_const(this->next);
			memcpy(rest->bytes, this->bytes + 1, rest_size);
		} else {
			this->size = rest_size;
			memmove(this->bytes, this->bytes + 1, rest_size);

			#if Trie_CONSERVATIVE
				rest = Allocator_resize(alc, this, ZZTrieSegment_allocsize(rest_size));
			#else
				rest = this;
			#endif
		}

		Trie stub = Trie_create(segment + 1, segment_size - 1, value, alc);

		if (a < b) {
			data->next[0] = Trie_upcast(rest, FLAG(TrieFlag));
			data->next[1] = stub;
		} else {
			data->next[0] = stub;
			data->next[1] = Trie_upcast(rest, FLAG(TrieFlag));
		}

		return Trie_upcast(data, FLAG(TrieFlag, BRANCH) | isoccupied);

	} else {
		const TrieSize idx1 = idx + 1;
		const TrieSize rest_size = this_size - idx1;

		Trie rest = ZZTrie_orflags(this->next, isconst);
		if (rest_size != 0) {
			TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(rest_size));
			data->size = rest_size;
			data->next = rest;
			memcpy(data->bytes, this->bytes + idx1, rest_size);
			rest = Trie_upcast(data, FLAG(TrieFlag));
		}

		TrieSegment *left;
		if (isconst) {
			left = Allocator_new(alc, ZZTrieSegment_allocsize(idx));
			left->size = idx;
			left->value = this->value;
			memcpy(left->bytes, this->bytes, idx);
		} else {
			this->size = idx;

			#if Trie_CONSERVATIVE
				left = Allocator_resize(alc, this, ZZTrieSegment_allocsize(idx));
			#else
				left = this;
			#endif
		}

		TrieBranch *branch = Allocator_new(alc, ZZTrieBranch_allocsize(2));
		memset(branch->map, 0, sizeof(branch->map));
		branch->map[a / 64] |= (u64)1 << (a % 64);
		branch->map[b / 64] |= (u64)1 << (b % 64);

		Trie stub = Trie_create(segment + idx1, segment_size - idx1, value, alc);

		if (a < b) {
			branch->next[0] = rest;
			branch->next[1] = stub;
		} else {
			branch->next[0] = stub;
			branch->next[1] = rest;
		}

		left->next = Trie_upcast(branch, FLAG(TrieFlag, BRANCH));

		return Trie_upcast(left, isoccupied);
	}
}

Trie TrieSegment_unset(
	Trie vthis,
	const u8 *segment, TrieSize segment_size,
	Allocator alc
) {
	const TrieFlag isoccupied = Trie_flags(vthis) & FLAG(TrieFlag, OCCUPIED);
	const TrieFlag isconst = Trie_flags(vthis) & FLAG(TrieFlag, CONST);
	TrieSegment *const this = Trie_data(vthis);

	if (segment_size == 0) {
		if (Trie_isnull(this->next)) {
			if (!isconst) {
				Allocator_delete(alc, this);
			}

			return Trie_NULL;
		} else {
			return Trie_upcast(this, isconst);
		}

	}

	const TrieSize this_size = this->size;

	if (
		Trie_isnull(this->next) ||
		(segment_size < this_size) ||
		(memcmp(segment, this->bytes, this_size) != 0)
	) {
		return Trie_upcast(this, isoccupied | isconst);
	}

	if (isconst) {
		Trie next = ZZTrie_unset(Trie_const(this->next),
			segment + this_size, segment_size - this_size, alc
		);

		if (ZZTrie_equal(next, this->next))
			return Trie_upcast(this, FLAG(TrieFlag, CONST) | isoccupied);

		if (Trie_isnull(next)) {
			if (isoccupied)
				return Trie_createstub(this->value, alc);
			else
				return Trie_NULL;

		} else {
			TrieSegment *data = Allocator_new(alc, ZZTrieSegment_allocsize(this_size));
			data->value = this->value;
			data->size = this_size;
			data->next = next;
			memcpy(data->bytes, this->bytes, this_size);
			return Trie_upcast(data, isoccupied);
		}
	} else {
		Trie next = ZZTrie_unset(this->next,
			segment + this_size, segment_size - this_size, alc
		);

		if (ZZTrie_equal(next, this->next))
			return Trie_upcast(this, isoccupied);

		if (Trie_isnull(next)) {
			if (isoccupied) {
				this->size = 0;
				#if Trie_CONSERVATIVE
					TrieSegment *data = Allocator_resize(alc, this, ZZTrieSegment_allocsize(0));
					return Trie_upcast(data, FLAG(TrieFlag));
				#else
					return Trie_upcast(this, FLAG(TrieFlag));
				#endif

			} else {
				Allocator_delete(alc, this);
				return Trie_NULL;
			}
		} else {

			this->next = next;
			return Trie_upcast(this, isoccupied);
		}

	}
}

void TrieSegment_destroy(Trie vthis, Allocator alc) {
	if (Trie_isconst(vthis)) return;
	TrieSegment *this = Trie_data(vthis);

	if (!Trie_isnull(this->next)) {
		Trie_destroy(this->next, alc);
	}

	Allocator_delete(alc, this);
}

void TrieSegment_print(Trie vthis, TrieSize depth, OutStream os) {
	const bool isoccupied = Trie_isoccupied(vthis);
	TrieSegment *this = Trie_data(vthis);
	PRINT(os, "> ");

	if (isoccupied) PRINT(os, "(",(Ptr)this->value,") ");

	if (this->size) {
		const String s = {
			.data = this->bytes,
			.size = this->size
		};

		PRINT(os, s," ");
	}

	const Trie next = this->next;
	if (Trie_isnull(this->next)) return;

	Trie_print(next, depth, os);
}
