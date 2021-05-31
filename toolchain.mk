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


# Compiler-RT

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

$(crt-obj)/%.o: $(crt-src)/%.c
	$(COMPILE.c) -o $@ $<

$(crt-obj)/%.o: $(crt-src)/%.S
	$(COMPILE.S) -o $@ $<

$(build)/libcompiler-rt.a: $(crt-objs)
	$(AR) crs $@ $^

$(build)/bios.elf: $(build)/libcompiler-rt.a
endif


# LiteX libraries

LITEX_SW_DIR  := $(top)/3rdparty/litex/litex/soc/software
LITEX_INC_DIR := $(top)/3rdparty/litex/litex/soc/software/include
LITEX_CPU_DIR := $(top)/3rdparty/litex/litex/soc/cores/cpu/minerva # FIXME
LITEX_GEN_DIR := $(top)/src/drivers/sdram/include # FIXME move elsewhere

ifdef libbase-y
libbase-src  := $(top)/3rdparty/litex/litex/soc/software/libbase
libbase-obj  := $(build)/litex/libbase
libbase-objs := $(addprefix $(libbase-obj)/,$(libbase-y))

$(foreach obj,$(libbase-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS += -L$(build)/litex
LDLIBS  += -lbase

# FIXME: -D__minerva__
CPPFLAGS_libbase := -nostdinc -I$(LITEX_INC_DIR)/base -I$(LITEX_CPU_DIR) -I$(LITEX_GEN_DIR) -D__minerva__

$(libbase-obj)/%.o: CPPFLAGS = $(CPPFLAGS_libbase)

$(libbase-obj)/%.o: $(libbase-src)/%.c
	$(COMPILE.c) -o $@ $<

$(build)/litex/libbase.a: $(libbase-objs)
	$(AR) crs $@ $^

$(build)/bios.elf: $(build)/litex/libbase.a
endif

ifdef liblitedram-y
liblitedram-src  := $(top)/3rdparty/litex/litex/soc/software/liblitedram
liblitedram-obj  := $(build)/litex/liblitedram
liblitedram-objs := $(addprefix $(liblitedram-obj)/,$(liblitedram-y))

$(foreach obj,$(liblitedram-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS += -L$(build)/litex
LDLIBS  += -llitedram

# FIXME: -D__minerva__
CPPFLAGS_liblitedram := -nostdinc -I$(LITEX_SW_DIR) -I$(LITEX_INC_DIR)/base -I$(LITEX_CPU_DIR) -I$(LITEX_GEN_DIR) -D__minerva__

$(liblitedram-obj)/%.o: CPPFLAGS = $(CPPFLAGS_liblitedram)

$(liblitedram-obj)/%.o: $(liblitedram-src)/%.c
	$(COMPILE.c) -o $@ $<

$(build)/litex/liblitedram.a: $(liblitedram-objs) $(libbase-objs) $(crt-objs)
	$(AR) crs $@ $^

$(build)/bios.elf: $(build)/litex/liblitedram.a
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
