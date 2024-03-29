/*H**********************************************************************
*       Copyright Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko 2019.  All rights reserved.
*H*/
#ifndef OSDNA_COMPRESSION_CORE_H
#define OSDNA_COMPRESSION_CORE_H

#include "osdna_compressor.h"

osdna_status compress_core(OSDNA_ctx *ctx);
osdna_status decompress_core(OSDNA_ctx *ctx);

#endif //OSDNA_COMPRESSION_CORE_H
