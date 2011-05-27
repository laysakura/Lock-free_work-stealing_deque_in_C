# CC is set by $ export CC=hoge

CC = gcc
CFLAGS = -Wall -O2 -g -DDEBUG
LDFLAGS = -lpthread -lrt
INCLUDES =

SRCS = $(wildcard *.c)
OBJS = $(subst .c,.o,$(SRCS))
DEPENDS = $(subst .c,.d,$(SRCS))
TARGETS = test_gsoc_taskqueue test_gsoc_task_circular_array
TEST_TARGETS = TEST_gsoc_task_circular_array \
               TEST_gsoc_taskqueue


TESTVAL_EXTENDS = 50


.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGETS) $(DEPENDS)

.PHONY: test
test: $(TEST_TARGETS)

.PHONY: TEST_gsoc_taskqueue
TEST_gsoc_task_circular_array: test_gsoc_task_circular_array
	@echo ""
	@echo "=== TEST_gsoc_task_circular_array ==="
	@./test_gsoc_task_circular_array ; \
	if [ $$? -ne 0 ]; then \
	    echo "Faild. This might be because you use 32bit CPU (and the pointer address cannot be so long). [NG]" ; \
	else \
	    echo "Success [OK]" ; \
	fi

.PHONY: TEST_gsoc_taskqueue
TEST_gsoc_taskqueue: test_gsoc_taskqueue
	@echo ""
	@echo "=== TEST_gsoc_taskqueue ==="
	@echo "You ALWAYS need to Recompile test_gsoc_taskqueue.c to make -D TESTVAL_* enable."
# Create logfile first since $(shell)  is extracted before all other commands
	$(shell ./test_gsoc_taskqueue > test_gsoc_taskqueue.log 2> debug.log)

# Count number of unique digits
	@LINES=$(shell wc -l < test_gsoc_taskqueue.log) ;UNIQS=$(shell sort -g test_gsoc_taskqueue.log |awk '{print $$1}' |uniq |wc -l) ;EXPECT=$(shell expr 131072 \* $(TESTVAL_EXTENDS)) ; \
	if [ $$UNIQS -lt $$EXPECT ]; then \
		if [ $$LINES -ne $$UNIQS ]; then \
			echo "Unique numbers ($$UNIQS) are fewer than expected ($$EXPECT) due to duplicated values [NG]" ; \
		else \
			echo "Unique numbers are fewer than expected due to skipping cells of taskqueue [NG]" ; \
		fi ; \
	elif [ $$UNIQS -eq $$EXPECT ]; then \
		echo "Unique numbers are the same as expected [OK]" ; \
	else \
		echo "I DON'T KNOW WHAT IS HAPPNING TO UNIQUE NUMBER [NG]" ;\
	fi

# Each number should be in a range
	@MIN=$(shell sort -g test_gsoc_taskqueue.log |head -n 1 |awk '{print $$1}') ; expr 0 \<= $$MIN > /dev/null ; \
	if [ $$? -ne 0 ]; then \
		echo "A number out of range [NG]" ; \
	else \
		echo "Minimum number is $$MIN [OK]" ; \
	fi
	@MAX=$(shell sort -g test_gsoc_taskqueue.log |tail -n 1 |awk '{print $$1}') ;UPPER_LIMIT=$(shell expr 131072 \* $(TESTVAL_EXTENDS)) ; \
	expr \( $$MAX + 1 \) = $$UPPER_LIMIT > /dev/null ; \
	if [ $$? -ne 0 ]; then \
		echo "Maximum number $$MAX is different from expected value upper limit $$UPPER_LIMIT - 1 [NG]" ; \
	else \
		echo "Maximum number is $$MAX while upper limit is $$UPPER_LIMIT [OK]" ; \
	fi

test_gsoc_taskqueue: gsoc_taskqueue.o test_gsoc_taskqueue.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS) -D TESTVAL_EXTENDS=$(TESTVAL_EXTENDS)

test_gsoc_task_circular_array: gsoc_task_circular_array.h test_gsoc_task_circular_array.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LDFLAGS)

%.d: %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@
-include $(DEPENDS)

.PHONY: check-syntax
check-syntax:
	gcc $(CFLAGS) -fsyntax-only $(CHK_SOURCES)
