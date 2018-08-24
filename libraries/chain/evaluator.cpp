/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/fba_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/market_evaluator.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/uint128.hpp>

namespace graphene { namespace chain {
database& generic_evaluator::db()const { return trx_state->db(); }

   operation_result generic_evaluator::start_evaluate( transaction_evaluation_state& eval_state, const operation& op, bool apply )
   { try {
      trx_state   = &eval_state;
      //check_required_authorities(op);
      auto result = evaluate( op );

      if( apply ) result = this->apply( op );
      return result;
   } FC_CAPTURE_AND_RETHROW() }

   void generic_evaluator::prepare_fee(account_id_type account_id, asset fee, operation op)
   {
      const database& d = db();
      const auto& gbo = d.get_global_properties();
      const auto current_fee_asset = gbo.parameters.current_fees->fee_asset_id(d);

      FC_ASSERT( fee.amount >= 0 );
      fee_paying_account = &account_id(d);

      fee_asset = &fee.asset_id(d);
      fee_asset_dyn_data = &fee_asset->dynamic_asset_data_id(d);

      fee_paid = calculate_fee_for_operation(op);

      if(fee_paid > 0)
      {
          FC_ASSERT( fee.amount == fee_paid, "Attempted to pay wrong amount of fee. Expected ${expected} payed ${payed}.",
                     ("expected", fee_paid)("payed", fee.amount) );
          FC_ASSERT( fee.asset_id == gbo.parameters.current_fees->fee_asset_id, "Attempted to pay fee by using asset ${a} '${sym}', which is unauthorized. Fee must be payed in ${f}.",
                     ("a", fee.asset_id)("sym", fee_asset->symbol)("f", current_fee_asset.symbol) );
      }

      // if asset is core just leave this part
      if(fee.asset_id == asset_id_type())
      {
         fee_paid = 0;
         return;
      }

      account_fee_balance_object = &(d.get_balance_object(account_id, fee.asset_id));

      auto balance = account_fee_balance_object->get_balance();
      FC_ASSERT( fee_paid <= balance.amount, "Low balance. Not enough to pay fee.");
   }

   void generic_evaluator::pay_fee()
   { try {
      if( !trx_state->skip_fee && fee_paid > 0) {
         database& d = db();

         d.modify(*account_fee_balance_object, [&](account_balance_object& b)
         {
            b.balance -= fee_paid;
         });

         /// put fee in fee pool or burn it if pool is not set
         const auto& dynamic_properties = d.get_dynamic_global_properties();
         if(dynamic_properties.fee_pool_account_id != account_id_type())
         {
            d.modify(d.get_balance_object(dynamic_properties.fee_pool_account_id, account_fee_balance_object->asset_type), [&](account_balance_object& b)
            {
               b.balance += fee_paid;
            });
         }
         else // this means that we have to burn fee asset
         {
            const auto& fee_asset = account_fee_balance_object->asset_type(d);
            d.modify(fee_asset.dynamic_asset_data_id(d), [&](asset_dynamic_data_object& addo)
            {
               addo.current_supply -= fee_paid;
            });
         }
      }
   } FC_CAPTURE_AND_RETHROW() }

   // this function is commented out in accordance with new implementation of fees
   // in cycles, and we do not use blind transfer operation so we do not need this implementation reimplemented
   // Jira task related to this is EXEX-70
   void generic_evaluator::pay_fba_fee( uint64_t fba_id )
   {
//      database& d = db();
//      const fba_accumulator_object& fba = d.get< fba_accumulator_object >( fba_accumulator_id_type( fba_id ) );
//      if( !fba.is_configured(d) )
//      {
//         generic_evaluator::pay_fee();
//         return;
//      }
//      d.modify( fba, [&]( fba_accumulator_object& _fba )
//      {
//         _fba.accumulated_fba_fees += core_fee_paid;
//      } );
   }

   share_type generic_evaluator::calculate_fee_for_operation(const operation& op) const
   {
     return db().current_fee_schedule().calculate_fee( op ).amount;
   }
   void generic_evaluator::db_adjust_balance(const account_id_type& fee_payer, asset fee_from_account)
   {
     db().adjust_balance(fee_payer, fee_from_account);
   }

} }
