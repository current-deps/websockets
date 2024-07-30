all:
	mkdir -p build; cd build && cmake ../ && cmake --build .

run: all
	./build/test_server


clean:
	rm -f build

test:
	pytest .
