/*
 * balance_checker.hpp
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/database.hpp>


namespace graphene
{
  namespace chain
  {
    struct balance_checker
    {
      /**
       * Minimum dascoin balance that must remain on each wallet
       */
      static const int MINIMUM_DSC_BALANCE = 1 * DASCOIN_DEFAULT_ASSET_PRECISION;

      /**
       * @brief Check if account remains minimum leftover balance.
       *
       * Checks if DSC balance remaining is at least MINIMUM_DSC_BALANCE defined above
       */
      static void_result check_remaining_balance(const database& db, const account_object& account,
                                                 const asset_object& asset, const graphene::chain::asset& amount_to_spend);
    };
  }
}

