SRC_DIRS=src
TEST_DIRS=tests

MAKE_DIRS = $(SRC_DIRS) $(TEST_DIRS)


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
