/*
  LzmaDecode.h
  LZMA Decoder interface

  LZMA SDK 4.40 Copyright (c) 1999-2006 Igor Pavlov (2006-05-01)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to
  statically or dynamically link your code (or bind by name) to the
  interfaces of this file without subjecting your linked code to the
  terms of the CPL or GNU LGPL. Any modifications or additions
  to this file, however, are subject to the LGPL or CPL terms.
*/

#ifndef __BASE_LZMA_PRIV_H__
#define __BASE_LZMA_PRIV_H__

#include <stddef.h>
#include <stdint.h>

typedef uint16_t CProb;

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

typedef struct
{
	int lc;
	int lp;
	int pb;
} CLzmaProperties;

int LzmaDecodeProperties(CLzmaProperties *propsRes, const uint8_t *propsData,
			 int size);

#define LzmaGetNumProbs(Properties) \
	(LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + \
			   (Properties)->lp)))

#define kLzmaNeedInitId -2

typedef struct
{
	CLzmaProperties Properties;
	CProb *Probs;
} CLzmaDecoderState;


int LzmaDecode(CLzmaDecoderState *vs,
	       const uint8_t *inStream, size_t inSize, size_t *inSizeProcessed,
	       uint8_t *outStream, size_t outSize, size_t *outSizeProcessed);

#endif /* __BASE_LZMA_PRIV_H__ */
