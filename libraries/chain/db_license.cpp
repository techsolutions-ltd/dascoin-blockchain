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

optional<share_type> database::get_dascoin_limit(account_id_type account_id, price dascoin_price) const
{
  const auto ADVOCATE_EUR_LIMIT = get(DASCOIN_NULL_LICENSE).eur_limit;
  const auto DASCOIN_ASSET_ID = get_dascoin_asset_id();
  const auto& account = get(account_id);

  const auto& get_limit_from_price = [DASCOIN_ASSET_ID](share_type eur_limit, price dascoin_price) -> share_type
  {
    auto res = asset{eur_limit, DASCOIN_ASSET_ID} * dascoin_price;
    return res.amount;
  };

  if ( !account.is_vault() )
    return {};

  auto result = get_limit_from_price(ADVOCATE_EUR_LIMIT, dascoin_price);

  if ( account.license_information.valid() )
  {
    const license_information_object& lic_info = get(*account.license_information);
    const license_type_object& max_license = get(lic_info.max_license);
    result = get_limit_from_price(max_license.eur_limit, dascoin_price);
  }

  return {result};
}

} }  // namespace graphhene::chain
