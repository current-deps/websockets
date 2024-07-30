CFLAGS = -I. \
		 -g \
		 -lssl \
		 -lcrypto \
		 -lpthread \
		 -lm \
		 -lstdc++ \
		 -std=c++17

all:
	mkdir -p build;
	# temporary. TODO: add cmake
	gcc -o build/ws tests/test.cpp third_party/wsServer/libws.a $(CFLAGS)

run: all
	./build/ws


clean:
	rm -f build/ws


test:
	pytest .
