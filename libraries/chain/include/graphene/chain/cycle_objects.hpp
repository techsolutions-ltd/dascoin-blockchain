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

  /**
   * @class cycle_issue_request_object
   * @brief A pending request to issue cycles to an account cycle balance.
   * @ingroup object
   *
   * This is an implementation detail.
   */
  class cycle_issue_request_object : public graphene::db::abstract_object<cycle_issue_request_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_cycle_issue_request_object_type;

      account_id_type cycle_issuer;  // This MUST be the license authentication authority.
      account_id_type account;                  // The account to benefit the license.
      share_type amount;             // The license to be granted to the account.
      time_point_sec expiration;                // Deadline for denial.
      optional<float> account_frequency;        // Account frequency lock to be applied.

      extensions_type extensions;

      void validate() const;
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

  struct by_account;
  struct by_expiration;
  struct by_issuer_id;
  typedef multi_index_container<
    cycle_issue_request_object,
    indexed_by<
      ordered_unique< tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique< tag<by_account>,
        composite_key< cycle_issue_request_object,
          member< cycle_issue_request_object, account_id_type, &cycle_issue_request_object::account >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_unique< tag<by_expiration>,
        composite_key< cycle_issue_request_object,
          member< cycle_issue_request_object, time_point_sec, &cycle_issue_request_object::expiration >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_non_unique< tag<by_issuer_id>,
        member< cycle_issue_request_object, account_id_type, &cycle_issue_request_object::cycle_issuer >
      >
    >
  > cycle_issue_request_multi_index_type;

  typedef generic_index<cycle_issue_request_object, cycle_issue_request_multi_index_type> cycle_issue_request_index;

} }  // namespace graphene::chain

FC_REFLECT_DERIVED( graphene::chain::cycle_issue_request_object, (graphene::db::object),
                    (cycle_issuer)
                    (account)
                    (amount)
                    (expiration)
                    (extensions)
                  )
