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
                                   upgrade_multiplier_type return_multipliers,
                                   share_type eur_limit)
{
  return create<license_type_object>([&](license_type_object& lto){
    lto.name = name;
    lto.amount = amount;
    lto.kind = kind;
    lto.balance_upgrade.reset(balance_multipliers);
    lto.requeue_upgrade.reset(requeue_multipliers);
    lto.return_upgrade.reset(return_multipliers);
    lto.eur_limit = eur_limit;
  }).id;
}

// TODO: create generic lookup method.
optional<license_information_object> database::get_license_information(account_id_type account_id) const
{
   auto& index = get_index_type<license_information_index>().indices().get<by_account_id>();
   auto itr = index.find(account_id);
   if ( itr != index.end() ) return *itr;
   return {};
}

} }  // namespace graphhene::chain
