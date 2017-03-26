ChainTypes.operations=
    transfer: 0
    limit_order_create: 1
    limit_order_cancel: 2
    call_order_update: 3
    account_create: 4
    account_update: 5
    account_whitelist: 6
    account_upgrade: 7
    account_transfer: 8
    asset_create: 9
    asset_update: 10
    asset_update_bitasset: 11
    asset_update_feed_producers: 12
    asset_issue: 13
    asset_reserve: 14
    asset_fund_fee_pool: 15
    asset_settle: 16
    asset_global_settle: 17
    asset_publish_feed: 18
    witness_create: 19
    witness_update: 20
    proposal_create: 21
    proposal_update: 22
    proposal_delete: 23
    withdraw_permission_create: 24
    withdraw_permission_update: 25
    withdraw_permission_claim: 26
    withdraw_permission_delete: 27
    committee_member_create: 28
    committee_member_update: 29
    committee_member_update_global_parameters: 30
    vesting_balance_create: 31
    vesting_balance_withdraw: 32
    worker_create: 33
    custom: 34
    assert: 35
    balance_claim: 36
    override_transfer: 37
    transfer_to_blind: 38
    blind_transfer: 39
    transfer_from_blind: 40
    asset_claim_fees: 41
    board_update_chain_authority: 42
    update_queue_parameters: 43
    create_license: 44
    issue_license: 45
    tether_accounts: 46
    asset_create_issue_request: 47
    asset_deny_issue_request: 48
    wire_out: 49
    wire_out_complete: 50
    wire_out_reject: 51
    transfer_vault_to_wallet: 52
    transfer_wallet_to_vault: 53
    submit_reserve_cycles_to_queue: 54
    submit_cycles_to_queue: 55
    record_submit_reserve_cycles_to_queue: 56
    record_submit_charter_license_cycles: 57
    record_distribute_dascoin: 58
    asset_distribute_completed_request: 59
    upgrade_account_cycles: 60
    fba_distribute: 61
    asset_settle_cancel: 62
    fill_order: 63

transfer_operation_fee_parameters = new Serializer( 
    "transfer_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

limit_order_create_operation_fee_parameters = new Serializer( 
    "limit_order_create_operation_fee_parameters"
    fee: uint64
)

limit_order_cancel_operation_fee_parameters = new Serializer( 
    "limit_order_cancel_operation_fee_parameters"
    fee: uint64
)

call_order_update_operation_fee_parameters = new Serializer( 
    "call_order_update_operation_fee_parameters"
    fee: uint64
)

account_create_operation_fee_parameters = new Serializer( 
    "account_create_operation_fee_parameters"
)

account_update_operation_fee_parameters = new Serializer( 
    "account_update_operation_fee_parameters"
    fee: int64
    price_per_kbyte: uint32
)

account_whitelist_operation_fee_parameters = new Serializer( 
    "account_whitelist_operation_fee_parameters"
    fee: int64
)

account_upgrade_operation_fee_parameters = new Serializer( 
    "account_upgrade_operation_fee_parameters"
    membership_annual_fee: uint64
    membership_lifetime_fee: uint64
)

account_transfer_operation_fee_parameters = new Serializer( 
    "account_transfer_operation_fee_parameters"
    fee: uint64
)

asset_create_operation_fee_parameters = new Serializer( 
    "asset_create_operation_fee_parameters"
)

asset_update_operation_fee_parameters = new Serializer( 
    "asset_update_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

asset_update_bitasset_operation_fee_parameters = new Serializer( 
    "asset_update_bitasset_operation_fee_parameters"
    fee: uint64
)

asset_update_feed_producers_operation_fee_parameters = new Serializer( 
    "asset_update_feed_producers_operation_fee_parameters"
    fee: uint64
)

asset_issue_operation_fee_parameters = new Serializer( 
    "asset_issue_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

asset_reserve_operation_fee_parameters = new Serializer( 
    "asset_reserve_operation_fee_parameters"
    fee: uint64
)

asset_fund_fee_pool_operation_fee_parameters = new Serializer( 
    "asset_fund_fee_pool_operation_fee_parameters"
    fee: uint64
)

asset_settle_operation_fee_parameters = new Serializer( 
    "asset_settle_operation_fee_parameters"
    fee: uint64
)

asset_global_settle_operation_fee_parameters = new Serializer( 
    "asset_global_settle_operation_fee_parameters"
    fee: uint64
)

asset_publish_feed_operation_fee_parameters = new Serializer( 
    "asset_publish_feed_operation_fee_parameters"
    fee: uint64
)

witness_create_operation_fee_parameters = new Serializer( 
    "witness_create_operation_fee_parameters"
    fee: uint64
)

witness_update_operation_fee_parameters = new Serializer( 
    "witness_update_operation_fee_parameters"
    fee: int64
)

proposal_create_operation_fee_parameters = new Serializer( 
    "proposal_create_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

proposal_update_operation_fee_parameters = new Serializer( 
    "proposal_update_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

proposal_delete_operation_fee_parameters = new Serializer( 
    "proposal_delete_operation_fee_parameters"
    fee: uint64
)

withdraw_permission_create_operation_fee_parameters = new Serializer( 
    "withdraw_permission_create_operation_fee_parameters"
    fee: uint64
)

withdraw_permission_update_operation_fee_parameters = new Serializer( 
    "withdraw_permission_update_operation_fee_parameters"
    fee: uint64
)

withdraw_permission_claim_operation_fee_parameters = new Serializer( 
    "withdraw_permission_claim_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

withdraw_permission_delete_operation_fee_parameters = new Serializer( 
    "withdraw_permission_delete_operation_fee_parameters"
    fee: uint64
)

committee_member_create_operation_fee_parameters = new Serializer( 
    "committee_member_create_operation_fee_parameters"
    fee: uint64
)

committee_member_update_operation_fee_parameters = new Serializer( 
    "committee_member_update_operation_fee_parameters"
    fee: uint64
)

committee_member_update_global_parameters_operation_fee_parameters = new Serializer( 
    "committee_member_update_global_parameters_operation_fee_parameters"
    fee: uint64
)

vesting_balance_create_operation_fee_parameters = new Serializer( 
    "vesting_balance_create_operation_fee_parameters"
    fee: uint64
)

vesting_balance_withdraw_operation_fee_parameters = new Serializer( 
    "vesting_balance_withdraw_operation_fee_parameters"
    fee: uint64
)

worker_create_operation_fee_parameters = new Serializer( 
    "worker_create_operation_fee_parameters"
    fee: uint64
)

custom_operation_fee_parameters = new Serializer( 
    "custom_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

assert_operation_fee_parameters = new Serializer( 
    "assert_operation_fee_parameters"
    fee: uint64
)

balance_claim_operation_fee_parameters = new Serializer( 
    "balance_claim_operation_fee_parameters"
)

override_transfer_operation_fee_parameters = new Serializer( 
    "override_transfer_operation_fee_parameters"
    fee: uint64
    price_per_kbyte: uint32
)

transfer_to_blind_operation_fee_parameters = new Serializer( 
    "transfer_to_blind_operation_fee_parameters"
    fee: uint64
    price_per_output: uint32
)

blind_transfer_operation_fee_parameters = new Serializer( 
    "blind_transfer_operation_fee_parameters"
    fee: uint64
    price_per_output: uint32
)

transfer_from_blind_operation_fee_parameters = new Serializer( 
    "transfer_from_blind_operation_fee_parameters"
    fee: uint64
)

asset_claim_fees_operation_fee_parameters = new Serializer( 
    "asset_claim_fees_operation_fee_parameters"
    fee: uint64
)

board_update_chain_authority_operation_fee_parameters = new Serializer( 
    "board_update_chain_authority_operation_fee_parameters"
)

update_queue_parameters_operation_fee_parameters = new Serializer( 
    "update_queue_parameters_operation_fee_parameters"
)

create_license_type_operation_fee_parameters = new Serializer( 
    "create_license_type_operation_fee_parameters"
)

issue_license_operation_fee_parameters = new Serializer( 
    "issue_license_operation_fee_parameters"
)

tether_accounts_operation_fee_parameters = new Serializer( 
    "tether_accounts_operation_fee_parameters"
)

asset_create_issue_request_operation_fee_parameters = new Serializer( 
    "asset_create_issue_request_operation_fee_parameters"
)

asset_deny_issue_request_operation_fee_parameters = new Serializer( 
    "asset_deny_issue_request_operation_fee_parameters"
)

wire_out_operation_fee_parameters = new Serializer( 
    "wire_out_operation_fee_parameters"
)

wire_out_complete_operation_fee_parameters = new Serializer( 
    "wire_out_complete_operation_fee_parameters"
)

wire_out_reject_operation_fee_parameters = new Serializer( 
    "wire_out_reject_operation_fee_parameters"
)

transfer_vault_to_wallet_operation_fee_parameters = new Serializer( 
    "transfer_vault_to_wallet_operation_fee_parameters"
)

transfer_wallet_to_vault_operation_fee_parameters = new Serializer( 
    "transfer_wallet_to_vault_operation_fee_parameters"
)

submit_reserve_cycles_to_queue_operation_fee_parameters = new Serializer( 
    "submit_reserve_cycles_to_queue_operation_fee_parameters"
)

submit_cycles_to_queue_operation_fee_parameters = new Serializer( 
    "submit_cycles_to_queue_operation_fee_parameters"
)

record_submit_reserve_cycles_to_queue_operation_fee_parameters = new Serializer( 
    "record_submit_reserve_cycles_to_queue_operation_fee_parameters"
)

record_submit_charter_license_cycles_operation_fee_parameters = new Serializer( 
    "record_submit_charter_license_cycles_operation_fee_parameters"
)

record_distribute_dascoin_operation_fee_parameters = new Serializer( 
    "record_distribute_dascoin_operation_fee_parameters"
)

asset_distribute_completed_request_operation_fee_parameters = new Serializer( 
    "asset_distribute_completed_request_operation_fee_parameters"
)

upgrade_account_cycles_operation_fee_parameters = new Serializer( 
    "upgrade_account_cycles_operation_fee_parameters"
)

fba_distribute_operation_fee_parameters = new Serializer( 
    "fba_distribute_operation_fee_parameters"
)

asset_settle_cancel_operation_fee_parameters = new Serializer( 
    "asset_settle_cancel_operation_fee_parameters"
)

fill_order_operation_fee_parameters = new Serializer( 
    "fill_order_operation_fee_parameters"
)

fee_parameters = static_variant [
    transfer_operation_fee_parameters    
    limit_order_create_operation_fee_parameters    
    limit_order_cancel_operation_fee_parameters    
    call_order_update_operation_fee_parameters    
    account_create_operation_fee_parameters    
    account_update_operation_fee_parameters    
    account_whitelist_operation_fee_parameters    
    account_upgrade_operation_fee_parameters    
    account_transfer_operation_fee_parameters    
    asset_create_operation_fee_parameters    
    asset_update_operation_fee_parameters    
    asset_update_bitasset_operation_fee_parameters    
    asset_update_feed_producers_operation_fee_parameters    
    asset_issue_operation_fee_parameters    
    asset_reserve_operation_fee_parameters    
    asset_fund_fee_pool_operation_fee_parameters    
    asset_settle_operation_fee_parameters    
    asset_global_settle_operation_fee_parameters    
    asset_publish_feed_operation_fee_parameters    
    witness_create_operation_fee_parameters    
    witness_update_operation_fee_parameters    
    proposal_create_operation_fee_parameters    
    proposal_update_operation_fee_parameters    
    proposal_delete_operation_fee_parameters    
    withdraw_permission_create_operation_fee_parameters    
    withdraw_permission_update_operation_fee_parameters    
    withdraw_permission_claim_operation_fee_parameters    
    withdraw_permission_delete_operation_fee_parameters    
    committee_member_create_operation_fee_parameters    
    committee_member_update_operation_fee_parameters    
    committee_member_update_global_parameters_operation_fee_parameters    
    vesting_balance_create_operation_fee_parameters    
    vesting_balance_withdraw_operation_fee_parameters    
    worker_create_operation_fee_parameters    
    custom_operation_fee_parameters    
    assert_operation_fee_parameters    
    balance_claim_operation_fee_parameters    
    override_transfer_operation_fee_parameters    
    transfer_to_blind_operation_fee_parameters    
    blind_transfer_operation_fee_parameters    
    transfer_from_blind_operation_fee_parameters    
    asset_claim_fees_operation_fee_parameters    
    board_update_chain_authority_operation_fee_parameters    
    update_queue_parameters_operation_fee_parameters    
    create_license_type_operation_fee_parameters    
    issue_license_operation_fee_parameters    
    tether_accounts_operation_fee_parameters    
    asset_create_issue_request_operation_fee_parameters    
    asset_deny_issue_request_operation_fee_parameters    
    wire_out_operation_fee_parameters    
    wire_out_complete_operation_fee_parameters    
    wire_out_reject_operation_fee_parameters    
    transfer_vault_to_wallet_operation_fee_parameters    
    transfer_wallet_to_vault_operation_fee_parameters    
    submit_reserve_cycles_to_queue_operation_fee_parameters    
    submit_cycles_to_queue_operation_fee_parameters    
    record_submit_reserve_cycles_to_queue_operation_fee_parameters    
    record_submit_charter_license_cycles_operation_fee_parameters    
    record_distribute_dascoin_operation_fee_parameters    
    asset_distribute_completed_request_operation_fee_parameters    
    upgrade_account_cycles_operation_fee_parameters    
    fba_distribute_operation_fee_parameters    
    asset_settle_cancel_operation_fee_parameters    
    fill_order_operation_fee_parameters
]

fee_schedule = new Serializer( 
    "fee_schedule"
    parameters: set fee_parameters
    scale: uint32
)

void_result = new Serializer( 
    "void_result"
)

asset = new Serializer( 
    "asset"
    amount: int64
    asset_id: protocol_id_type "asset"
)

operation_result = static_variant [
    void_result    
    object_id_type    
    asset
]

processed_transaction = new Serializer( 
    "processed_transaction"
    ref_block_num: uint16
    ref_block_prefix: uint32
    expiration: time_point_sec
    operations: array operation
    extensions: set future_extensions
    signatures: array bytes 65
    operation_results: array operation_result
)

signed_block = new Serializer( 
    "signed_block"
    previous: bytes 20
    timestamp: time_point_sec
    witness: protocol_id_type "witness"
    transaction_merkle_root: bytes 20
    extensions: set future_extensions
    witness_signature: bytes 65
    transactions: array processed_transaction
)

block_header = new Serializer( 
    "block_header"
    previous: bytes 20
    timestamp: time_point_sec
    witness: protocol_id_type "witness"
    transaction_merkle_root: bytes 20
    extensions: set future_extensions
)

signed_block_header = new Serializer( 
    "signed_block_header"
    previous: bytes 20
    timestamp: time_point_sec
    witness: protocol_id_type "witness"
    transaction_merkle_root: bytes 20
    extensions: set future_extensions
    witness_signature: bytes 65
)

public_key = new Serializer( 
    "public_key"
    key_data: bytes 33
)

memo_data = new Serializer( 
    "memo_data"
    from: public_key
    to: public_key
    nonce: uint64
    message: bytes()
)

transfer = new Serializer( 
    "transfer"
    fee: asset
    from: protocol_id_type "account"
    to: protocol_id_type "account"
    amount: asset
    memo: optional memo_data
    extensions: set future_extensions
)

limit_order_create = new Serializer( 
    "limit_order_create"
    fee: asset
    seller: protocol_id_type "account"
    amount_to_sell: asset
    min_to_receive: asset
    expiration: time_point_sec
    fill_or_kill: bool
    extensions: set future_extensions
)

limit_order_cancel = new Serializer( 
    "limit_order_cancel"
    fee: asset
    fee_paying_account: protocol_id_type "account"
    order: protocol_id_type "limit_order"
    extensions: set future_extensions
)

call_order_update = new Serializer( 
    "call_order_update"
    fee: asset
    funding_account: protocol_id_type "account"
    delta_collateral: asset
    delta_debt: asset
    extensions: set future_extensions
)

authority = new Serializer( 
    "authority"
    weight_threshold: uint32
    account_auths: map (protocol_id_type "account"), (uint16)
    key_auths: map (public_key), (uint16)
    address_auths: map (address), (uint16)
)

account_options = new Serializer( 
    "account_options"
    memo_key: public_key
    voting_account: protocol_id_type "account"
    num_witness: uint16
    num_committee: uint16
    votes: set vote_id
    extensions: set future_extensions
)

account_create = new Serializer( 
    "account_create"
    fee: asset
    kind: uint8
    registrar: protocol_id_type "account"
    referrer: protocol_id_type "account"
    referrer_percent: uint16
    name: string
    owner: authority
    active: authority
    options: account_options
)

account_update = new Serializer( 
    "account_update"
    fee: asset
    account: protocol_id_type "account"
    owner: optional authority
    active: optional authority
    new_options: optional account_options
)

account_whitelist = new Serializer( 
    "account_whitelist"
    fee: asset
    authorizing_account: protocol_id_type "account"
    account_to_list: protocol_id_type "account"
    new_listing: uint8
    extensions: set future_extensions
)

account_upgrade = new Serializer( 
    "account_upgrade"
    fee: asset
    account_to_upgrade: protocol_id_type "account"
    upgrade_to_lifetime_member: bool
    extensions: set future_extensions
)

account_transfer = new Serializer( 
    "account_transfer"
    fee: asset
    account_id: protocol_id_type "account"
    new_owner: protocol_id_type "account"
    extensions: set future_extensions
)

price = new Serializer( 
    "price"
    base: asset
    quote: asset
)

asset_options = new Serializer( 
    "asset_options"
    max_supply: int64
    market_fee_percent: uint16
    max_market_fee: int64
    issuer_permissions: uint16
    flags: uint16
    core_exchange_rate: price
    whitelist_authorities: set protocol_id_type "account"
    blacklist_authorities: set protocol_id_type "account"
    whitelist_markets: set protocol_id_type "asset"
    blacklist_markets: set protocol_id_type "asset"
    description: string
    extensions: set future_extensions
)

bitasset_options = new Serializer( 
    "bitasset_options"
    feed_lifetime_sec: uint32
    minimum_feeds: uint8
    force_settlement_delay_sec: uint32
    force_settlement_offset_percent: uint16
    maximum_force_settlement_volume: uint16
    short_backing_asset: protocol_id_type "asset"
    extensions: set future_extensions
)

asset_create = new Serializer( 
    "asset_create"
    fee: asset
    issuer: protocol_id_type "account"
    symbol: string
    precision: uint8
    common_options: asset_options
    bitasset_opts: optional bitasset_options
    is_prediction_market: bool
    extensions: set future_extensions
)

asset_update = new Serializer( 
    "asset_update"
    fee: asset
    issuer: protocol_id_type "account"
    asset_to_update: protocol_id_type "asset"
    new_issuer: optional protocol_id_type "account"
    new_options: asset_options
    extensions: set future_extensions
)

asset_update_bitasset = new Serializer( 
    "asset_update_bitasset"
    fee: asset
    issuer: protocol_id_type "account"
    asset_to_update: protocol_id_type "asset"
    new_options: bitasset_options
    extensions: set future_extensions
)

asset_update_feed_producers = new Serializer( 
    "asset_update_feed_producers"
    fee: asset
    issuer: protocol_id_type "account"
    asset_to_update: protocol_id_type "asset"
    new_feed_producers: set protocol_id_type "account"
    extensions: set future_extensions
)

asset_issue = new Serializer( 
    "asset_issue"
    fee: asset
    issuer: protocol_id_type "account"
    asset_to_issue: asset
    issue_to_account: protocol_id_type "account"
    memo: optional memo_data
    extensions: set future_extensions
)

asset_reserve = new Serializer( 
    "asset_reserve"
    fee: asset
    payer: protocol_id_type "account"
    amount_to_reserve: asset
    extensions: set future_extensions
)

asset_fund_fee_pool = new Serializer( 
    "asset_fund_fee_pool"
    fee: asset
    from_account: protocol_id_type "account"
    asset_id: protocol_id_type "asset"
    amount: int64
    extensions: set future_extensions
)

asset_settle = new Serializer( 
    "asset_settle"
    fee: asset
    account: protocol_id_type "account"
    amount: asset
    extensions: set future_extensions
)

asset_global_settle = new Serializer( 
    "asset_global_settle"
    fee: asset
    issuer: protocol_id_type "account"
    asset_to_settle: protocol_id_type "asset"
    settle_price: price
    extensions: set future_extensions
)

price_feed = new Serializer( 
    "price_feed"
    settlement_price: price
    maintenance_collateral_ratio: uint16
    maximum_short_squeeze_ratio: uint16
    core_exchange_rate: price
)

asset_publish_feed = new Serializer( 
    "asset_publish_feed"
    fee: asset
    publisher: protocol_id_type "account"
    asset_id: protocol_id_type "asset"
    feed: price_feed
    extensions: set future_extensions
)

witness_create = new Serializer( 
    "witness_create"
    fee: asset
    witness_account: protocol_id_type "account"
    url: string
    block_signing_key: public_key
)

witness_update = new Serializer( 
    "witness_update"
    fee: asset
    witness: protocol_id_type "witness"
    witness_account: protocol_id_type "account"
    new_url: optional string
    new_signing_key: optional public_key
)

op_wrapper = new Serializer( 
    "op_wrapper"
    op: operation
)

proposal_create = new Serializer( 
    "proposal_create"
    fee: asset
    fee_paying_account: protocol_id_type "account"
    expiration_time: time_point_sec
    proposed_ops: array op_wrapper
    review_period_seconds: optional uint32
    extensions: set future_extensions
)

proposal_update = new Serializer( 
    "proposal_update"
    fee: asset
    fee_paying_account: protocol_id_type "account"
    proposal: protocol_id_type "proposal"
    active_approvals_to_add: set protocol_id_type "account"
    active_approvals_to_remove: set protocol_id_type "account"
    owner_approvals_to_add: set protocol_id_type "account"
    owner_approvals_to_remove: set protocol_id_type "account"
    key_approvals_to_add: set public_key
    key_approvals_to_remove: set public_key
    extensions: set future_extensions
)

proposal_delete = new Serializer( 
    "proposal_delete"
    fee: asset
    fee_paying_account: protocol_id_type "account"
    using_owner_authority: bool
    proposal: protocol_id_type "proposal"
    extensions: set future_extensions
)

withdraw_permission_create = new Serializer( 
    "withdraw_permission_create"
    fee: asset
    withdraw_from_account: protocol_id_type "account"
    authorized_account: protocol_id_type "account"
    withdrawal_limit: asset
    withdrawal_period_sec: uint32
    periods_until_expiration: uint32
    period_start_time: time_point_sec
)

withdraw_permission_update = new Serializer( 
    "withdraw_permission_update"
    fee: asset
    withdraw_from_account: protocol_id_type "account"
    authorized_account: protocol_id_type "account"
    permission_to_update: protocol_id_type "withdraw_permission"
    withdrawal_limit: asset
    withdrawal_period_sec: uint32
    period_start_time: time_point_sec
    periods_until_expiration: uint32
)

withdraw_permission_claim = new Serializer( 
    "withdraw_permission_claim"
    fee: asset
    withdraw_permission: protocol_id_type "withdraw_permission"
    withdraw_from_account: protocol_id_type "account"
    withdraw_to_account: protocol_id_type "account"
    amount_to_withdraw: asset
    memo: optional memo_data
)

withdraw_permission_delete = new Serializer( 
    "withdraw_permission_delete"
    fee: asset
    withdraw_from_account: protocol_id_type "account"
    authorized_account: protocol_id_type "account"
    withdrawal_permission: protocol_id_type "withdraw_permission"
)

committee_member_create = new Serializer( 
    "committee_member_create"
    fee: asset
    committee_member_account: protocol_id_type "account"
    url: string
)

committee_member_update = new Serializer( 
    "committee_member_update"
    fee: asset
    committee_member: protocol_id_type "committee_member"
    committee_member_account: protocol_id_type "account"
    new_url: optional string
)

chain_parameters = new Serializer( 
    "chain_parameters"
    current_fees: fee_schedule
    block_interval: uint8
    maintenance_interval: uint32
    maintenance_skip_slots: uint8
    committee_proposal_review_period: uint32
    maximum_transaction_size: uint32
    maximum_block_size: uint32
    maximum_time_until_expiration: uint32
    maximum_proposal_lifetime: uint32
    maximum_asset_whitelist_authorities: uint8
    maximum_asset_feed_publishers: uint8
    maximum_witness_count: uint16
    maximum_committee_count: uint16
    maximum_authority_membership: uint16
    reserve_percent_of_fee: uint16
    network_percent_of_fee: uint16
    lifetime_referrer_percent_of_fee: uint16
    cashback_vesting_period_seconds: uint32
    cashback_vesting_threshold: int64
    count_non_member_votes: bool
    allow_non_member_whitelists: bool
    witness_pay_per_block: int64
    worker_budget_per_day: int64
    max_predicate_opcode: uint16
    fee_liquidation_threshold: int64
    accounts_per_fee_scale: uint16
    account_fee_scale_bitshifts: uint8
    max_authority_depth: uint8
    upgrade_event_interval_days: uint16
    minimum_transfer_limit: int64
    maximum_transfer_limit: int64
    limit_interval_elapse_time_seconds: uint32
    web_asset_request_expiration_time_seconds: uint32
    reward_interval_time_seconds: uint32
    dascoin_reward_amount: int64
    enable_cycle_issuing: bool
    enable_dascoin_queue: bool
    extensions: set future_extensions
)

committee_member_update_global_parameters = new Serializer( 
    "committee_member_update_global_parameters"
    fee: asset
    new_parameters: chain_parameters
)

linear_vesting_policy_initializer = new Serializer( 
    "linear_vesting_policy_initializer"
    begin_timestamp: time_point_sec
    vesting_cliff_seconds: uint32
    vesting_duration_seconds: uint32
)

cdd_vesting_policy_initializer = new Serializer( 
    "cdd_vesting_policy_initializer"
    start_claim: time_point_sec
    vesting_seconds: uint32
)

vesting_policy_initializer = static_variant [
    linear_vesting_policy_initializer    
    cdd_vesting_policy_initializer
]

vesting_balance_create = new Serializer( 
    "vesting_balance_create"
    fee: asset
    creator: protocol_id_type "account"
    owner: protocol_id_type "account"
    amount: asset
    policy: vesting_policy_initializer
)

vesting_balance_withdraw = new Serializer( 
    "vesting_balance_withdraw"
    fee: asset
    vesting_balance: protocol_id_type "vesting_balance"
    owner: protocol_id_type "account"
    amount: asset
)

refund_worker_initializer = new Serializer( 
    "refund_worker_initializer"
)

vesting_balance_worker_initializer = new Serializer( 
    "vesting_balance_worker_initializer"
    pay_vesting_period_days: uint16
)

burn_worker_initializer = new Serializer( 
    "burn_worker_initializer"
)

worker_initializer = static_variant [
    refund_worker_initializer    
    vesting_balance_worker_initializer    
    burn_worker_initializer
]

worker_create = new Serializer( 
    "worker_create"
    fee: asset
    owner: protocol_id_type "account"
    work_begin_date: time_point_sec
    work_end_date: time_point_sec
    daily_pay: int64
    name: string
    url: string
    initializer: worker_initializer
)

custom = new Serializer( 
    "custom"
    fee: asset
    payer: protocol_id_type "account"
    required_auths: set protocol_id_type "account"
    id: uint16
    data: bytes()
)

account_name_eq_lit_predicate = new Serializer( 
    "account_name_eq_lit_predicate"
    account_id: protocol_id_type "account"
    name: string
)

asset_symbol_eq_lit_predicate = new Serializer( 
    "asset_symbol_eq_lit_predicate"
    asset_id: protocol_id_type "asset"
    symbol: string
)

block_id_predicate = new Serializer( 
    "block_id_predicate"
    id: bytes 20
)

predicate = static_variant [
    account_name_eq_lit_predicate    
    asset_symbol_eq_lit_predicate    
    block_id_predicate
]

assert = new Serializer( 
    "assert"
    fee: asset
    fee_paying_account: protocol_id_type "account"
    predicates: array predicate
    required_auths: set protocol_id_type "account"
    extensions: set future_extensions
)

balance_claim = new Serializer( 
    "balance_claim"
    fee: asset
    deposit_to_account: protocol_id_type "account"
    balance_to_claim: protocol_id_type "balance"
    balance_owner_key: public_key
    total_claimed: asset
)

override_transfer = new Serializer( 
    "override_transfer"
    fee: asset
    issuer: protocol_id_type "account"
    from: protocol_id_type "account"
    to: protocol_id_type "account"
    amount: asset
    memo: optional memo_data
    extensions: set future_extensions
)

stealth_confirmation = new Serializer( 
    "stealth_confirmation"
    one_time_key: public_key
    to: optional public_key
    encrypted_memo: bytes()
)

blind_output = new Serializer( 
    "blind_output"
    commitment: bytes 33
    range_proof: bytes()
    owner: authority
    stealth_memo: optional stealth_confirmation
)

transfer_to_blind = new Serializer( 
    "transfer_to_blind"
    fee: asset
    amount: asset
    from: protocol_id_type "account"
    blinding_factor: bytes 32
    outputs: array blind_output
)

blind_input = new Serializer( 
    "blind_input"
    commitment: bytes 33
    owner: authority
)

blind_transfer = new Serializer( 
    "blind_transfer"
    fee: asset
    inputs: array blind_input
    outputs: array blind_output
)

transfer_from_blind = new Serializer( 
    "transfer_from_blind"
    fee: asset
    amount: asset
    to: protocol_id_type "account"
    blinding_factor: bytes 32
    inputs: array blind_input
)

asset_claim_fees = new Serializer( 
    "asset_claim_fees"
    fee: asset
    issuer: protocol_id_type "account"
    amount_to_claim: asset
    extensions: set future_extensions
)

board_update_chain_authority = new Serializer( 
    "board_update_chain_authority"
    fee: asset
    kind: string
    account: protocol_id_type "account"
    committee_member_account: protocol_id_type "account"
)

update_queue_parameters = new Serializer( 
    "update_queue_parameters"
    fee: asset
    issuer: protocol_id_type "account"
    enable_dascoin_queue: optional bool
    reward_interval_time_seconds: optional uint32
    dascoin_reward_amount: optional int64
)

create_license = new Serializer( 
    "create_license"
    fee: asset
    admin: protocol_id_type "account"
    name: string
    amount: int64
    kind: string
    balance_multipliers: array uint8
    requeue_multipliers: array uint8
    return_multipliers: array uint8
)

issue_license = new Serializer( 
    "issue_license"
    fee: asset
    issuer: protocol_id_type "account"
    account: protocol_id_type "account"
    license: protocol_id_type "license"
    bonus_percentage: int64
    frequency_lock: int64
    activated_at: time_point_sec
    extensions: set future_extensions
)

tether_accounts = new Serializer( 
    "tether_accounts"
    fee: asset
    wallet_account: protocol_id_type "account"
    vault_account: protocol_id_type "account"
    extensions: set future_extensions
)

asset_create_issue_request = new Serializer( 
    "asset_create_issue_request"
    fee: asset
    issuer: protocol_id_type "account"
    receiver: protocol_id_type "account"
    amount: int64
    asset_id: protocol_id_type "asset"
    reserved_amount: int64
    extensions: set future_extensions
)

asset_deny_issue_request = new Serializer( 
    "asset_deny_issue_request"
    fee: asset
    authenticator: protocol_id_type "account"
    request: protocol_id_type "issue_asset_request"
    extensions: set future_extensions
)

wire_out = new Serializer( 
    "wire_out"
    account: protocol_id_type "account"
    asset_to_wire: asset
    memo: string
    extensions: set future_extensions
)

wire_out_complete = new Serializer( 
    "wire_out_complete"
    wire_out_handler: protocol_id_type "account"
    holder_object_id: protocol_id_type "wire_out_holder"
    extensions: set future_extensions
)

wire_out_reject = new Serializer( 
    "wire_out_reject"
    wire_out_handler: protocol_id_type "account"
    holder_object_id: protocol_id_type "wire_out_holder"
    extensions: set future_extensions
)

transfer_vault_to_wallet = new Serializer( 
    "transfer_vault_to_wallet"
    fee: asset
    from_vault: protocol_id_type "account"
    to_wallet: protocol_id_type "account"
    asset_to_transfer: asset
    reserved_to_transfer: int64
    extensions: set future_extensions
)

transfer_wallet_to_vault = new Serializer( 
    "transfer_wallet_to_vault"
    fee: asset
    from_wallet: protocol_id_type "account"
    to_vault: protocol_id_type "account"
    asset_to_transfer: asset
    reserved_to_transfer: int64
    extensions: set future_extensions
)

submit_reserve_cycles_to_queue = new Serializer( 
    "submit_reserve_cycles_to_queue"
    fee: asset
    issuer: protocol_id_type "account"
    account: protocol_id_type "account"
    amount: int64
    frequency_lock: int64
    comment: string
    extensions: set future_extensions
)

submit_cycles_to_queue = new Serializer( 
    "submit_cycles_to_queue"
    fee: asset
    account: protocol_id_type "account"
    amount: int64
    extensions: set future_extensions
)

record_submit_reserve_cycles_to_queue = new Serializer( 
    "record_submit_reserve_cycles_to_queue"
    fee: asset
    cycle_issuer: protocol_id_type "account"
    account: protocol_id_type "account"
    amount: int64
    frequency_lock: int64
    comment: string
    extensions: set future_extensions
)

record_submit_charter_license_cycles = new Serializer( 
    "record_submit_charter_license_cycles"
    fee: asset
    license_issuer: protocol_id_type "account"
    account: protocol_id_type "account"
    amount: int64
    frequency_lock: int64
    extensions: set future_extensions
)

record_distribute_dascoin = new Serializer( 
    "record_distribute_dascoin"
    fee: asset
    origin: string
    license: optional protocol_id_type "license"
    account: protocol_id_type "account"
    cycle_amount: int64
    frequency: int64
    reward_amount: int64
    time: time_point_sec
    extensions: set future_extensions
)

asset_distribute_completed_request = new Serializer( 
    "asset_distribute_completed_request"
    fee: asset
    issuer: protocol_id_type "account"
    receiver: protocol_id_type "account"
    amount: asset
    reserved_amount: int64
    extensions: set future_extensions
)

upgrade_account_cycles = new Serializer( 
    "upgrade_account_cycles"
    fee: asset
    account: protocol_id_type "account"
    description: string
    extensions: set future_extensions
)

fba_distribute = new Serializer( 
    "fba_distribute"
    fee: asset
    account_id: protocol_id_type "account"
    fba_id: protocol_id_type "fba_accumulator"
    amount: int64
)

asset_settle_cancel = new Serializer( 
    "asset_settle_cancel"
    fee: asset
    settlement: protocol_id_type "force_settlement"
    account: protocol_id_type "account"
    amount: asset
    extensions: set future_extensions
)

fill_order = new Serializer( 
    "fill_order"
    fee: asset
    order_id: object_id_type
    account_id: protocol_id_type "account"
    pays: asset
    receives: asset
)

operation = static_variant [
    transfer    
    limit_order_create    
    limit_order_cancel    
    call_order_update    
    account_create    
    account_update    
    account_whitelist    
    account_upgrade    
    account_transfer    
    asset_create    
    asset_update    
    asset_update_bitasset    
    asset_update_feed_producers    
    asset_issue    
    asset_reserve    
    asset_fund_fee_pool    
    asset_settle    
    asset_global_settle    
    asset_publish_feed    
    witness_create    
    witness_update    
    proposal_create    
    proposal_update    
    proposal_delete    
    withdraw_permission_create    
    withdraw_permission_update    
    withdraw_permission_claim    
    withdraw_permission_delete    
    committee_member_create    
    committee_member_update    
    committee_member_update_global_parameters    
    vesting_balance_create    
    vesting_balance_withdraw    
    worker_create    
    custom    
    assert    
    balance_claim    
    override_transfer    
    transfer_to_blind    
    blind_transfer    
    transfer_from_blind    
    asset_claim_fees    
    board_update_chain_authority    
    update_queue_parameters    
    create_license    
    issue_license    
    tether_accounts    
    asset_create_issue_request    
    asset_deny_issue_request    
    wire_out    
    wire_out_complete    
    wire_out_reject    
    transfer_vault_to_wallet    
    transfer_wallet_to_vault    
    submit_reserve_cycles_to_queue    
    submit_cycles_to_queue    
    record_submit_reserve_cycles_to_queue    
    record_submit_charter_license_cycles    
    record_distribute_dascoin    
    asset_distribute_completed_request    
    upgrade_account_cycles    
    fba_distribute    
    asset_settle_cancel    
    fill_order
]

transaction = new Serializer( 
    "transaction"
    ref_block_num: uint16
    ref_block_prefix: uint32
    expiration: time_point_sec
    operations: array operation
    extensions: set future_extensions
)

signed_transaction = new Serializer( 
    "signed_transaction"
    ref_block_num: uint16
    ref_block_prefix: uint32
    expiration: time_point_sec
    operations: array operation
    extensions: set future_extensions
    signatures: array bytes 65
)

