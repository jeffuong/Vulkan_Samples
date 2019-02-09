CC = clang++

CFLAGS = -Werror -std=c++11 

LDFLAGS = -lglfw -lvulkan

ODIR = ../../build

ifdef config
DEF_CONFIG = -D_$(config)
endif

hello_triangle: hello_triangle.cpp
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $(ODIR)/hello_triangle hello_triangle.cpp $(LDFLAGS) $(DEF_CONFIG)

.PHONY: test clean

test: 
	../../build/hello_triangle

clean:
	rm -f ../../build/hello_triangle