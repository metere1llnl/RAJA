/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file for kernel lambda executor.
 *
 ******************************************************************************
 */


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


#ifndef RAJA_pattern_kernel_Lambda_HPP
#define RAJA_pattern_kernel_Lambda_HPP

#include "RAJA/config.hpp"

#include <iostream>
#include <type_traits>

#include "camp/camp.hpp"
#include "camp/concepts.hpp"
#include "camp/tuple.hpp"

#include "RAJA/util/macros.hpp"
#include "RAJA/util/types.hpp"
#include "RAJA/util/ShmemTile.hpp"

#include "RAJA/pattern/kernel/internal.hpp"
#include <typeinfo>

namespace RAJA
{

namespace statement
{

/*!
 * A RAJA::kernel statement that invokes a lambda function.
 *
 * The lambda is specified by its index in the sequence of lambda arguments
 * to a RAJA::kernel method.
 *
 * for example:
 * RAJA::kernel<exec_pol>(make_tuple{s0, s1, s2}, lambda0, lambda1);
 *
 */
template <camp::idx_t BodyIdx>
struct Lambda : internal::Statement<camp::nil> {
  const static camp::idx_t loop_body_index = BodyIdx;
};

//This statement will create shared memory
struct CreateShmem : public internal::Statement<camp::nil> {
};

}  // end namespace statement

namespace internal
{

template <camp::idx_t LoopIndex>
struct StatementExecutor<statement::Lambda<LoopIndex>> {

  template <typename Data>
  static RAJA_INLINE void exec(Data &&data)
  {
    invoke_lambda<LoopIndex>(std::forward<Data>(data));
  }
};


//Traverse Shared Memory
template<camp::idx_t id_num, typename Data>
RAJA_INLINE void createSharedMemory(Data &&data)
{

  auto tuple_size = camp::tuple_size<typename camp::decay<Data>::param_tuple_t>::value;

  //const auto id_num = idx;
  using varType = typename camp::tuple_element_t<id_num, typename camp::decay<Data>::param_tuple_t>::type;
  varType rajaShared;
  camp::get<id_num>(data.param_tuple).SharedMem = &rajaShared;

}


template<bool T, camp::idx_t id_num>
struct SharedMemHelper
{

  template<typename Data>
  static RAJA_INLINE void createShared(Data && data)
  {  
    using varType = typename camp::tuple_element_t<id_num, typename camp::decay<Data>::param_tuple_t>::type;
    varType rajaShared;
    camp::get<id_num>(data.param_tuple).SharedMem = &rajaShared;
    
    //Check contents of tuple
    constexpr bool tcheck = (id_num+1) < camp::tuple_size<typename camp::decay<Data>::param_tuple_t>::value;
    SharedMemHelper<tcheck,id_num+1>::createShared(data);
  }

};

template<camp::idx_t id_num>
struct SharedMemHelper<false, id_num>
{
  template<typename Data>
  static RAJA_INLINE void createShared(Data && )
  {
    //do nothing...
  }
};


template<>
struct StatementExecutor<statement::CreateShmem >{

  template<typename Data>
  static RAJA_INLINE void exec(Data &&data)
  {
    
    //traverse the tuple and create instances of objects
    constexpr bool tcheck = 0 < camp::tuple_size<typename camp::decay<Data>::param_tuple_t>::value;
    SharedMemHelper<tcheck,0>::createShared(data);    
  }
};

}  // namespace internal

}  // end namespace RAJA


#endif /* RAJA_pattern_kernel_HPP */
