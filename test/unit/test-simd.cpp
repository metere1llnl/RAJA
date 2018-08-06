//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-18, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For details about use and distribution, please read RAJA/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "RAJA/RAJA.hpp"
#include "RAJA_gtest.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cassert>


using namespace RAJA;
using namespace RAJA::statement;

TEST(SIMD, Align){

  int N = 1024;
  double c = 0.5;
  double *a = RAJA::allocate_aligned_type<double>(RAJA::DATA_ALIGN,N*sizeof(double));
  double *b = RAJA::allocate_aligned_type<double>(RAJA::DATA_ALIGN,N*sizeof(double));
    
  for(int i=0; i<N; ++i) 
    {
      a[i] = 0; 
      b[i] = 2.0;
    }


  auto *y = RAJA::align_hint(a);
  auto *x = RAJA::align_hint(b);

  RAJA::forall<RAJA::simd_exec>(RAJA::RangeSegment(0, N), [=] (int i) {
      y[i] += x[i] * c;
    });

  for(int i=0; i<N; ++i)
    {
      ASSERT_FLOAT_EQ(y[i], 1.0);
    }

  delete[] a;
  delete[] b;
}

TEST(SIMD, ThreadsAndSimd){

    using POL =
    RAJA::KernelPolicy<
      RAJA::statement::For<1, RAJA::omp_parallel_for_exec,
        RAJA::statement::For<0, RAJA::simd_exec,
          RAJA::statement::Lambda<0>
        >
      >
    >;
    
    const RAJA::Index_type N = 32;
    const RAJA::Index_type M = 32;

    double *a = RAJA::allocate_aligned_type<double>(RAJA::DATA_ALIGN, N*M*sizeof(double));
    double *b = RAJA::allocate_aligned_type<double>(RAJA::DATA_ALIGN, N*M*sizeof(double));
    double *c = RAJA::allocate_aligned_type<double>(RAJA::DATA_ALIGN, N*M*sizeof(double));

    for(int i=0; i<N*M; ++i)
      {      
        a[i] = 1;
        b[i] = 1;
        c[i] = 0.0;
      }
    
    RAJA::kernel<POL>
    (RAJA::make_tuple(RAJA::RangeSegment(0, N), RAJA::RangeSegment(0, M)),
     [=](RAJA::Index_type i, RAJA::Index_type j) {
        c[i + j*N] = a[i + j*N] + b[i + j*N];
    });

    for(int i=0; i<N*M; ++i)
    {
      ASSERT_FLOAT_EQ(c[i], 2.0);
    }
      
    delete[] a;
    delete[] b;
    delete[] c;
}
