ifdef CONFIG_CPU_MINERVA
LITEX_CPU_DIR  := $(top)/3rdparty/litex/litex/soc/cores/cpu/minerva
CPPFLAGS_litex := -D__minerva__
else
$(error Unsupported CPU)
endif

LITEX_SW_DIR  := $(top)/3rdparty/litex/litex/soc/software
LITEX_INC_DIR := $(top)/3rdparty/litex/litex/soc/software/include

LIBBASE_GENERATED     := $(top)/src/litex/base/include
LIBLITEDRAM_GENERATED := $(top)/src/litex/litedram/include
LIBLITEETH_GENERATED  := $(top)/src/litex/liteeth/include

CPPFLAGS_litex += \
	-nostdinc \
	-I$(LITEX_CPU_DIR) \
	-I$(LITEX_SW_DIR) \
	-I$(LITEX_INC_DIR) \
	-I$(LITEX_INC_DIR)/base \
	-I$(build) \

litex-obj := $(obj)/3rdparty/litex

ifdef crt-y
liblitex-objs += $(crt-objs)
endif

ifdef libbase-y
CPPFLAGS_libbase = \
	-I$(LIBBASE_GENERATED) \

libbase-src   := $(LITEX_SW_DIR)/libbase
libbase-obj   := $(litex-obj)/libbase
liblitex-objs += $(addprefix $(libbase-obj)/,$(libbase-y))

$(libbase-obj)/%.o: CPPFLAGS = $(CPPFLAGS_litex) $(CPPFLAGS_libbase)
$(libbase-obj)/%.o: $(libbase-src)/%.c
	$(COMPILE.c) -o $@ $<
endif

ifdef liblitedram-y
CPPFLAGS_liblitedram = \
	-I$(LIBLITEDRAM_GENERATED) \
	-I$(litedram_dir) \

liblitedram-src := $(LITEX_SW_DIR)/liblitedram
liblitedram-obj := $(litex-obj)/liblitedram
liblitex-objs   += $(addprefix $(liblitedram-obj)/,$(liblitedram-y))

$(liblitedram-obj)/%.o: CPPFLAGS = $(CPPFLAGS_litex) $(CPPFLAGS_liblitedram)
$(liblitedram-obj)/%.o: $(liblitedram-src)/%.c
	$(COMPILE.c) -o $@ $<
endif

ifdef libliteeth-y
CPPFLAGS_libliteeth = \
	-I$(LIBLITEETH_GENERATED) \
	-I$(liteeth_dir) \

libliteeth-src := $(LITEX_SW_DIR)/libliteeth
libliteeth-obj := $(litex-obj)/libliteeth
liblitex-objs  += $(addprefix $(libliteeth-obj)/,$(libliteeth-y))

$(libliteeth-obj)/%.o: CPPFLAGS = $(CPPFLAGS_litex) $(CPPFLAGS_libliteeth)
$(libliteeth-obj)/%.o: $(libliteeth-src)/%.c
	$(COMPILE.c) -o $@ $<
endif

$(foreach obj,$(liblitex-objs), \
	$(eval dirs += $(dir $(obj))))

LDFLAGS += -L$(litex-obj)
LDLIBS  += -llitex
deps    += $(liblitex-objs:.o=.d)

$(litex-obj)/liblitex.a: $(liblitex-objs)
	$(AR) crs $@ $^

$(obj)/bios.elf: $(litex-obj)/liblitex.a
