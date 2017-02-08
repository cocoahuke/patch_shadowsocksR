clear = ""

patching:build/patch_ss
	@echo ""
	@echo "start patching"
	@cd build&&./patch_ss

build/patch_ss:build/hack_clock_gettime.dylib
	clang -w src/patch_ss.c -o $@

build/hack_clock_gettime.dylib:$(clear)
	clang -dynamiclib src/hack_clock_gettime.c src/fishhook.c $^ \
	-install_name hack_clock_gettime.dylib \
	-o $@ \

$(clear):
	@rm -rf build
	@mkdir -p build
