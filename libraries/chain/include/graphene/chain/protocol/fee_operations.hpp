
#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {
/**
   * @brief Request to change fee for particular operation.
   *
   */
  struct change_fee_for_operation : public base_operation
  {
    struct fee_parameters_type {};  // No fees are paid for this operation.

    asset fee;
    account_id_type issuer;
    uint64_t new_fee;
    unsigned op_num;
    string comment;

    change_fee_for_operation() = default;
    explicit change_fee_for_operation(account_id_type issuer, uint64_t new_fee, unsigned op_num, const string& comment)
        : issuer(issuer)
        , new_fee(new_fee)
        , comment(comment)
        , op_num(op_num) {}


    account_id_type fee_payer() const { return issuer; }
    void validate() const{}
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };


} }

FC_REFLECT( graphene::chain::change_fee_for_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::change_fee_for_operation, (fee)(issuer)(new_fee)(op_num)(comment) )
