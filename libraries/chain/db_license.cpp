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
                                   share_type eur_limit,
                                   license_type_object::upgrade_policy upgrade_policy)
{
  return create<license_type_object>([&](license_type_object& lto){
    lto.name = name;
    lto.amount = amount;
    lto.kind = kind;
    lto.balance_upgrade.reset(balance_multipliers);
    lto.requeue_upgrade.reset(requeue_multipliers);
    lto.return_upgrade.reset(return_multipliers);
    lto.eur_limit = eur_limit;
    lto.up_policy = upgrade_policy;
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

optional<share_type> database::get_dascoin_limit(const account_object& account, price dascoin_price) const
{
  const auto advocate_eur_limit = DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE;
  const auto webeur_asset_it = get_web_asset_id();

  const auto& get_limit_from_price = [webeur_asset_it](share_type eur_limit, price dascoin_price) -> share_type
  {
    auto res = asset{eur_limit, webeur_asset_it} * dascoin_price;
    return res.amount;
  };

  if ( !account.is_vault() )
    return {};

  auto result = get_limit_from_price(advocate_eur_limit, dascoin_price);

  if ( account.license_information.valid() )
  {
    const license_information_object& lic_info = get(*account.license_information);
    const license_type_object& max_license = get(lic_info.max_license);
    result = get_limit_from_price(max_license.eur_limit, dascoin_price);
  }

  return {result};
}

share_type database::get_eur_limit(const optional<license_information_object> &license_info) const
{
  if (!license_info.valid())
    return DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE;
  const auto& license_type = get(license_info->max_license);
  return license_type.eur_limit;
}

} }  // namespace graphhene::chain
