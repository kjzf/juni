int ZZentry(testing_TestContext *ctx) {
	PDBG_INIT(FileOutStream_upcast(stderr));

	Trie db = Trie_NULL;

	#define X(C, V) db = Trie_set(db, (ubyte[]){C}, 1, V, Malloc)
		X(9, (Ptr)0);
		X(10, (Ptr)0);
		X(13, (Ptr)0);
		X(32, (Ptr)0);
		X('(', (Ptr)1);
		X(')', (Ptr)2);
		X('{', (Ptr)3);
		X('}', (Ptr)4);
	#undef X

	db = Trie_set(db, USTR("int"), (Ptr)256, Malloc);
	db = Trie_set(db, USTR("main"), (Ptr)257, Malloc);
	db = Trie_set(db, USTR("return"), (Ptr)258, Malloc);

	SourceRef src = Source_openfile("tests/juni_parser/example.juni", Malloc);
	SourcePos pos = SourcePos_NULL;

	Parser_entry(nullptr, Trie_const(db), src, &pos);

	Trie_destroy(db, Malloc);
	Source_release(src);

	return 0;

}
