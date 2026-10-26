#ifndef SHELL_H
#define SHELL_H

#include "types.h"

/* Memory sizes reported by the bootloader, in KiB (0 if unknown). */
void shell_set_meminfo(uint32_t lower_kb, uint32_t upper_kb);
void shell_run(void);          /* never returns */

#endif
