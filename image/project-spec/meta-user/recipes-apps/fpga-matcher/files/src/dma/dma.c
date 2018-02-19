#include <dma.h>
#include <debug.h>

/* Write to memory location or register */
#define X_mWriteReg(RegOffset, data) \
           *(unsigned int *)((uint32_t)ptr + RegOffset) = ((unsigned int) data);
/* Read from memory location or register */
#define X_mReadReg(RegOffset) \
           *(unsigned int *)((uint32_t)ptr + RegOffset);

#define DMA_BASE_ADDR 0x8E200000
#define DMA_SIZE 0x10000

#define CDMACR  0x00
#define CDMASR  0x04
#define CDMASA  0x18
#define CDMADA  0x20
#define CDMABTT 0x28

#define CDMASR_IDLE  0x00000002

static void *ptr;

void dmaInit() {
  int fd;
  unsigned addr, page_addr, page_offset;
  unsigned page_size=sysconf(_SC_PAGESIZE);

  fd = open("/dev/mem",O_RDWR);
  if(fd < 1) {
    LOG(LOG_ERROR, "Error opening /dev/mem\n");
    exit(-1);
  }

  LOG(LOG_INFO, "/dev/mem successfully open\n");

  addr = DMA_BASE_ADDR;

  ptr = mmap(NULL, DMA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
  if((int)ptr==-1) {
    LOG(LOG_INFO, "Error during mmap");
    exit(-1);
  }

  LOG(LOG_INFO, "Memory successfully mapped\n");
}

void dmaEnableLogSet(bool enable) {
  LOG_ENABLE_SET(enable);
}

void dmaLogLevelSet(enum log_level level) {
  LOG_LEVEL_SET(level);
}

void dmaTransfer(u32 * src, u32 * dst, u32 sizeDMA){

	LOG(LOG_INFO, "Waiting for DMA to be ready\n");
	volatile int value = X_mReadReg(CDMASR);
	while(!(value & CDMASR_IDLE)){
		value = X_mReadReg(CDMASR);
	}
	LOG(LOG_INFO, "DMA ready\n");

	LOG(LOG_DEBUG, " ... programming\n");

	X_mWriteReg(CDMACR, 0x00005000);	// set control reg Error Interrupt Enable
												//Complete Interrupt Enable

	X_mWriteReg(CDMASA, (unsigned int)src);	// set src addr reg
	X_mWriteReg(CDMADA, (unsigned int)dst);	 // set dst addr reg

  value = X_mReadReg(CDMASA);
  LOG(LOG_DEBUG, " DMA src address 0x%08x\n", value);
  value = X_mReadReg(CDMADA);
  LOG(LOG_DEBUG, " DMA dst address 0x%08x\n", value);

	LOG(LOG_DEBUG, " ...DMA status before sending\n");

	value = X_mReadReg(CDMASR);
	LOG(LOG_DEBUG, "status: 0x%08x = 0x%08x\n", DMA_BASE_ADDR + CDMASR, value);
	X_mWriteReg(CDMABTT, sizeDMA);//0x00000100);	// set number of Byte trans reg (64 bytes) TRIGGER DMA SEND
  value = X_mReadReg(CDMABTT);
  LOG(LOG_DEBUG, "size: 0x%08x = 0x%08x\n", DMA_BASE_ADDR + CDMABTT, value);

	LOG(LOG_DEBUG, " ... polling DMA status\n");
	value = X_mReadReg(CDMASR);
	while(!(value & CDMASR_IDLE)){
		value = X_mReadReg(CDMASR);
    LOG(LOG_DEBUG, "status: 0x%08x = 0x%08x\n", DMA_BASE_ADDR + CDMASR, value);
	}

  LOG(LOG_INFO, "DMA transfer finished {src: 0x%08x, dst: 0x%08x, size: %d}\n", src, dst, sizeDMA);

	value = X_mReadReg(CDMASR);	// get DMA status
	LOG(LOG_DEBUG, "status: 0x%08x = 0x%08x\n", DMA_BASE_ADDR + CDMASR, value);
	X_mWriteReg(CDMASR, 0x00001000);	//clean interrupt for next one
	value = X_mReadReg(CDMASR);	// get DMA status
	LOG(LOG_DEBUG, "status: 0x%08x = 0x%08x\n", DMA_BASE_ADDR + CDMASR, value);
	X_mWriteReg(CDMACR, 0x00000004);	//RESET DMA>> maybe this isnt the best idea.
	LOG(LOG_INFO, "DMA ready for next transaction\n");
}
