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
   * An authorized license authentication authority may define licenses to be distributed to users.
   */
  struct license_type_create_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     account_id_type license_authentication_account;  // This MUST be the license authentication authority.

     // License data. NOTE: should match the @ref license_type_object!
     string name;                    // Name of the license.
     share_type amount;              // The amount of cycles the license grants.
     uint8_t upgrades;               // The number of cycle upgrades the license grants.
     uint32_t policy_flags = 0;      // Cycle policy flags.

     account_id_type fee_payer() const { return license_authentication_account; }
     void validate() const;
     share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief edit a license type object
   * @ingroup operations
   *
   * An authorized license authentication authority may edit licenses to be distributed to users.
   *
   * WARNING: this operation is NOT RETROACTIVE!
   */
  struct license_type_edit_operation : public base_operation
  {
     struct fee_parameters_type {};  // No fees are paid for this operation.

     asset fee;
     license_type_id_type license;
     account_id_type license_authentication_account;  // This MUST be the license authentication authority.

     // License data. NOTE: should match the @ref license_type_object
     optional<string> name;                       // Name of the license.
     optional<share_type> amount;                 // The amount of cycles the license grants.
     optional<uint8_t> upgrades;                  // The number of cycle upgrades the license grants.
     optional<uint32_t> policy_flags;             // How is the cycle queue handled?

     account_id_type fee_payer() const { return license_authentication_account; }
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
    account_id_type license_issuing_account;  // This MUST be the license authentication authority.

    account_id_type account;                  // The account to benefit the license.
    license_type_id_type license;             // The license to be granted.

    optional<frequency_type> account_frequency;

    extensions_type   extensions;

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

    license_request_id_type request;  // The license request we are denying.

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
            (license_authentication_account)
            (name)
            (amount)
            (upgrades)
            (policy_flags)
          )

FC_REFLECT( graphene::chain::license_type_edit_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_type_edit_operation,
            (fee)
            (license)
            (license_authentication_account)
            (name)
            (amount)
            (upgrades)
            (policy_flags)
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
            (extensions)
          )

FC_REFLECT( graphene::chain::license_approve_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_approve_operation,
            (fee)
            (license_authentication_account)
            (request)
            (extensions)
          )

FC_REFLECT( graphene::chain::license_deny_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::license_deny_operation,
            (fee)
            (license_authentication_account)
            (request)
          )
