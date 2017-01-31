/**
 * DASCOIN!
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
   * An authorized license administration authority may define licenses to be distributed to users.
   */
  struct license_type_create_operation : public base_operation
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

     account_id_type fee_payer() const { return admin; }
     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief edit a license type object
   * @ingroup operations
   *
   * An authorized license administration authority may edit licenses to be distributed to users.
   *
   * WARNING: this operation is NOT RETROACTIVE!
   */
  struct license_type_edit_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     license_type_id_type license;
     account_id_type admin;

     optional<string> name;
     optional<share_type> amount;

     optional<upgrade_multiplier_type> balance_multipliers;
     optional<upgrade_multiplier_type> requeue_multipliers;
     optional<upgrade_multiplier_type> return_multipliers;

     account_id_type fee_payer() const { return admin; }
     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief delete a license type object
   * @ingroup operations
   *
   * An authorized license authentication authority may delete an existing license given to users.
   * Upon deletion, all license holders must revert to the default NO_LICENSE. Granted cycle balances are unaffected.
   *
   * WARNING: this operation is NOT RETROACTIVE!
   */
  struct license_type_delete_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_authentication_account;  // This MUST be the license authentication authority.

    license_type_id_type license;

    account_id_type fee_payer() const { return license_authentication_account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Request a license to be granted an account
   * @ingroup operations
   *
   * An authorized license issuing authority may request a license to be granted to an account.
   * An independent authorized license authentication authority must inspect and approve this request. Upon approval,
   * the license is irredeemably granted to the account.
   */
  struct license_request_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_issuing_account;  // This MUST be the license issuer authority.

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.
    frequency_type frequency;                 // The frequency lock on this license, zero for none.

    extensions_type extensions;

    account_id_type fee_payer() const { return license_issuing_account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Approve a license grant to an account
   * @ingroup operations
   *
   * An authorized license authentication authority may approve an existing request made by the license issuing
   * authority. If such a request is approved the license is irrevocably granted to the account.
   */
  struct license_approve_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_authentication_account;  // This MUST be the license issuing authority.

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.

    extensions_type   extensions;

    account_id_type fee_payer() const { return license_authentication_account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Deny a license grant to an account
   * @ingroup operations
   *
   * An authorized license authentication authority may deny an existing request made by the license issuing
   * authority. This operation is also applied if the request object expires.
   *
   * If the authorized license authentication authority does not deny the request, the request is automatically approved
   * on expiration.
   */
  struct license_deny_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type license_authentication_account;  // This MUST be the license authentication authority.

    license_request_id_type request;  // The license request we are denying.

    account_id_type fee_payer() const { return license_authentication_account; }
    void validate() const {}
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::license_type_create_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_type_create_operation,
            (fee)
            (admin)
            (name)
            (amount)
            (kind)
            (balance_multipliers)
            (requeue_multipliers)
            (return_multipliers)
          )

FC_REFLECT( graphene::chain::license_type_edit_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_type_edit_operation,
            (fee)
            (license)
            (admin)
            (name)
            (amount)
            (balance_multipliers)
            (requeue_multipliers)
            (return_multipliers)
          )
FC_REFLECT( graphene::chain::license_type_delete_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_type_delete_operation,
            (fee)
            (license_authentication_account)
            (license)
          )

FC_REFLECT( graphene::chain::license_request_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_request_operation,
            (fee)
            (license_issuing_account)
            (account)
            (license)
            (frequency)
            (extensions)
          )

FC_REFLECT( graphene::chain::license_approve_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_approve_operation,
            (fee)
            (license_authentication_account)
            (account)
            (license)
            (extensions)
          )

FC_REFLECT( graphene::chain::license_deny_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_deny_operation,
            (fee)
            (license_authentication_account)
            (request)
          )
