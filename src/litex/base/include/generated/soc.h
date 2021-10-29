#ifndef __LIBBASE_GENERATED_SOC_H
#define __LIBBASE_GENERATED_SOC_H

#include <litex_config.h>

#define CONFIG_CSR_DATA_WIDTH LX_CONFIG_SOC_CSR_DATA_WIDTH

#define CONFIG_CLOCK_FREQUENCY LX_CONFIG_SOC_CLOCK_FREQ

#ifdef LX_CONFIG_CPU_ARCH_RISCV
#define CONFIG_CPU_NOP "nop"
static inline const char * config_cpu_nop_read(void) {
	return "nop";
}
#else
#error "Unsupported CPU architecture"
#endif

#ifdef LX_CONFIG_SDRAM_CACHE_SIZE
#define CONFIG_L2_SIZE LX_CONFIG_SDRAM_CACHE_SIZE
static inline int config_l2_size_read(void) {
        return CONFIG_L2_SIZE;
}
#endif

#ifdef LX_CONFIG_SOC_MEMTEST_DATA_SIZE
#define MEMTEST_DATA_SIZE LX_CONFIG_SOC_MEMTEST_DATA_SIZE
static inline int memtest_data_size_read(void) {
	return MEMTEST_DATA_SIZE;
}
#endif

#ifdef LX_CONFIG_SOC_MEMTEST_ADDR_SIZE
#define MEMTEST_ADDR_SIZE LX_CONFIG_SOC_MEMTEST_ADDR_SIZE
static inline int memtest_addr_size_read(void) {
        return MEMTEST_ADDR_SIZE;
}
#endif

#endif
