typedef struct {
	Arc_MEMBER rc;
	Allocator alc;
	SmallString name;
	usize data_size;
	ubyte data[];
} Source;

usize ZZSource_allocsize(usize size) {
	return offsetof(Source, data) + size;
}

typedef struct { Arc value; } SourceRef;

const Source *Source_data(SourceRef this) {
	return (Ptr)Arc_state(this.value);
}

StringSpan Source_at(SourceRef rthis, usize offset) {
	auto this = Source_data(rthis);

	#if BUILD_SAFE
		if (offset > this->data_size) PANIC("SourceRef_at: overflow");
	#endif

	return (StringSpan) {
		.begin = this->data + offset,
		.end = this->data + this->data_size
	};
}

SourceRef Source_copy(SourceRef rthis) {
	return (SourceRef){Arc_copy(rthis.value)};
}

SourceRef Source_const(SourceRef rthis) {
	return (SourceRef){Arc_const(rthis.value)};
}

void Source_release(SourceRef rthis) {
	if (Arc_release(rthis.value)) {
		auto this = Source_data(rthis);

		ubyte *name = (Ptr)SmallString_data(this->name);

		if (name)
			Allocator_delete(this->alc, name);

		Allocator_delete(this->alc, (Ptr)this);
	}
}

SmallString Source_name(SourceRef rthis) {
	auto this = Source_data(rthis);
	return this->name;
}

SmallString ZZSource_makename(String name, Allocator alc) {
	if (name.size) {
		ubyte *namebuf = Allocator_new(alc, name.size);
		memcpy(namebuf, name.data, name.size);
		return SmallString_upcast(namebuf, name.size);
	} else {
		return SmallString_NULL;
	}
}


SourceRef Source_fromstring(String name, String data, Allocator alc) {
	Source *this = Allocator_new(alc, ZZSource_allocsize(data.size));
	this->alc = alc;

	this->name = ZZSource_makename(name, alc);
	this->data_size = data.size;
	memcpy(this->data, data.data, data.size);

	return (SourceRef){Arc_init(&this->rc)};
}

SourceRef Source_fromfile(String name, FILE *file, Allocator alc) {
	long tell_origin = ftell(file);
	if (tell_origin < 0) PANIC("ftell failed");

	if (fseek(file, 0, SEEK_END)) PANIC("fseek failed");

	long tell_size = ftell(file);
	if (tell_size < 0) PANIC("ftell failed");

	if (fseek(file, tell_origin, SEEK_SET)) PANIC("fseek failed");

	usize data_size = (usize)(tell_size - tell_origin);

	Source *this = Allocator_new(alc, ZZSource_allocsize((usize)data_size));

	if (fread(this->data, 1, data_size, file) != data_size) PANIC("read_size mismatch");
	if (fseek(file, tell_origin, SEEK_SET)) PANIC("fseek failed");

	this->alc = alc;
	this->data_size = data_size;
	this->name = ZZSource_makename(name, alc);

	return (SourceRef){Arc_init(&this->rc)};
}

SourceRef Source_openfile(Str path, Allocator alc) {
	FILE *file = fopen(path, "r");
	if (!file) PANICF("Failed to open file ",path);

	SourceRef ref = Source_fromfile(STRING(path), file, alc);

	fclose(file);
	return ref;
}
