#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
  typedef void *handle_t;

  struct range_t
  {
    uint64_t start;
    uint64_t count;
  };

#ifndef __cplusplus
  typedef struct range_t range_t;
#endif

  struct buffer_t
  {
    uint8_t *p;
    uint64_t len;
  };

#ifndef __cplusplus
  typedef struct buffer_t buffer_t;
#endif

  struct plain_bulletin_t
  {
    uint64_t size;
    uint64_t s;
    uint64_t n;
    uint8_t sigma_mkl_root[32];
  };

#ifndef __cplusplus
  typedef struct plain_bulletin_t plain_bulletin_t;
#endif

  struct table_bulletin_t
  {
    uint64_t s;
    uint64_t n;
    uint8_t sigma_mkl_root[32];
    uint8_t vrf_meta_digest[32];
  };

#ifndef __cplusplus
  typedef struct table_bulletin_t table_bulletin_t;
#endif

#ifdef __cplusplus
}
#endif
