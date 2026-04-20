typedef enum : u8 {
	VStringFlag_BIT_NOUTF,
	VStringFlag_BIT_KEEPUTF,
	VStringFlag_BIT_KEEPHT,
	VStringFlag_BIT_KEEPCR,
	VStringFlag_BIT_KEEPLF,
	VStringFlag__BITS
} VStringFlag;

#define VStringFlag__MASK ((1ULL << VStringFlag__BITS) - 1)

typedef struct {
	const ubyte *data;
	usize meta;
} VString;

usize VString_size(VString this) {
	return this.meta >> VStringFlag__BITS;
}

VStringFlag VString_flags(VString this) {
	return (VStringFlag)(this.meta & VStringFlag__MASK);
}

VString VString_upcast(const ubyte *data, usize size, VStringFlag flags) {
	return (VString) {
		.data = data,
		.meta = (size << VStringFlag__BITS) | flags
	};
}

void VString_print(VString this, OutStream os) {
	VStringFlag flags = VString_flags(this);

	ubyte smallbuf[16];
	ubyte buffer[256];
	const ubyte *bend = buffer + sizeof(buffer);
	ubyte *bp = buffer;

	const ubyte *it = this.data;
	const ubyte *end = it + VString_size(this);


	for (; it < end; it++) {
		ubyte c = *it;

		String repr = {.data = it, .size = 1};

		switch (c) {
			case 9:
				if (!(flags & FLAG(VStringFlag, KEEPHT)))
					repr = STRING("&ht;");
				goto putc;

			case 10:
				if (!(flags & FLAG(VStringFlag, KEEPLF)))
					repr = STRING("&lf;");
				goto putc;

			case 13:
				if (!(flags & FLAG(VStringFlag, KEEPCR)))
					repr = STRING("&cr;");
				goto putc;

			default:
		}

		if (flags & FLAG(VStringFlag, NOUTF))
			goto skip_utf;

		#define XISUTFCONT(c) ((c & 0b1100'0000u) == 0b1000'0000u)

		if ((c & 0b1111'1000u) == 0b1111'0000u) { // 4 byte utf8 sequence
			if (
				((end - it) >= 4) &&
				XISUTFCONT(it[1]) &&
				XISUTFCONT(it[2]) &&
				XISUTFCONT(it[3])
			) {
				it += 3;
				if (flags & FLAG(VStringFlag, KEEPUTF)) {
					repr.size = 4;
					goto putc;
				} else {
					repr = STRING("&??;");
					goto putc;
				}
			} else {
				goto malformed_utf;
			}
		}

		if ((c & 0b1111'0000u) == 0b1110'0000u) { // 3
			if (
				((end - it) >= 3) &&
				XISUTFCONT(it[1]) &&
				XISUTFCONT(it[2])
			) {
				it += 2;
				if (flags & FLAG(VStringFlag, KEEPUTF)) {
					repr.size = 3;
					goto putc;
				} else {
					repr = STRING("&??;");
					goto putc;
				}
			} else {
				goto malformed_utf;
			}
		}

		if ((c & 0b1110'0000u) == 0b1100'0000u) { // 2
			if (
				((end - it) >= 2) &&
				XISUTFCONT(it[1])
			) {
				it += 1;
				if (flags & FLAG(VStringFlag, KEEPUTF)) {
					repr.size = 2;
					goto putc;
				} else {
					repr = STRING("&??;");
					goto putc;
				}
			} else {
				goto malformed_utf;
			}
		}

		// utf8 continuation byte
		if (XISUTFCONT(c)) malformed_utf: {
			repr.size = (usize)snprintf((char*)smallbuf, sizeof(smallbuf), "&?%02X;", c);
			repr.data = smallbuf;
			goto putc;
		}

		#undef XISUTFCONT

		skip_utf:;

		if (c < 32 || c > 126) {
			repr.size = (usize)snprintf((char*)smallbuf, sizeof(smallbuf), "&%u;", c);
			repr.data = smallbuf;
			goto putc;
		}

		putc:;
		if (((usize)(bp - buffer) + repr.size) > sizeof(buffer)) {
			OutStream_write(os, buffer, (usize)(bp - buffer));
		}

		memcpy(bp, repr.data, repr.size);
		bp += repr.size;
	}

	if (bp != buffer) {
		OutStream_write(os, buffer, (usize)(bp - buffer));
	}

	#undef XENSURE
}
