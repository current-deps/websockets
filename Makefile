.PHONY: all run clean test fmt

CLANG_FORMAT=$(shell echo "$${CLANG_FORMAT:-clang-format}")

all:
	mkdir -p build; cd build && cmake -DENABLE_TEST=ON .. && cmake --build .

run: all
	./build/test_server

clean:
	rm -rf build

test:
	pytest .

fmt:
	${CLANG_FORMAT} -i src/*.cc src/*.h
