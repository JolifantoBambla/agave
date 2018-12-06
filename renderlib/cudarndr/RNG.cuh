#pragma once

#include "Geometry.h"

class CRNG
{
public:
  HOD CRNG(unsigned int* pSeed0, unsigned int* pSeed1)
  {
    m_pSeed0 = pSeed0;
    m_pSeed1 = pSeed1;
  }

  HOD float Get1(void)
  {
    *m_pSeed0 = 36969 * ((*m_pSeed0) & 65535) + ((*m_pSeed0) >> 16);
    *m_pSeed1 = 18000 * ((*m_pSeed1) & 65535) + ((*m_pSeed1) >> 16);

    unsigned int ires = ((*m_pSeed0) << 16) + (*m_pSeed1);

    union
    {
      float f;
      unsigned int ui;
    } res;

    res.ui = (ires & 0x007fffff) | 0x40000000;

    return (res.f - 2.f) / 2.f;
  }

  HOD float2 Get2(void) { return make_float2(Get1(), Get1()); }

  HOD float3 Get3(void) { return make_float3(Get1(), Get1(), Get1()); }

private:
  unsigned int* m_pSeed0;
  unsigned int* m_pSeed1;
};
