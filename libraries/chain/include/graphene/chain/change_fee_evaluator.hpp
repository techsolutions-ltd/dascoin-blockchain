#pragma once
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {


class change_fee_evaluator : public evaluator<change_fee_evaluator>
{
   public:
      typedef change_operation_fee_operation operation_type;

      void_result do_evaluate( const change_operation_fee_operation& o );
      void_result do_apply( const change_operation_fee_operation& o );
};

class change_fee_pool_account_evaluator : public evaluator<change_fee_pool_account_evaluator>
{
   public:
      typedef change_fee_pool_account_operation operation_type;

      void_result do_evaluate( const change_fee_pool_account_operation& o );
      void_result do_apply( const change_fee_pool_account_operation& o );
};

} }  // namespace graphene::chain
