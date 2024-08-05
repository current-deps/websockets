.PHONY: all run clean test fmt

CLANG_FORMAT=$(shell echo "$${CLANG_FORMAT:-clang-format}")

all:
	cmake -DENABLE_TEST=ON -B build . && cmake --build build

run: all
	./build/test_server

clean:
	rm -rf build

test:
	pytest .

fmt:
	${CLANG_FORMAT} -i src/*.cc src/*.h
