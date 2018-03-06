#pragma once
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {


class change_fee_evaluator : public evaluator<change_fee_evaluator>
{
   public:
      typedef change_fee_for_operation operation_type;

      void_result do_evaluate( const change_fee_for_operation& o );
      void_result do_apply( const change_fee_for_operation& o );

   private:
      void perform_root_authority_check(database& db, const account_id_type& authority_account_id);
};

} }  // namespace graphene::chain
