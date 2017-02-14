/**
 * DASCOIN!
 */

#include <graphene/chain/database.hpp>

#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/license_evaluator.hpp>

class optional;

namespace graphene { namespace chain {

object_id_type database::create_license_type(license_kind kind, const string& name, share_type amount, 
                                   upgrade_multiplier_type balance_multipliers,
                                   upgrade_multiplier_type requeue_multipliers,
                                   upgrade_multiplier_type return_multipliers)
{
  return create<license_type_object>([&](license_type_object& lto){
    lto.name = name;
    lto.amount = amount;
    lto.kind = kind;
    lto.balance_upgrade.reset(balance_multipliers);
    lto.requeue_upgrade.reset(requeue_multipliers);
    lto.return_upgrade.reset(return_multipliers);
  }).id;
}

void database::edit_license_type(license_type_id_type license_id, optional<string> name,
                                 optional<share_type> amount,
                                 optional<upgrade_multiplier_type> balance_multipliers,
                                 optional<upgrade_multiplier_type> requeue_multipliers,
                                 optional<upgrade_multiplier_type> return_multipliers)
{
  modify(get(license_id), [&](license_type_object& lto) {
      if (name.valid()) lto.name = *name;
      if (amount.valid()) lto.amount = *amount;
      if (balance_multipliers.valid()) lto.balance_upgrade.reset(*balance_multipliers);
      if (requeue_multipliers.valid()) lto.requeue_upgrade.reset(*requeue_multipliers);
      if (return_multipliers.valid()) lto.return_upgrade.reset(*return_multipliers);
  });
}

void database::fulfill_license_request(const license_request_object& req)
{
  const auto& account_obj = req.account(*this);
  const auto& new_license_obj = req.license(*this);
  const auto& pending = *account_obj.license_info.pending;

  FC_ASSERT( pending.request == req.id );
  FC_ASSERT( pending.license == req.license );

  // We need to modify the account in order to change the license info.
  modify(account_obj, [&](account_object& a) {
    auto& info = a.license_info;

    // Add the license to the top of the history, so it becomes the new active license:
    info.add_license(new_license_obj.id, req.frequency);

    // Improve all the upgrades to match the new license:
    info.balance_upgrade += new_license_obj.balance_upgrade;
    info.requeue_upgrade += new_license_obj.requeue_upgrade;
    info.return_upgrade += new_license_obj.return_upgrade;

    // The license is no longer pending:
    info.clear_pending();
  });

  // For regular licenses, increase the cycle balance for the appropriate amount:
  if ( new_license_obj.kind == license_kind::regular )
    issue_cycles(account_obj.id, new_license_obj.amount);

  // For auto submit licenses, submit a new license request with frequency locked:
  else if ( new_license_obj.kind == license_kind::chartered || new_license_obj.kind == license_kind::promo )
  {
    // Create a new element in the reward queue:
    create<reward_queue_object>([&](reward_queue_object& rqo){
      rqo.account = req.account;
      rqo.amount = new_license_obj.amount;
      rqo.frequency = req.frequency;
      rqo.time = head_block_time();
    });

    // Submit a virtual operation for the submission of the license cycles:
    submit_cycles_operation vop;
    vop.account = req.account;
    vop.amount = new_license_obj.amount;

    push_applied_operation(vop);
  }

}

} }  // namespace graphhene::chain
