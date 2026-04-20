typedef enum : u8 {
	ParserCode_OK,
	ParserCode_ERROR
} ParserCode;

typedef struct {
	ParserCode code;
} ParserResult;

typedef struct {
	OutStream log;
} ParserContext;

ParserResult Parser_entry(ParserContext *ctx, Trie db, SourceRef src, SourcePos *ppos) {
	SourcePos pos = *ppos;
	StringSpan buf = Source_at(src, pos.offset);

	next_value:;

	const ubyte *it = buf.begin;
	const ubyte *value_end = nullptr;
	Ptr value = nullptr;
	Trie dbhead = db;

	while (true) {
		const usize buf_size = (usize)(buf.end - it);
		if (!buf_size) break;

		const Ptr vdata = Trie_data(dbhead);
		if (Trie_isoccupied(dbhead)) {
			value_end = it;
			value = *(Ptr*)vdata;
		}

		if (Trie_isbranch(dbhead)) {
			const TrieBranch *data = vdata;

			ubyte c = *(it++);

			u8 map_idx = c / 64;
			u64 map_bit = (u64)1 << (c % 64);

			if (data->map[map_idx] & map_bit) {
				uint i = ZZTrieBranch_index(data, map_idx, map_bit);
				dbhead = data->next[i];
				continue;
			}

			break;
		} else {
			const TrieSegment *data = vdata;

			Trie next = data->next;
			const usize size = data->size;

			if (
				Trie_isnull(next) ||
				(size > buf_size) ||
				memcmp(data->bytes, it, size)
			) break;

			it += size;
			dbhead = next;
			continue;
		}
	}

	if (!value_end) {
		return (ParserResult) {ParserCode_ERROR};
	}

	PDBG(pos.row,":",pos.col," ",value);

	it = value_end;
	pos = SourcePos_advance(pos, buf.begin, value_end);
	buf.begin = value_end;

	goto next_value;
}
