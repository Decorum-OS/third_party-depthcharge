/*
 * coreboot interface to memory-saving variant of LZMA decoder
 *
 * Copyright (C) 2006 Carl-Daniel Hailfinger
 * Released under the BSD license
 *
 * Parts of this file are based on C/7zip/Compress/LZMA_C/LzmaTest.c from the LZMA
 * SDK 4.42, which is written and distributed to public domain by Igor Pavlov.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "base/lzma/lzma.h"
#include "base/lzma/priv.h"

size_t ulzman(const void *src, size_t srcn, void *dst, size_t dstn)
{
	unsigned char properties[LZMA_PROPERTIES_SIZE];
	const int data_offset = LZMA_PROPERTIES_SIZE + 8;
	uint32_t outSize;
	size_t inProcessed;
	size_t outProcessed;
	int res;
	CLzmaDecoderState state;
	size_t mallocneeds;
	unsigned char scratchpad[15980];

	memcpy(properties, src, LZMA_PROPERTIES_SIZE);
	ssize_t size = ulzma_expanded_size(src, srcn);
	if (size < -1)
		return 0;
	else if (size == -1 || size > dstn)
		outSize = dstn;
	else
		outSize = size;
	if (LzmaDecodeProperties(&state.Properties, properties,
				 LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK) {
		printf("lzma: Incorrect stream properties.\n");
		return 0;
	}
	mallocneeds = (LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
	if (mallocneeds > 15980) {
		printf("lzma: Decoder scratchpad too small!\n");
		return 0;
	}
	state.Probs = (CProb *)scratchpad;
	res = LzmaDecode(&state, (uint8_t *)src + data_offset,
			 srcn - data_offset, &inProcessed,
			 (uint8_t *)dst, outSize, &outProcessed);
	if (res != 0) {
		printf("lzma: Decoding error = %d\n", res);
		return 0;
	}
	return outProcessed;
}

size_t ulzma(const void *src, void *dst)
{
	return ulzman(src, (size_t)(-1), dst, (size_t)(-1));
}

ssize_t ulzma_expanded_size(const void *src, size_t srcn)
{
	uint64_t size;
	if (srcn < LZMA_PROPERTIES_SIZE + sizeof(size)) {
		printf("lzma: Truncated stream.\n");
		return -2;
	}
	memcpy(&size, (const uint8_t *)src + LZMA_PROPERTIES_SIZE,
	       sizeof(size));
	if (size == ~(uint64_t)0) {
		printf("lzma: Unknown lzma size.\n");
		return -1;
	}

	return size;
}
