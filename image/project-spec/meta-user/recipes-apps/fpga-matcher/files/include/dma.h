#ifndef DMA1_H
#define DMA1_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>

/**************************** Type Definitions ******************************/
typedef uint32_t u32;

void dmaInit();

void dmaTransfer(u32 * src, u32 * dst, u32 sizeDMA);
void dmaEnableLogSet(bool enable);
void dmaLogLevelSet(enum log_level level);

#ifdef __cplusplus
}
#endif

#endif
