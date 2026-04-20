typedef struct {
	usize offset;
	u32 row;
	u32 col;
} SourcePos;

#define SourcePos_DEFAULT_COL 1
#define SourcePos_NULL ((SourcePos){.offset=0,.row=1,.col=SourcePos_DEFAULT_COL})

SourcePos SourcePos_advance(SourcePos pos, const ubyte *it, const ubyte *end) {
	pos.offset += (usize)(end - it);

	for (; it < end; it++) {
		const ubyte c = *it;
		if (c == 10) { // line feed
			pos.col = SourcePos_DEFAULT_COL;
			pos.row++;
		} else if (
			c == 9 ||               // horizontal tab
			(c >= 32 && c < 127) || // printable ascii
			c >= 192                // lead utf8 byte
		) {
			pos.col++;
		}
	}

	return pos;
}
