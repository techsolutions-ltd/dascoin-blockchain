/*
 * balance_checker.cpp
 */

#include <graphene/chain/balance_checker.hpp>
namespace graphene
{
  namespace chain
  {
    void_result balance_checker::check_remaining_balance(const database& db, const account_object& account,
                                                         const asset_object& asset, const graphene::chain::asset& amount_to_spend)
    {
      try
      {
        if (db.head_block_time() > HARDFORK_EXEX_127_TIME)
        {
          if (asset.id == db.get_dascoin_asset_id() && !account.is_custodian())
          {
            int minimum_dsc_no_precision = MINIMUM_DSC_BALANCE / DASCOIN_DEFAULT_ASSET_PRECISION;
            FC_ASSERT( db.get_balance(account, asset).amount - amount_to_spend.amount >= MINIMUM_DSC_BALANCE,
		       "insufficient balance, there must be ${a} dsc remaining", ("a", minimum_dsc_no_precision));
          }
        }

        return {};
      }
      FC_CAPTURE_AND_RETHROW()
    }
  }
}


