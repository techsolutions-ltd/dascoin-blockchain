#include <fc/container/flat.hpp>

#include <graphene/chain/protocol/authority.hpp>
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/protocol/transaction.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/chain/confidential_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/issued_asset_record_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/impacted.hpp>

using namespace fc;
using namespace graphene::chain;

// TODO:  Review all of these, especially no-ops
struct get_impacted_account_visitor
{
   flat_set<account_id_type>& _impacted;
   get_impacted_account_visitor( flat_set<account_id_type>& impact ):_impacted(impact) {}
   typedef void result_type;

   void operator()( const transfer_operation& op )
   {
      _impacted.insert( op.to );
   }

   void operator()( const asset_claim_fees_operation& op ) {}
   void operator()( const limit_order_create_operation& op ) {}
   void operator()( const limit_order_cancel_operation& op )
   {
      _impacted.insert( op.fee_paying_account );
   }
   void operator()( const call_order_update_operation& op ) {}
   void operator()( const fill_order_operation& op )
   {
      _impacted.insert( op.account_id );
   }

   void operator()( const account_create_operation& op )
   {
      _impacted.insert( op.registrar );
      _impacted.insert( op.referrer );
      add_authority_accounts( _impacted, op.owner );
      add_authority_accounts( _impacted, op.active );
   }

   void operator()( const account_update_operation& op )
   {
      _impacted.insert( op.account );
      if( op.owner )
         add_authority_accounts( _impacted, *(op.owner) );
      if( op.active )
         add_authority_accounts( _impacted, *(op.active) );
   }

   void operator()( const account_whitelist_operation& op )
   {
      _impacted.insert( op.account_to_list );
   }

   void operator()( const account_upgrade_operation& op ) {}
   void operator()( const account_transfer_operation& op )
   {
      _impacted.insert( op.new_owner );
   }

   void operator()( const asset_create_operation& op ) {}
   void operator()( const asset_update_operation& op )
   {
      if( op.new_issuer )
         _impacted.insert( *(op.new_issuer) );
   }

   void operator()( const asset_update_bitasset_operation& op ) {}
   void operator()( const asset_update_feed_producers_operation& op ) {}

   void operator()( const asset_issue_operation& op )
   {
      _impacted.insert( op.issue_to_account );
   }

   void operator()( const asset_reserve_operation& op ) {}
   void operator()( const asset_fund_fee_pool_operation& op ) {}
   void operator()( const asset_settle_operation& op ) {}
   void operator()( const asset_global_settle_operation& op ) {}
   void operator()( const asset_publish_feed_operation& op ) {}
   void operator()( const remove_root_authority_operation& op ) {}
   void operator()( const create_witness_operation& op ) {}
   void operator()( const update_witness_operation& op ) {}
   void operator()( const remove_witness_operation& op ) {}
   void operator()( const activate_witness_operation& op ) {}
   void operator()( const deactivate_witness_operation& op ) {}
   void operator()( const witness_create_operation& op )
   {
      _impacted.insert( op.witness_account );
   }
   void operator()( const witness_update_operation& op )
   {
      _impacted.insert( op.witness_account );
   }

   void operator()( const proposal_create_operation& op )
   {
      vector<authority> other;
      for( const auto& proposed_op : op.proposed_ops )
         operation_get_required_authorities( proposed_op.op, _impacted, _impacted, other );
      for( auto& o : other )
         add_authority_accounts( _impacted, o );
   }

   void operator()( const proposal_update_operation& op ) {}
   void operator()( const proposal_delete_operation& op ) {}

   void operator()( const withdraw_permission_create_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_update_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_claim_operation& op )
   {
      _impacted.insert( op.withdraw_from_account );
   }

   void operator()( const withdraw_permission_delete_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const committee_member_create_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_global_parameters_operation& op ) {}
   
   void operator()( const board_update_chain_authority_operation& op ) 
   {
      _impacted.insert( op.account );
      _impacted.insert( op.committee_member_account );
   }

   void operator()( const create_license_type_operation& op ) {}

   void operator()( const edit_license_type_operation& op ) {}

   void operator()( const issue_license_operation& op )
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.account );
   }

   void operator()( const vesting_balance_create_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const vesting_balance_withdraw_operation& op ) {}
   void operator()( const worker_create_operation& op ) {}
   void operator()( const custom_operation& op ) {}
   void operator()( const assert_operation& op ) {}
   void operator()( const balance_claim_operation& op ) {}

   void operator()( const remove_vault_limit_operation& op )
   {
      _impacted.insert( op.authority );
   }

   void operator()( const change_operation_fee_operation& op )
   {
      _impacted.insert( op.issuer );
   }

   void operator()( const change_fee_pool_account_operation& op )
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.fee_pool_account_id );
   }

   void operator()( const override_transfer_operation& op )
   {
      _impacted.insert( op.to );
      _impacted.insert( op.from );
      _impacted.insert( op.issuer );
   }

   void operator()( const transfer_to_blind_operation& op )
   {
      _impacted.insert( op.from );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const blind_transfer_operation& op )
   {
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const transfer_from_blind_operation& op )
   {
      _impacted.insert( op.to );
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
   }

   void operator()( const asset_settle_cancel_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const fba_distribute_operation& op )
   {
      _impacted.insert( op.account_id );
   }

   void operator()( const tether_accounts_operation& op )
   {
      _impacted.insert( op.wallet_account );
      _impacted.insert( op.vault_account );
   }

   void operator()( const upgrade_account_cycles_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const change_public_keys_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const set_roll_back_enabled_operation& op )
   {
     _impacted.insert( op.account );
   }

   void operator()( const roll_back_public_keys_operation& op )
   {
     _impacted.insert( op.authority );
     _impacted.insert( op.account );
   }

  void operator()( const asset_create_issue_request_operation& op )
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.receiver );
   }

   void operator()( const asset_distribute_completed_request_operation& op )
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.receiver );
   }

   void operator()( const asset_deny_issue_request_operation& op ) {}

   void operator()( const wire_out_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const wire_out_complete_operation& op )
   {
      _impacted.insert( op.wire_out_handler );
   }

   void operator()( const wire_out_reject_operation& op )
   {
      _impacted.insert( op.wire_out_handler );
   }

   void operator()( const wire_out_result_operation& op )
   {
      _impacted.insert( op.account );
   }

  void operator()( const wire_out_with_fee_operation& op )
  {
    _impacted.insert( op.account );
  }

  void operator()( const wire_out_with_fee_complete_operation& op )
  {
    _impacted.insert( op.wire_out_handler );
  }

  void operator()( const wire_out_with_fee_reject_operation& op )
  {
    _impacted.insert( op.wire_out_handler );
  }

  void operator()( const wire_out_with_fee_result_operation& op )
  {
    _impacted.insert( op.account );
  }

   void operator()( const transfer_vault_to_wallet_operation& op )
   {
      _impacted.insert( op.from_vault );
      _impacted.insert( op.to_wallet );
   }

   void operator()( const transfer_wallet_to_vault_operation& op )
   {
      _impacted.insert( op.from_wallet );
      _impacted.insert( op.to_vault );
   }

   void operator()( const submit_reserve_cycles_to_queue_operation& op ) 
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.account );
   }

   void operator()( const submit_cycles_to_queue_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const fee_pool_cycles_submit_operation& op )
   {
      _impacted.insert( op.issuer );
   }

   void operator()( const submit_cycles_to_queue_by_license_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const record_submit_reserve_cycles_to_queue_operation& op )
   {
      _impacted.insert( op.cycle_issuer );
      _impacted.insert( op.account );
   }

   void operator()( const record_submit_charter_license_cycles_operation& op )
   {
      _impacted.insert( op.license_issuer );
      _impacted.insert( op.account );
   }

   void operator()( const record_distribute_dascoin_operation& op )
   {
      _impacted.insert( op.account );
   }
   
   void operator()( const update_queue_parameters_operation& op ) {}

   void operator()( const update_global_frequency_operation& op )
   {
      _impacted.insert( op.authority );
   }

   void operator()( const set_daspay_transaction_ratio_operation& op )
   {
      _impacted.insert( op.authority );
   }

   void operator()( const create_payment_service_provider_operation& op )
   {
     _impacted.insert( op.authority );
     _impacted.insert( op.payment_service_provider_account );
     for (const auto& acc : op.payment_service_provider_clearing_accounts)
       _impacted.insert( acc );
   }

   void operator()( const update_payment_service_provider_operation& op )
   {
     _impacted.insert( op.authority );
     _impacted.insert( op.payment_service_provider_account );
     for (const auto& acc : op.payment_service_provider_clearing_accounts)
       _impacted.insert( acc );
   }

   void operator()( const delete_payment_service_provider_operation& op )
   {
     _impacted.insert( op.authority );
     _impacted.insert( op.payment_service_provider_account );
   }

   void operator()( const update_daspay_clearing_parameters_operation& op )
   {
     _impacted.insert( op.authority );
   }

   void operator()( const update_delayed_operations_resolver_parameters_operation& op )
   {
     _impacted.insert( op.authority );
   }

   void operator() ( const issue_free_cycles_operation& op )
   {
      _impacted.insert( op.authority );
      _impacted.insert( op.account );
   }

   void operator() ( const update_euro_limit_operation& op )
   {
      _impacted.insert( op.authority );
      _impacted.insert( op.account );
   }

   void operator() ( const create_upgrade_event_operation& op )
   {
      _impacted.insert( op.upgrade_creator );
   }

   void operator() ( const update_upgrade_event_operation& op )
   {
     _impacted.insert( op.upgrade_creator );
   }

   void operator() ( const delete_upgrade_event_operation& op )
   {
     _impacted.insert( op.upgrade_creator );
   }

   void operator() ( const update_license_operation& op )
   {
      _impacted.insert( op.authority );
   }

   void operator() ( const issue_cycles_to_license_operation& op )
   {
      _impacted.insert( op.authority );
      _impacted.insert( op.account );
   }

   void operator() ( const purchase_cycle_asset_operation& op )
   {
      _impacted.insert( op.wallet_id );
   }

   void operator() ( const transfer_cycles_from_licence_to_wallet_operation& op )
   {
      _impacted.insert( op.vault_id );
      _impacted.insert( op.wallet_id );
   }

   void operator() ( const set_starting_cycle_asset_amount_operation& op )
   {
      _impacted.insert(op.issuer);
   }

   void operator() ( const register_daspay_authority_operation& op )
   {
      _impacted.insert(op.issuer);
   }

   void operator() ( const unregister_daspay_authority_operation& op )
   {
      _impacted.insert(op.issuer);
   }

   void operator() ( const reserve_asset_on_account_operation& op )
   {
      _impacted.insert(op.account);
   }

   void operator() ( const unreserve_asset_on_account_operation& op )
   {
      _impacted.insert(op.account);
   }

   void operator() ( const unreserve_completed_operation& op )
   {
      _impacted.insert(op.account);
   }

   void operator() ( const daspay_debit_account_operation& op )
   {
      _impacted.insert(op.payment_service_provider_account);
      _impacted.insert(op.account);
      _impacted.insert(op.clearing_account);
   }

   void operator() ( const daspay_credit_account_operation& op )
   {
      _impacted.insert(op.payment_service_provider_account);
      _impacted.insert(op.account);
      _impacted.insert(op.clearing_account);
   }

   void operator() ( const set_chain_authority_operation& op )
   {
     _impacted.insert(op.issuer);
     _impacted.insert(op.account);
   }

   void operator() ( const das33_pledge_asset_operation& op )
   {
     _impacted.insert(op.account_id);
   }

   void operator() ( const das33_project_create_operation& op )
   {
     _impacted.insert(op.authority);
     _impacted.insert(op.owner);
   }

   void operator() ( const das33_project_update_operation& op )
   {
     _impacted.insert(op.authority);
   }

   void operator() ( const das33_project_delete_operation& op )
   {
     _impacted.insert(op.authority);
   }

   void operator() ( const update_global_parameters_operation& op )
   {
     _impacted.insert(op.authority);
   }
};

void graphene::chain::operation_get_impacted_accounts( const operation& op, flat_set<account_id_type>& result )
{
   get_impacted_account_visitor vtor = get_impacted_account_visitor( result );
   op.visit( vtor );
}

void graphene::chain::transaction_get_impacted_accounts( const transaction& tx, flat_set<account_id_type>& result )
{
   for( const auto& op : tx.operations )
      operation_get_impacted_accounts( op, result );
}

// TODO: fill this for ALL object types.
// TODO: figure out how to properly fill this out for each object type.
void get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )
{
      if( obj->id.space() == protocol_ids )
      {
         switch( (object_type)obj->id.type() )
         {
            case null_object_type:
            case base_object_type:
            case OBJECT_TYPE_COUNT:
               return;
            case account_object_type:{
               accounts.insert( obj->id );
               break;
            } case asset_object_type:{
               const auto& aobj = dynamic_cast<const asset_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->issuer );
               break;
            } case force_settlement_object_type:{
               const auto& aobj = dynamic_cast<const force_settlement_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->owner );
               break;
            } case committee_member_object_type:{
               const auto& aobj = dynamic_cast<const committee_member_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->committee_member_account );
               break;
            } case witness_object_type:{
               const auto& aobj = dynamic_cast<const witness_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->witness_account );
               break;
            } case limit_order_object_type:{
               const auto& aobj = dynamic_cast<const limit_order_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->seller );
               break;
            } case call_order_object_type:{
               const auto& aobj = dynamic_cast<const call_order_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->borrower );
               break;
            } case custom_object_type:{
              break;
            } case proposal_object_type:{
               const auto& aobj = dynamic_cast<const proposal_object*>(obj);
               assert( aobj != nullptr );
               flat_set<account_id_type> impacted;
               transaction_get_impacted_accounts( aobj->proposed_transaction, accounts );
               break;
            } case operation_history_object_type:{
               const auto& aobj = dynamic_cast<const operation_history_object*>(obj);
               assert( aobj != nullptr );
               flat_set<account_id_type> impacted;
               operation_get_impacted_accounts( aobj->op, accounts );
               break;
            } case withdraw_permission_object_type:{
               const auto& aobj = dynamic_cast<const withdraw_permission_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->withdraw_from_account );
               accounts.insert( aobj->authorized_account );
               break;
            } case vesting_balance_object_type:{
               const auto& aobj = dynamic_cast<const vesting_balance_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->owner );
               break;
            } case worker_object_type:{
               const auto& aobj = dynamic_cast<const worker_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->worker_account );
               break;
            } case balance_object_type:
              case license_type_object_type:{
               /** these are free from any accounts */
               break;
            } case upgrade_event_object_type:{
               break;
            }
         }
      }
      else if( obj->id.space() == implementation_ids )
      {
         switch( (impl_object_type)obj->id.type() )
         {
            case impl_global_property_object_type:
               break;
            case impl_dynamic_global_property_object_type:
               break;
            case impl_reserved0_object_type:
               break;
            case impl_asset_dynamic_data_type:
               break;
            case impl_asset_bitasset_data_type:
               break;
            case impl_account_balance_object_type:{
               const auto& aobj = dynamic_cast<const account_balance_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->owner );
               break;
               }
            case impl_account_cycle_balance_object_type:{
               const auto& acobj = dynamic_cast<const account_cycle_balance_object*>(obj);
               assert( acobj != nullptr );
               accounts.insert(acobj->owner);
               break;
            } case impl_account_statistics_object_type:{
               const auto& aobj = dynamic_cast<const account_statistics_object*>(obj);
               assert( aobj != nullptr );
               accounts.insert( aobj->owner );
               break;
            } case impl_transaction_object_type:{
               const auto& aobj = dynamic_cast<const transaction_object*>(obj);
               assert( aobj != nullptr );
               flat_set<account_id_type> impacted;
               transaction_get_impacted_accounts( aobj->trx, impacted );
               break;
            } case impl_blinded_balance_object_type:{
               const auto& aobj = dynamic_cast<const blinded_balance_object*>(obj);
               assert( aobj != nullptr );
               for( const auto& a : aobj->owner.account_auths )
                  accounts.insert( a.first );
               break;
            } case impl_license_information_object_type:{
               const auto& lio = dynamic_cast<const license_information_object*>(obj);
               assert( nullptr != lio );
               accounts.insert( lio->account );
               break;
            } case impl_issued_asset_record_object_type:{
               const auto& iaro = dynamic_cast<const issued_asset_record_object*>(obj);
               assert( nullptr != iaro );
               accounts.insert( iaro->issuer );
               accounts.insert( iaro->receiver );
               break;
            } case impl_block_summary_object_type:
               break;
            case impl_account_transaction_history_object_type:
               break;
            case impl_chain_property_object_type:
               break;
            case impl_witness_schedule_object_type:
               break;
            case impl_budget_record_object_type:
               break;
            case impl_special_authority_object_type:
               break;
            case impl_buyback_object_type:
               break;
            case impl_fba_accumulator_object_type:
               break;              
            case impl_issue_asset_request_object_type:
               break;
            case impl_wire_out_holder_object_type:
               break;
            case impl_wire_out_with_fee_holder_object_type:
               break;
            case impl_reward_queue_object_type:
               break;
            case impl_frequency_history_record_object_type:
               break;
            case impl_witness_delegate_data_colection_object_type:
               break;
            case impl_payment_service_provider_object_type:
               break;
            case impl_daspay_authority_object_type:
              break;
            case impl_das33_project_object_type:
              break;
            case impl_das33_pledge_holder_object_type:
	      break;
            case impl_delayed_operation_object_type:
              break;
      }
   }
} // end get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )

namespace graphene { namespace chain {

void database::notify_changed_objects()
{ try {
   if ( _undo_db.enabled() )
   {
      const auto& head_undo = _undo_db.head();
      
      // New:
      if( !new_objects.empty() )
      {
         vector<object_id_type> new_ids;  new_ids.reserve(head_undo.new_ids.size());
         flat_set<account_id_type> new_accounts_impacted;
         for( const auto& item : head_undo.new_ids )
         {
            new_ids.push_back(item);
            auto obj = find_object(item);
            if(obj != nullptr)
               get_relevant_accounts(obj, new_accounts_impacted);
         }

         new_objects(new_ids, new_accounts_impacted);
      }

      // Changed:
      if( !changed_objects.empty() )
      {
         vector<object_id_type> changed_ids;  changed_ids.reserve(head_undo.old_values.size());
         flat_set<account_id_type> changed_accounts_impacted;
         for( const auto& item : head_undo.old_values )
         {
            changed_ids.push_back(item.first);
            get_relevant_accounts(item.second.get(), changed_accounts_impacted);
         }

         changed_objects(changed_ids, changed_accounts_impacted);
      }

      // Removed:
      if( !removed_objects.empty() )
      {
         vector<object_id_type> removed_ids; removed_ids.reserve( head_undo.removed.size() );
         vector<const object*> removed; removed.reserve( head_undo.removed.size() );
         flat_set<account_id_type> removed_accounts_impacted;
         for( const auto& item : head_undo.removed )
         {
            removed_ids.emplace_back( item.first );
            auto obj = item.second.get();
            removed.emplace_back( obj );
            get_relevant_accounts(obj, removed_accounts_impacted);
         }

         removed_objects(removed_ids, removed, removed_accounts_impacted);
      }
   }
} FC_CAPTURE_AND_LOG( (0) ) }

} }
