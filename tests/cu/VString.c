int ZZentry(testing_TestContext *ctx) {
	String hello = STRING("こんにちは、世界よ 🌍\n");
	FPRINT(stdout, VString_upcast(hello.data, hello.size, FLAG(VStringFlag, KEEPUTF)) ,"\n");
	return 0;
}
