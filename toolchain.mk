CFLAGS   += -Os -Wall
CPPFLAGS += -I$(build)/include -I$(src)/include -MMD
LDFLAGS  := -nostdlib -T$(obj)/link.ld

PYTHON   := python3
MSCIMG   := $(PYTHON) util/mkmscimg.py
ifeq ($(CONFIG_CPU_BYTEORDER), "little")
MSCIMG   += --little
endif

objs.o   := $(filter %.o,$(objs))
objs.ld  := $(filter %.ld,$(objs))
deps     := $(objs.o:.o=.d) $(objs.ld:.ld=.d)

ifdef crt-y
crt-src  := $(top)/3rdparty/llvm-project/compiler-rt/lib/builtins
crt-obj  := $(build)/compiler-rt

crt-objs := $(addprefix $(crt-obj)/,$(crt-y))

$(foreach obj,$(crt-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS  += -L$(build)
LDLIBS   += -lcompiler-rt
deps     += $(crt-objs:.o=.d)

CPPFLAGS_crt := -I$(src)/include -I$(crt-src) -D'mode(x)='
ifeq ($(CONFIG_CPU_BYTEORDER), "little")
CPPFLAGS_crt += -D_YUGA_LITTLE_ENDIAN=1
else
CPPFLAGS_crt += -D_YUGA_BIG_ENDIAN=1
endif

$(crt-obj)/%.o: CPPFLAGS = $(CPPFLAGS_crt)

main.tar.gz:
	wget https://github.com/llvm/llvm-project/archive/main.tar.gz

$(crt-src)/%.c: main.tar.gz
	mkdir -p $(top)/3rdparty/llvm-project
	tar xf main.tar.gz --strip-components=1 -C $(top)/3rdparty/llvm-project

$(crt-obj)/%.o: $(crt-src)/%.c
	$(COMPILE.c) -o $@ $<

$(crt-obj)/%.o: $(crt-src)/%.S
	$(COMPILE.S) -o $@ $<

$(build)/libcompiler-rt.a: $(crt-objs)
	$(AR) crs $@ $^

$(build)/bios.elf: $(build)/libcompiler-rt.a
endif

-include deps

$(obj)/%.ld: $(src)/%.ld.S
	$(CPP) $(CPPFLAGS) -P -o $@ $<

$(obj)/%.o: $(src)/%.c
	$(COMPILE.c) -o $@ $<

$(obj)/%.o: $(src)/%.S
	$(COMPILE.S) -o $@ $<

$(build)/bios.elf: $(objs)
	$(LINK.o) -o $@ $(objs.o) $(LDLIBS)

$(build)/bios.bin: $(build)/bios.elf
	$(OBJCOPY) -O binary $< $@
	$(MSCIMG) $@
