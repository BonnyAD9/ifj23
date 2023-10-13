TARGET:=ifj23
CFLAGS:=-g -Wall -std=c17 -fsanitize=address
RFLAGS:=-std=c17 -DNDEBUG -O3

SRC:=$(wildcard src/*.c)
SRCTEST:=$(wildcard test/*.c)
DOBJ:=$(patsubst src/%.c, obj/debug/%.o, $(SRC))
ROBJ:=$(patsubst src/%.c, obj/release/%.o, $(SRC))
TOBJ:=$(patsubst test/%.c, obj/debug/%.o, $(SRCTEST))
DOBJTEST:=$(patsubst src/%.c,obj/debug/%.o,$(filter-out src/main.c,$(SRC)))

-include dep.d

.DEFAULT_GOAL:=debug

.PHONY: debug release clean rel deb test


debug:
	mkdir -p obj/debug
	$(CC) -MM $(SRC) | sed -r 's/^.*:.*$$/obj\/debug\/\0/' > dep.d
	$(MAKE) deb

release:
	mkdir -p obj/release
	$(CC) -MM $(SRC) | sed -r 's/^.*:.*$$/obj\/release\/\0/' > dep.d
	$(MAKE) rel

deb: $(DOBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS)

rel: $(ROBJ)
	$(CC) $(RFLAGS) $^ -o $(TARGET) $(LDFLAGS)

obj/release/%.o: src/%.c
	$(CC) $(RFLAGS) -c -o $@ $<

obj/debug/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm obj/debug/*.o obj/release/*.o $(TARGET) dep.d

test: $(TOBJ) $(DOBJTEST)
	$(CC) $(CFLAGS) $^ -o lex_test $(LDFLAGS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LDFLAGS)

obj/debug/%.o: test/%.c
	$(CC) $(CFLAGS) -c -o $@ $<