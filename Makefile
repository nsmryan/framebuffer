PROG_NAME=framebuffer

ifeq ($(OS),Windows_NT)
	LDFLAGS += -s -lopengl32 -lgdi32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
	else ifeq ($(UNAME_S),Linux)
		LDFLAGS += -s -lGLU -lGL -lX11
	endif
endif

VPATH+=src
CFLAGS += -Iinc
LDFLAGS+=-lm

SRC=framebuffer.c tigr.c easing.c randq.c

.PHONY: fast
fast: $(SRC)
	gcc $^ -DWIDTH=512 -DHEIGHT=512 -O3 -march=native -mtune=native -o ${PROG_NAME} $(CFLAGS) $(LDFLAGS)

.PHONY: debug
debug : $(SRC)
	gcc $^ -O0 -g -o ${PROG_NAME} $(CFLAGS) $(LDFLAGS)

.PHONY: dev
dev: $(SRC)
	tcc $^ -DWIDTH=256 -DHEIGHT=256 -O0 -o ${PROG_NAME} $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	-rm framebuffer

.PHONY: run
run:
	./framebuffer
