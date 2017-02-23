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

} }  // namespace graphhene::chain
