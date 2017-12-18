
#include <graphene/chain/virtaul_operation_helper.hpp>


namespace graphene { namespace chain {


struct is_virtual_operation_visitor
{
   is_virtual_operation_visitor( ) {}
   typedef bool result_type;

   bool operator()( const transfer_operation& op ) { return false; }
   bool operator()( const asset_claim_fees_operation& op ) { return false; }
   bool operator()( const limit_order_create_operation& op ) { return false; }
   bool operator()( const limit_order_cancel_operation& op ) { return false; }
   bool operator()( const call_order_update_operation& op ) { return false; }
   bool operator()( const fill_order_operation& op ) { return true; }
   bool operator()( const account_create_operation& op ) { return false; }
   bool operator()( const account_update_operation& op ) { return false; }
   bool operator()( const account_whitelist_operation& op ) { return false; }
   bool operator()( const account_upgrade_operation& op ) { return false; }
   bool operator()( const account_transfer_operation& op ) { return false; }
   bool operator()( const asset_create_operation& op ) { return false; }
   bool operator()( const asset_update_operation& op ) { return false; }
   bool operator()( const asset_update_bitasset_operation& op ) { return false; }
   bool operator()( const asset_update_feed_producers_operation& op ) { return false; }
   bool operator()( const asset_issue_operation& op ) { return false; }
   bool operator()( const asset_reserve_operation& op ) { return false; }
   bool operator()( const asset_fund_fee_pool_operation& op ) { return false; }
   bool operator()( const asset_settle_operation& op ) { return false; }
   bool operator()( const asset_global_settle_operation& op ) { return false; }
   bool operator()( const asset_publish_feed_operation& op ) { return false; }
   bool operator()( const witness_create_operation& op ) { return false; }
   bool operator()( const witness_update_operation& op ) { return false; }
   bool operator()( const proposal_create_operation& op ) { return false; }
   bool operator()( const proposal_update_operation& op ) { return false; }
   bool operator()( const proposal_delete_operation& op ) { return false; }
   bool operator()( const withdraw_permission_create_operation& op ) { return false; }
   bool operator()( const withdraw_permission_update_operation& op ) { return false; }
   bool operator()( const withdraw_permission_claim_operation& op ) { return false; }
   bool operator()( const withdraw_permission_delete_operation& op ) { return false; }
   bool operator()( const committee_member_create_operation& op ) { return false; }
   bool operator()( const committee_member_update_operation& op ) { return false; }
   bool operator()( const committee_member_update_global_parameters_operation& op ) { return false; }
   bool operator()( const board_update_chain_authority_operation& op ) { return false; }
   bool operator()( const create_license_type_operation& op ) { return false; }
   bool operator()( const edit_license_type_operation& op ) { return false; }
   bool operator()( const issue_license_operation& op ) { return false; }
   bool operator()( const vesting_balance_create_operation& op ) { return false; }
   bool operator()( const vesting_balance_withdraw_operation& op ) { return false; }
   bool operator()( const worker_create_operation& op ) { return false; }
   bool operator()( const custom_operation& op ) { return false; }
   bool operator()( const assert_operation& op ) { return false; }
   bool operator()( const balance_claim_operation& op ) { return false; }
   bool operator()( const override_transfer_operation& op ) { return false; }
   bool operator()( const transfer_to_blind_operation& op ) { return false; }
   bool operator()( const blind_transfer_operation& op ) { return false; }
   bool operator()( const transfer_from_blind_operation& op ) { return false; }
   bool operator()( const asset_settle_cancel_operation& op ) { return true; }
   bool operator()( const fba_distribute_operation& op ) { return true; }
   bool operator()( const tether_accounts_operation& op ) { return false; }
   bool operator()( const upgrade_account_cycles_operation& op ) { return true; }
   bool operator()( const change_public_keys_operation& op ) { return false; }
   bool operator()( const update_pi_limits_operation& op ) { return false; }
   bool operator()( const asset_create_issue_request_operation& op ) { return false; }
   bool operator()( const asset_distribute_completed_request_operation& op ) { return true; }
   bool operator()( const asset_deny_issue_request_operation& op ) { return false; }
   bool operator()( const wire_out_operation& op ) { return false; }
   bool operator()( const wire_out_complete_operation& op ) { return false; }
   bool operator()( const wire_out_reject_operation& op ) { return false; }
   bool operator()( const wire_out_result_operation& op ) { return false; }
   bool operator()( const transfer_vault_to_wallet_operation& op ) { return false; }
   bool operator()( const transfer_wallet_to_vault_operation& op ) { return false; }
   bool operator()( const submit_reserve_cycles_to_queue_operation& op ) { return false; }
   bool operator()( const submit_cycles_to_queue_operation& op ) { return false; }
   bool operator()( const submit_cycles_to_queue_by_license_operation& op ) { return false; }
   bool operator()( const record_submit_reserve_cycles_to_queue_operation& op ) { return true; }
   bool operator()( const record_submit_charter_license_cycles_operation& op ) { return true; }
   bool operator()( const record_distribute_dascoin_operation& op ) { return true; }
   bool operator()( const update_queue_parameters_operation& op ) { return false; }
   bool operator()( const update_global_frequency_operation& op ) { return false; }
   bool operator() ( const issue_free_cycles_operation& op ) { return false; }
   bool operator() ( const update_euro_limit_operation& op ) { return false; }
};


bool is_virtual_operation(const operation& op)
{
   is_virtual_operation_visitor vivo = is_virtual_operation_visitor( );
   return op.visit( vivo );
}

bool is_virtual_operation(const unsigned operation_num)
{
   unsigned limit = 61;

   if(operation_num > limit)
   {
      return true;
   }

   return false;
}

} }
