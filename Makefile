ifndef target
	target := msh
endif
objs   := main.o builtins.o
CC     := gcc
CFLAGS := -O3 -std=c99 -pedantic -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wno-unused-result
PREFIX := /usr/local

all: $(target)

###
# Automatic dependency tracking
#
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

###
# Compile the program
#
$(target): $(objs)
	$(CC) $(CFLAGS) -o $@ $^

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< $(DEPFLAGS)

###
# Phony targets
#
.PHONY: install uninstall clean
install: $(target)
	mkdir -p $(PREFIX)/bin
	cp $(target) $(PREFIX)/bin/$(target)

uninstall:
	rm -f $(PREFIX)/bin/$(target)

clean:
	rm -f $(target) $(objs) $(deps)
