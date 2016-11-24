/**
 * DASCOIN!
 */
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/personal_identity_evaluator.hpp>

namespace graphene { namespace chain {

void_result update_pi_limits_evaluator::do_evaluate(const update_pi_limits_operation& op)
{ try {

  const database& d = db();
  const auto gpo = d.get_global_properties();

  account_id_type pi_validator_id = gpo.authorities.pi_validator;
  const account_object& account = op.account(d);

  FC_ASSERT( pi_validator_id == op.pi_validator );  // This is signed by the current validator id.

  // TODO: check if the level is OK?

  if( op.new_limits.valid() )
  {
    auto min = gpo.parameters.minimum_transfer_limit;
    auto max = gpo.parameters.maximum_transfer_limit;
    for ( auto limit : *op.new_limits )
    {
      FC_ASSERT( limit > min && limit < max );
    }
  }
  acnt = &account;

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_pi_limits_evaluator::do_apply(const update_pi_limits_operation& op)
{ try {
  auto& d = db();
  // Update the levels and the limits on the account:
  db().modify(*acnt, [&](account_object& a) {
    a.pi_level = op.level;
    if ( op.new_limits.valid() )
      a.limits = *op.new_limits;
  });

  if ( acnt->is_vault() )
  {
    // For each parent check if the parents pi_level is less then the updated one and, in that case, update the
    // pi_level and the limits.
    for ( auto parent_id : acnt->parents )
    {
      const auto& parent = parent_id(d);
      if ( parent.pi_level < op.level )
        d.modify(parent, [&](account_object& a){
          a.pi_level = op.level;
          if ( op.new_limits.valid() )
            a.limits = *op.new_limits;
        });
    }
  }
  return void_result();

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain

