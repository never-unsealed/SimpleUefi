#pragma once

#include "simple_uefi.h"

CHAR8 *base64Encode(const CHAR8 *in, size_t len);
int base64Decode(CHAR8 *in, CHAR8 *out, size_t outlen);
size_t base64DecodedSize(CHAR8 *in);