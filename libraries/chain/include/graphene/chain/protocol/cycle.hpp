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
  struct cycle_issue_request_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type cycle_issuer;  // This MUST be the cycle issuer authority.

    account_id_type account;
    share_type amount;

    extensions_type extensions;

    account_id_type fee_payer() const { return cycle_issuer; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Finalize issuing cycles to an account.
   * @ingroup operations
   *
   * When the issue request expires, record the final issue of cycles to an account.
   * NOTE: this is a virtual operation.
   */
  struct cycle_issue_complete_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type cycle_authenticator;  // This MUST be the cycle authenticator authority.

    account_id_type account;
    share_type amount;

    extensions_type extensions;

    account_id_type fee_payer() const { return cycle_authenticator; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
   * @brief Deny issuing cycles to an account.
   * @ingroup operations
   *
   * An authorized cycle authentication authority can deny a cycle issuing request.
   * NOTE: this is a virtual operation.
   */
  struct cycle_issue_deny_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type cycle_authenticator;  // This MUST be the cycle authenticator authority.

    cycle_issue_request_id_type request;

    extensions_type extensions;

    account_id_type fee_payer() const { return cycle_authenticator; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::cycle_issue_request_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::cycle_issue_request_operation,
            (fee)
            (cycle_issuer)
            (account)
            (amount)
            (extensions)
          )

FC_REFLECT( graphene::chain::cycle_issue_complete_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::cycle_issue_complete_operation,
            (fee)
            (cycle_authenticator)
            (account)
            (amount)
            (extensions)
          )

FC_REFLECT( graphene::chain::cycle_issue_deny_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::cycle_issue_deny_operation,
            (fee)
            (cycle_authenticator)
            (request)
            (extensions)
          )
