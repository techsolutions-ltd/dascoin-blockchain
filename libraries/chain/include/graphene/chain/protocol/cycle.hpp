/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
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

    frequency_type frequency;
    string comment;

    extensions_type extensions;

    submit_cycles_to_queue_operation() = default;
    explicit submit_cycles_to_queue_operation(account_id_type account, share_type amount, frequency_type frequency,
                                     const string& comment)
        : account(account),
          amount(amount),
          frequency(frequency),
          comment(comment) {}

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Submit cycles by license to the DasCoin distribution queue.
   * @ingroup operations
   *
   * A user can submit their cycles to the dascoin distribution queue where they await to be minted.
   */
  struct submit_cycles_to_queue_by_license_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;

    account_id_type account;
    share_type amount;

    license_type_id_type license_type;
    frequency_type frequency_lock;
    string comment;

    extensions_type extensions;

    submit_cycles_to_queue_by_license_operation() = default;
    explicit submit_cycles_to_queue_by_license_operation(account_id_type account, share_type amount, license_type_id_type license_type,
                                                         frequency_type frequency_lock, const string& comment)
            : account(account),
              amount(amount),
              license_type(license_type),
              frequency_lock(frequency_lock),
              comment(comment) {}

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
    optional<share_type> dascoin_reward_amount;

    update_queue_parameters_operation() = default;
    explicit update_queue_parameters_operation(account_id_type issuer,
      optional<bool> enable_dascoin_queue,
      optional<uint32_t> reward_interval_time_seconds,
      optional<share_type> dascoin_reward_amount) :
        issuer(issuer),
        enable_dascoin_queue(enable_dascoin_queue),
        reward_interval_time_seconds(reward_interval_time_seconds),
        dascoin_reward_amount(dascoin_reward_amount) {}

    extensions_type extensions;

    account_id_type fee_payer() const { return issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct update_global_frequency_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type authority;  // This MUST be the current license issuer authority.
    frequency_type frequency;
    string comment;

    extensions_type extensions;

    update_global_frequency_operation() = default;
    explicit update_global_frequency_operation(account_id_type authority, frequency_type frequency, 
                                               const string& comment)
      : authority(authority),
        frequency(frequency),
        comment(comment) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct issue_free_cycles_operation : public base_operation
  {
    struct fee_parameters_type{};

    asset fee;

    account_id_type authority;

    uint8_t origin;
    account_id_type account;
    share_type amount;
    string comment;

    extensions_type extensions;

    issue_free_cycles_operation() = default;
    explicit issue_free_cycles_operation(account_id_type authority, uint8_t origin, account_id_type account,
                                         share_type amount, const string& comment)
        : authority(authority)
        , origin(origin)
        , amount(amount)
        , comment(comment) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct issue_cycles_to_license_operation : public base_operation
  {
    struct fee_parameters_type{};

    asset fee;

    account_id_type authority;

    account_id_type account;
    license_type_id_type license;
    share_type amount;
    string origin;
    string comment;

    extensions_type extensions;

    issue_cycles_to_license_operation() = default;
    explicit issue_cycles_to_license_operation(account_id_type authority, account_id_type account,
                                               license_type_id_type license, share_type amount, const string& origin, const string& comment)
            : authority(authority)
            , account(account)
            , license(license)
            , amount(amount)
            , origin(origin)
            , comment(comment) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct purchase_cycle_asset_operation : public base_operation
  {
    struct fee_parameters_type{};

    asset fee;

    account_id_type wallet_id;

    share_type amount;
    frequency_type frequency;
    share_type expected_amount;

    extensions_type extensions;

    purchase_cycle_asset_operation() = default;
    explicit purchase_cycle_asset_operation(account_id_type wallet_id, share_type amount, frequency_type frequency,
                                            share_type expected_amount)
            : wallet_id(wallet_id)
            , amount(amount)
            , frequency(frequency)
            , expected_amount(expected_amount) {}

    account_id_type fee_payer() const { return wallet_id; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct transfer_cycles_from_licence_to_wallet_operation : public base_operation
  {
    struct fee_parameters_type{};

    asset fee;

    account_id_type vault_id;
    license_type_id_type license_id;
    share_type amount;
    account_id_type wallet_id;

    extensions_type extensions;

    transfer_cycles_from_licence_to_wallet_operation() = default;
    explicit transfer_cycles_from_licence_to_wallet_operation(account_id_type vault_id, license_type_id_type license_id, share_type amount,
                                                              account_id_type wallet_id)
            : vault_id(vault_id)
            , license_id(license_id)
            , amount(amount)
            , wallet_id(wallet_id) {}

    account_id_type fee_payer() const { return vault_id; }
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
            (frequency)
            (comment)
            (extensions)
          )

FC_REFLECT( graphene::chain::submit_cycles_to_queue_by_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::submit_cycles_to_queue_by_license_operation,
            (fee)
            (account)
            (amount)
            (license_type)
            (frequency_lock)
            (comment)
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

FC_REFLECT( graphene::chain::update_global_frequency_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::update_global_frequency_operation,
            (fee)
            (authority)
            (frequency)
          )

FC_REFLECT( graphene::chain::issue_free_cycles_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::issue_free_cycles_operation,
            (fee)
            (authority)
            (origin)
            (account)
            (amount)
            (comment)
            (extensions)
          )

FC_REFLECT( graphene::chain::issue_cycles_to_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::issue_cycles_to_license_operation,
            (fee)
            (authority)
            (account)
            (license)
            (amount)
            (origin)
            (comment)
            (extensions)
)

FC_REFLECT( graphene::chain::purchase_cycle_asset_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::purchase_cycle_asset_operation,
            (fee)
            (wallet_id)
            (amount)
            (frequency)
            (expected_amount)
            (extensions)
)

FC_REFLECT( graphene::chain::transfer_cycles_from_licence_to_wallet_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::transfer_cycles_from_licence_to_wallet_operation,
            (fee)
            (vault_id)
            (license_id)
            (amount)
            (wallet_id)
            (extensions)
)
