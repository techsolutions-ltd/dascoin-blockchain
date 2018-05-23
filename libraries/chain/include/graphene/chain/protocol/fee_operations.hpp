#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {
   /**
   * @brief Request to change fee for particular operation.
   *
   */
  struct change_operation_fee_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;
    uint64_t new_fee;
    unsigned op_num;
    string comment;

    extensions_type extensions;

    change_operation_fee_operation() = default;
    explicit change_operation_fee_operation(account_id_type issuer, uint64_t new_fee, unsigned op_num, const string& comment)
        : issuer(issuer)
        , new_fee(new_fee)
        , op_num(op_num)
        , comment(comment) {}

    account_id_type fee_payer() const { return issuer; }
    void validate() const{}
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  /**
  * @brief Request to change fee pool account.
  *
  */
 struct change_fee_pool_account_operation : public base_operation
 {
   struct fee_parameters_type {};  // No fees are paid for this operation.

   asset fee;
   account_id_type issuer;
   account_id_type fee_pool_account_id;
   string comment;

   extensions_type extensions;

   change_fee_pool_account_operation() = default;
   explicit change_fee_pool_account_operation(account_id_type issuer, account_id_type fee_pool_account_id, const string& comment)
       : issuer(issuer)
       , fee_pool_account_id(fee_pool_account_id)
       , comment(comment)
   {}

   account_id_type fee_payer() const { return issuer; }
   void validate() const{}
   share_type calculate_fee(const fee_parameters_type&) const { return 0; }
 };

 /**
   * @brief Submit cycles to queue from fee pool account.
   *
   */
  struct fee_pool_cycles_submit_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;
    share_type amount;
    string comment;

    extensions_type extensions;

    fee_pool_cycles_submit_operation() = default;
    explicit fee_pool_cycles_submit_operation(account_id_type issuer, share_type amount, const string& comment)
        : issuer(issuer)
        , amount(amount)
        , comment(comment)
    {}

    account_id_type fee_payer() const { return issuer; }
    void validate() const{}
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }

FC_REFLECT( graphene::chain::change_operation_fee_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::change_operation_fee_operation, (fee)(issuer)(new_fee)(op_num)(comment)(extensions) )

FC_REFLECT( graphene::chain::change_fee_pool_account_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::change_fee_pool_account_operation, (fee)(issuer)(fee_pool_account_id)(comment)(extensions) )

FC_REFLECT( graphene::chain::fee_pool_cycles_submit_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::fee_pool_cycles_submit_operation, (fee)(issuer)(amount)(comment)(extensions) )
