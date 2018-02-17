/*
* Placeholder PetaLinux user application.
*
* Replace this with your application code

* Copyright (C) 2013 - 2016  Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or (b) that interact
* with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string>
// #include <xuartps.h>
#include <xeuclidean.h>
#include <dma.h>
#include <debug.h>

#define NUM_FEATURES 120
#define SIZE_DESC 64

float u32_to_float(u32 val);
unsigned int float_to_u32(float val);

static u32 * featureTable1Addr = (u32 *) 0xC0000000;
static u32 * featureTable2Addr = (u32 *) 0xC2000000;
static u32 * distancesTableAddr = (u32 *) 0xC4000000;
static u32 * indexTableAddr = (u32 *) 0x80000000;
static u32 * sendAddr  = (u32 *) 0xFFFF8000;

static volatile uint32_t * dmaMemInt = NULL;
static volatile float * dmaMemFloat = NULL;
static void * dmaMem  = NULL;
static volatile uint32_t * indexTable  = NULL; //

static XEuclidean euclidean_block;

void start_euclidean(float th) {
  int j = 0;

  XEuclidean_Set_con(&euclidean_block, 1);
  XEuclidean_Set_tresh(&euclidean_block, float_to_u32(th));
  XEuclidean_Start(&euclidean_block);

  while(XEuclidean_IsDone(&euclidean_block)==0){
		j++;
	}

  LOG(LOG_DEBUG, "L(%d) - done status %s\n", __LINE__, XEuclidean_IsDone(&euclidean_block) ? "True" : "False");
  LOG(LOG_DEBUG, "L(%d) - idle status %s\n", __LINE__, XEuclidean_IsIdle(&euclidean_block) ? "True" : "False");
  LOG(LOG_DEBUG, "L(%d) - wait time (loop) %d\n", __LINE__, j);
}

int test_ones_vector() {
  bool comp = false;
  uint32_t expectedIndex = 0;

  LOG(LOG_INFO, "Running Ones Descriptors test\n");

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 1 - Filled with ones descriptors\n");
  for (int i = 0; i < NUM_FEATURES; i++) {
    for (int j = 0; j < SIZE_DESC; j++) {
      // dmaTransfer(sendAddr + i*SIZE_DESC*sizeof(uint32_t), (u32*)((uint32_t)featureTable1Addr + i*SIZE_DESC*sizeof(uint32_t)), SIZE_DESC*sizeof(uint32_t));
      dmaMemInt[i*SIZE_DESC + j] = float_to_u32(1.0f);
    }
  }
  dmaTransfer(sendAddr, featureTable1Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 2 - Filled with zeros\n");
  memset((void *)dmaMemInt, 0x00, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));
  dmaTransfer(sendAddr, featureTable2Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  start_euclidean(1000.0f);

  dmaTransfer(distancesTableAddr, sendAddr, NUM_FEATURES*sizeof(uint32_t));
  for (int i = 0; i < NUM_FEATURES; i++) {
    comp = (dmaMemFloat[i] == 8.0f);
    LOG(LOG_DEBUG, "L(%d) - %d-th distance %f\n", __LINE__, i, (volatile float)dmaMemFloat[i]);
    if (!comp) {
      LOG(LOG_ERROR, "Distance is differente than 8.0f\n");
      return -1;
    }

    comp = (indexTable[i] == expectedIndex);
    LOG(LOG_DEBUG, "L(%d) - %d-th index %d\n", __LINE__, i, indexTable[i]);

    if (!comp) {
      LOG(LOG_ERROR, "Index differente than %d\n", expectedIndex);
      return -1;
    }
  }

  LOG(LOG_INFO, "Ones Descriptors test PASSED\n");
  return 0;
}

int test_fs_vector() {
  bool comp = false;
  uint32_t expectedIndex = 255;

  LOG(LOG_INFO, "Running 0xFFFFFFFF Descriptors test\n");

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 1 - Filled with 0xFFFFFFFF descriptors\n");
  memset((void *)dmaMemInt, 0xFF, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));
  dmaTransfer(sendAddr, featureTable1Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 2 - Filled with zeros\n");
  memset((void *)dmaMemInt, 0x00, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));
  dmaTransfer(sendAddr, featureTable2Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  start_euclidean(1000.0f);

  dmaTransfer(distancesTableAddr, sendAddr, NUM_FEATURES*sizeof(uint32_t));
  for (int i = 0; i < NUM_FEATURES; i++) {
    comp = (dmaMemFloat[i] == 1000.0f);
    LOG(LOG_DEBUG, "L(%d) - %d-th distance %f\n", __LINE__, i, (volatile float)dmaMemFloat[i]);
    if (!comp) {
      LOG(LOG_ERROR, "Distance is differente than 1000.0f\n");
      return -1;
    }

    // FIXME: This is an issue in the design it should be 0xFFFFFFFF
    comp = (indexTable[i] == expectedIndex);
    LOG(LOG_DEBUG, "L(%d) - %d-th index %d\n", __LINE__, i, indexTable[i]);

    if (!comp) {
      LOG(LOG_ERROR, "Index differente than %d\n", expectedIndex);
      return -1;
    }
  }

  LOG(LOG_INFO, "Fs Descriptors test PASSED\n");
  return 0;
}

int test_matched_vector() {
  bool comp = false;
  int zeroFeat = 50;
  const float expectedDist = 0.0f;

  LOG(LOG_INFO, "Running 0xFFFFFFFF Descriptors test\n");

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 1 - Filled with Zeros descriptors\n");
  memset((void *)dmaMemInt, 0x00, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  dmaTransfer(sendAddr, featureTable1Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  LOG(LOG_DEBUG, "L(%d) - checkpoint\n", __LINE__);

  LOG(LOG_INFO, "Features Table 2 - Filled with 0xFF\n");
  memset((void *)dmaMemInt, 0xFF, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));
  LOG(LOG_INFO, "Setting Feature %d to Zeros\n", zeroFeat);
  memset((void *)((uint32_t)dmaMemInt + zeroFeat * SIZE_DESC * sizeof(uint32_t)), 0x00, SIZE_DESC*sizeof(uint32_t));
  dmaTransfer(sendAddr, featureTable2Addr, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t));

  start_euclidean(1000.0f);

  dmaTransfer(distancesTableAddr, sendAddr, NUM_FEATURES*sizeof(uint32_t));
  for (int i = 0; i < NUM_FEATURES; i++) {
    comp = (dmaMemFloat[i] == expectedDist);
    LOG(LOG_DEBUG, "L(%d) - %d-th distance %f\n", __LINE__, i, (volatile float)dmaMemFloat[i]);

    if (!comp) {
      LOG(LOG_ERROR, "Distance is differente than %f\n", expectedDist);
      return -1;
    }

    comp = (indexTable[i] == zeroFeat);
    LOG(LOG_DEBUG, "L(%d) - %d-th index %d\n", __LINE__, i, indexTable[i]);

    if (!comp) {
      LOG(LOG_ERROR, "Index differente than %d\n", zeroFeat);
      return -1;
    }

  }

  LOG(LOG_INFO, "Matched test PASSED\n");
  return 0;
}

void usage(){
  printf("Usage of fpga-matcher\n");
  printf("  fpga-matcher [-d true|false] [-l debug|info|warning|error] [-h] \n");
}

void parse_cmd_line(int argc, char *argv[]) {
  int opt = 0;
  std::string val = {};
  // const char * true_string = 'true';

  while ((opt = getopt(argc, argv, "d:l:h")) != -1) {
    switch(opt) {
    case 'd':
      val = optarg;
      if (!val.compare("true")) {
        LOG_ENABLE_SET(true);
        dmaEnableLogSet(true);
      }
      break;
    case 'l':
      val = optarg;
      if (!val.compare("debug")) {
        LOG_LEVEL_SET(LOG_DEBUG);
        dmaLogLevelSet(LOG_DEBUG);
      }

      if (!val.compare("info")) {
        LOG_LEVEL_SET(LOG_INFO);
        dmaLogLevelSet(LOG_INFO);
      }

      if (!val.compare("warning")) {
        LOG_LEVEL_SET(LOG_WARNING);
        dmaLogLevelSet(LOG_WARNING);
      }

      if (!val.compare("error")) {
        LOG_LEVEL_SET(LOG_ERROR);
        dmaLogLevelSet(LOG_ERROR);
      }
      break;
    case 'h':
      usage();
      break;
    default:
      continue;
    }
  }
}

int main(int argc, char *argv[]){
  int fd = 0;
  int ret = 0;

  LOG_ENABLE_SET(true);
  LOG_LEVEL_SET(LOG_WARNING);
  dmaEnableLogSet(true);
  dmaLogLevelSet(LOG_WARNING);


  parse_cmd_line(argc, argv);

  fd = open("/dev/mem", O_RDWR);
  if (fd < 0) {
    LOG(LOG_ERROR, "Error opening /dev/mem\n");
    exit(-1);
  }
  LOG(LOG_INFO, "/dev/mem opened successfully\n");

  dmaMem = mmap(NULL, NUM_FEATURES*SIZE_DESC*sizeof(uint32_t), PROT_READ | PROT_WRITE,  MAP_SHARED, fd, (uint32_t) sendAddr);
  if (dmaMem <= 0) {
    LOG(LOG_ERROR, "Error mapping features memory\n");
    exit(-1);
  }
  LOG(LOG_INFO, "Features Memory mapped correctly\n");

  dmaMemInt = (uint32_t *) dmaMem;
  dmaMemFloat = (float *) dmaMem;

  indexTable = (uint32_t *) mmap(NULL, NUM_FEATURES*sizeof(uint32_t), PROT_READ | PROT_WRITE,  MAP_SHARED, fd, (uint32_t) indexTableAddr);
  if (indexTable <= 0) {
    LOG(LOG_ERROR, "Error mapping index table memory\n");
    exit(-1);
  }
  LOG(LOG_INFO, "Index table Memory mapped correctly\n");

  dmaInit();
  LOG(LOG_INFO, "DMA Initialized\n");

  XEuclidean_Initialize(&euclidean_block, "euclidean");
  LOG(LOG_INFO, "Euclidean Block Initialized\n");

  ret = test_ones_vector();
  if (ret) {
    LOG(LOG_ERROR, "Ones Descriptors test failed\n");
    exit(-1);
  }

  ret = test_fs_vector();
  if (ret) {
    LOG(LOG_ERROR, "Fs Descriptors test failed\n");
    exit(-1);
  }

  ret = test_matched_vector();
  if (ret) {
    LOG(LOG_ERROR, "Matched test failed\n");
    exit(-1);
  }

}

float u32_to_float(u32 val){
	union u_type{
	  u32 u32_part;
	  float f_part;
	}cnvrt;

	cnvrt.u32_part = val;

	return cnvrt.f_part;
}

unsigned int float_to_u32(float val){
	unsigned int result;
	union float_bytes{
		float v;
		unsigned char bytes[4];
	} data;
	data.v = val;
	result = ((data.bytes[3] << 24) + (data.bytes[2]<<16) + (data.bytes[1] << 8) + (data.bytes[0] << 0));
	return result;

}
