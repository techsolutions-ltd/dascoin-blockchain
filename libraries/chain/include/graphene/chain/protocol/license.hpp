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

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  ///////////////////////////////
  // OPERATIONS:               //
  ///////////////////////////////

  /**
   * @brief create a license type object
   * @ingroup operations
   *
   * Create a new type of license. Must be signed by the current license_administration authority.
   */
  struct create_license_type_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     account_id_type admin;

     string name;
     share_type amount;
     string kind;

     upgrade_multiplier_type balance_multipliers;
     upgrade_multiplier_type requeue_multipliers;
     upgrade_multiplier_type return_multipliers;

     share_type eur_limit;

     create_license_type_operation() = default;
     explicit create_license_type_operation(const string& name, share_type amount, const string& kind,
                                            const upgrade_multiplier_type& balance_multipliers,
                                            const upgrade_multiplier_type& requeue_multipliers,
                                            const upgrade_multiplier_type& return_multipliers, 
                                            share_type eur_limit)
         : name(name)
         , amount(amount)
         , kind(kind)
         , balance_multipliers(balance_multipliers)
         , requeue_multipliers(requeue_multipliers)
         , return_multipliers(return_multipliers)
         , eur_limit(eur_limit) {}

     account_id_type fee_payer() const { return admin; }
     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct edit_license_type_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     account_id_type authority;
     license_type_id_type license_type;

     optional<string> name;
     optional<share_type> amount;
     optional<share_type> eur_limit;

     edit_license_type_operation() = default;

     account_id_type fee_payer() const { return authority; }
     explicit edit_license_type_operation(account_id_type authority, license_type_id_type license_type,
                                          optional<string> name, optional<share_type> amount, optional<share_type> eur_limit)
         : authority(authority)
         , license_type(license_type)
         , name(name)
         , amount(amount)
         , eur_limit(eur_limit) {}

     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Request a license to be granted an account
   * @ingroup operations
   *
   * Grant a license to an account. This operation must be signed by the current license_issuer authority.
   */
  struct issue_license_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.
    share_type bonus_percentage;              // The bonus multiplier of base license cycles.
    frequency_type frequency_lock;            // The frequency lock on this license, zero for none.
    time_point_sec activated_at;              // Time point of activation.

    extensions_type extensions;

    issue_license_operation() = default;
    explicit issue_license_operation(account_id_type issuer,
                                     account_id_type account,
                                     license_type_id_type license,
                                     share_type bonus_percentage,
                                     frequency_type frequency_lock,
                                     time_point_sec activated_at)
        : issuer(issuer),
          account(account),
          license(license),
          bonus_percentage(bonus_percentage),
          frequency_lock(frequency_lock),
          activated_at(activated_at) {}

    account_id_type fee_payer() const { return issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

   /**
    * @brief Update a license issued to an account
    * @ingroup operations
    *
    * Update a license issued to an account. This operation must be signed by the current license_issuer authority.
   */
  struct update_license_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type authority;

    account_id_type account;                  // The account to update the license.
    license_type_id_type license;             // The license to be updated.
    optional<share_type> bonus_percentage;    // Optional bonus multiplier of base license cycles.
    optional<frequency_type> frequency_lock;  // Optional frequency lock on this license, zero for none.
    optional<time_point_sec> activated_at;    // Optional Time point of activation.

    extensions_type extensions;

    update_license_operation() = default;
    explicit update_license_operation(account_id_type authority,
                                     account_id_type account,
                                     license_type_id_type license,
                                     optional<share_type> bonus_percentage,
                                     optional<frequency_type> frequency_lock,
                                     optional<time_point_sec> activated_at)
        : authority(authority),
          account(account),
          license(license),
          bonus_percentage(bonus_percentage),
          frequency_lock(frequency_lock),
          activated_at(activated_at) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::create_license_type_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::create_license_type_operation,
            (fee)
            (admin)
            (name)
            (amount)
            (kind)
            (balance_multipliers)
            (requeue_multipliers)
            (return_multipliers)
            (eur_limit)
          )

FC_REFLECT( graphene::chain::edit_license_type_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::edit_license_type_operation,
            (fee)
            (authority)
            (license_type)
            (name)
            (amount)
            (eur_limit)
          )

FC_REFLECT( graphene::chain::issue_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::issue_license_operation,
            (fee)
            (issuer)
            (account)
            (license)
            (bonus_percentage)
            (frequency_lock)
            (activated_at)
            (extensions)
          )

FC_REFLECT( graphene::chain::update_license_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::update_license_operation,
            (fee)
            (authority)
            (account)
            (license)
            (bonus_percentage)
            (frequency_lock)
            (activated_at)
            (extensions)
)
