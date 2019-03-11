/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>

#include <graphene/chain/database.hpp>
#include <graphene/chain/license_evaluator.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/market_object.hpp>

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

void database::remove_limit_from_all_vaults()
{
   const auto& accounts_by_id = get_index_type<account_index>().indices().get<by_id>();
   for(auto itr = accounts_by_id.begin(); itr != accounts_by_id.end(); itr++)
   {
      if(itr->kind == account_kind::vault)
      {
         modify(*itr,[](account_object& account_obj){
            account_obj.disable_vault_to_wallet_limit = true;
         });
      }
   }

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

optional<price> database::get_price_in_web_eur(const asset_id_type asset_id) const
{
    if (asset_id == get_dascoin_asset_id())
    {
        return get_dynamic_global_properties().last_dascoin_price;
    }

    if (asset_id == get_web_asset_id())
    {
        return price{asset{1, get_web_asset_id()}, asset{1, get_web_asset_id()}};
    }

    const auto& use_market_price_for_token = get_global_properties().use_market_price_for_token;
    if (std::find(use_market_price_for_token.begin(), use_market_price_for_token.end(), asset_id) != use_market_price_for_token.end())
    {
      const auto& market_idx = get_index_type<last_price_index>().indices().get<by_market_key>();
      auto market_itr = market_idx.find(market_key{asset_id, get_web_asset_id()});
      if (market_itr != market_idx.end())
      {
        return market_itr->last_price;
      }
    }
    else
    {
      const auto& external_idx = get_index_type<external_price_index>().indices().get<by_market_key>();
      auto external_itr = external_idx.find(market_key{asset_id, get_web_asset_id()});
      if (external_itr != external_idx.end())
      {
        return external_itr->external_price;
      }
    }

    return {};
}

} }  // namespace graphene::chain
