bin_objects=$(patsubst ../src/%.c, ../src/%.o, $(wildcard ../src/*.c))


include mk/*.mk

.PHONY: static clean

static: $(bin_objects)
	$(AR) rcs libupoalglib_s.a $^

clean:
	$(RM) $(bin_objects) *.so *.a
