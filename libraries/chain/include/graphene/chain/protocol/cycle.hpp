/**
 * DASCOIN!
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {

///////////////////////////////
// OPERATIONS:               //
///////////////////////////////

  /**
   * @brief Request to issue cycles to an account.
   * @ingroup operations
   *
   * An authorized cycle issuing authority can request to issue a certain amount of cycles.
   * An independent authorized cycle authentication authority must inspect and approve this request.
   */
  struct submit_reserve_cycles_to_queue_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;  // This MUST be the cycle issuer authority.

    account_id_type account;
    share_type amount;
    frequency_type frequency_lock;
    string comment;  // Comment the reason the cycles were submitted.

    submit_reserve_cycles_to_queue_operation() = default;
    explicit submit_reserve_cycles_to_queue_operation(account_id_type issuer, account_id_type account,
                                                      share_type amount, frequency_type frequency_lock,
                                                      const string& comment)
        : issuer(issuer)
        , account(account)
        , amount(amount)
        , frequency_lock(frequency_lock)
        , comment(comment) {}

    extensions_type extensions;

    account_id_type fee_payer() const { return issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Submit cycles to the DasCoin distribution queue.
   * @ingroup operations
   *
   * A user can submit their cycles to the dascoin distribution queue where they await to be minted.
   */
  struct submit_cycles_to_queue_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;

    account_id_type account;
    share_type amount;

    extensions_type extensions;

    submit_cycles_to_queue_operation() = default;
    submit_cycles_to_queue_operation(account_id_type account, share_type amount)
        : account(account)
        , amount(amount) {}

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct record_submit_reserve_cycles_to_queue_operation : public base_operation
  {
    struct fee_parameters_type {};  // Virtual operation.

    asset fee;

    account_id_type cycle_issuer;

    account_id_type account;
    share_type amount;
    frequency_type frequency_lock;
    string comment;  // Comment the reason the cycles were submitted.

    extensions_type extensions;

    record_submit_reserve_cycles_to_queue_operation() = default;
    explicit record_submit_reserve_cycles_to_queue_operation(account_id_type cycle_issuer, account_id_type account,
                                                             share_type amount, frequency_type frequency_lock,
                                                             const string& comment)
        : cycle_issuer(cycle_issuer)
        , amount(amount)
        , frequency_lock(frequency_lock)
        , comment(comment) {}


    account_id_type fee_payer() const { return account; }
    void validate() const { FC_ASSERT( false ); }
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct record_submit_charter_license_cycles_operation : public base_operation
  {
    struct fee_parameters_type {};  // Virtual operation.

    asset fee;

    account_id_type license_issuer;

    account_id_type account;
    share_type amount;
    frequency_type frequency_lock;
    
    extensions_type extensions;

    record_submit_charter_license_cycles_operation() = default;
    record_submit_charter_license_cycles_operation(account_id_type license_issuer, account_id_type account,
                                                   share_type amount, frequency_type frequency_lock)
        : license_issuer(license_issuer)
        , account(account)
        , amount(amount)
        , frequency_lock(frequency_lock) {}

    account_id_type fee_payer() const { return account; }
    void validate() const { FC_ASSERT( false ); }
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct update_queue_parameters_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;  // This MUST be the cycle issuer authority.

    optional<bool> enable_dascoin_queue;
    optional<uint32_t> reward_interval_time_seconds;
    optional<uint32_t> dascoin_reward_amount;

    update_queue_parameters_operation() = default;
    explicit update_queue_parameters_operation(account_id_type issuer,
      optional<bool> enable_dascoin_queue,
      optional<uint32_t> reward_interval_time_seconds,
      optional<uint32_t> dascoin_reward_amount) :
        issuer(issuer),
        enable_dascoin_queue(enable_dascoin_queue),
        reward_interval_time_seconds(reward_interval_time_seconds),
        dascoin_reward_amount(dascoin_reward_amount) {}

    extensions_type extensions;

    account_id_type fee_payer() const { return issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::submit_reserve_cycles_to_queue_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::submit_reserve_cycles_to_queue_operation,
            (fee)
            (issuer)
            (account)
            (amount)
            (frequency_lock)
            (comment)
            (extensions)
          )

FC_REFLECT( graphene::chain::submit_cycles_to_queue_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::submit_cycles_to_queue_operation,
            (fee)
            (account)
            (amount)
            (extensions)
          )

FC_REFLECT( graphene::chain::record_submit_reserve_cycles_to_queue_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::record_submit_reserve_cycles_to_queue_operation,
            (fee)
            (cycle_issuer)
            (account)
            (amount)
            (frequency_lock)
            (comment)
            (extensions)
          )

FC_REFLECT( graphene::chain::record_submit_charter_license_cycles_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::record_submit_charter_license_cycles_operation,
            (fee)
            (license_issuer)
            (account)
            (amount)
            (frequency_lock)
            (extensions)
          )

FC_REFLECT( graphene::chain::update_queue_parameters_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::update_queue_parameters_operation,
            (fee)
            (issuer)
            (enable_dascoin_queue)
            (reward_interval_time_seconds)
            (dascoin_reward_amount)
          )