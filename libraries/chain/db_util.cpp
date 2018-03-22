/**
 * DASCOIN!
*/

#include <cmath>

#include <graphene/chain/database.hpp>
#include <graphene/chain/license_evaluator.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

share_type database::cycles_to_dascoin(share_type cycles, share_type frequency) const
{
  FC_ASSERT( frequency != 0 );
  return (cycles * DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION) / frequency;
}

share_type database::dascoin_to_cycles(share_type dascoin, share_type frequency) const
{
  return dascoin * frequency / (DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION);
}

void database::perform_chain_authority_check(const string& auth_type_name, account_id_type auth_id,
                                             const account_object& acc_obj) const
{
  FC_ASSERT( acc_obj.id == auth_id,
             "Operation must be signed by ${auth_type} authority '${auth_name}', signed by '${a}' instead'",
             ("auth_type", auth_type_name)
             ("auth_name", auth_id(*this).name)
             ("a", acc_obj.name)
           );
}

void database::perform_root_authority_check(const account_id_type& authority_account_id)
{
   FC_ASSERT(get_dynamic_global_properties().is_root_authority_enabled_flag, "Your authority is deprecated!");

   const auto root_administrator_id = get_global_properties().authorities.root_administrator;
   const auto& op_authority_account_obj = authority_account_id(*this);

   perform_chain_authority_check("root authority", root_administrator_id, op_authority_account_obj);
}

share_type database::get_licence_max_reward_in_dascoin(const license_type_object& lto, share_type bonus_percentage, share_type frequency) const
{
  share_type initial_licence_cycles = detail::apply_percentage(lto.amount, bonus_percentage);
  share_type max_licence_cycles = initial_licence_cycles * std::pow(2, lto.balance_upgrade.multipliers.size());

  return cycles_to_dascoin(max_licence_cycles, frequency);
}

share_type database::get_total_dascoin_amount_in_system() const
{
  const auto& queue = get_index_type<reward_queue_index>().indices().get<by_time>();

  if (queue.size() == 0)
    return get_dynamic_global_properties().total_dascoin_minted;

  return queue.rbegin()->historic_sum;
}

} }  // namespace graphene::chain
