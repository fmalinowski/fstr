SRC_DIRS=src

MAKE_DIRS = $(SRC_DIRS)


all: multimake 

multimake:
	@for d in $(MAKE_DIRS);  \
	do          \
		make -C $$d;    \
	done;       \

clean:
	@for d in $(MAKE_DIRS); \
	do          \
		make -C $$d clean;  \
	done;           \
