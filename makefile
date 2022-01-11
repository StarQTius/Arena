check:
	mkdir -p build
	cd build && cmake .. && cmake --build .
	build/test
