SRC_DIRS=src
TEST_DIRS=tests tests-bash-scripts

MAKE_DIRS = $(SRC_DIRS) $(TEST_DIRS)


all: clean multimake 

multimake:
	@for d in $(MAKE_DIRS);  \
	do          \
		make -C $$d || exit 1;    \
	done;       \

clean:
	@for d in $(MAKE_DIRS); \
	do          \
		make -C $$d clean;  \
	done;           \
