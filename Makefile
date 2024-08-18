.PHONY: all run clean test fmt

CLANG_FORMAT=$(shell echo "$${CLANG_FORMAT:-clang-format}")

all:
	pls b

run: all
	./.debug/test_server

clean:
	rm -rf build

test:
	pytest .

fmt:
	${CLANG_FORMAT} -i src/*.cc src/*.h
