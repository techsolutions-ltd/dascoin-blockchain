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
   * @class submit_reserve_cycles_to_queue_request_object
   * @brief A pending request to issue cycles to an account cycle balance.
   * @ingroup object
   *
   * This is an implementation detail.
   */
  class submit_reserve_cycles_to_queue_request_object : public graphene::db::abstract_object<submit_reserve_cycles_to_queue_request_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_submit_reserve_cycles_to_queue_request_object_type;

      account_id_type cycle_issuer;

      account_id_type account;
      share_type amount;
      frequency_type frequency_lock;
      time_point_sec expiration;

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
    submit_reserve_cycles_to_queue_request_object,
    indexed_by<
      ordered_unique< tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique< tag<by_account>,
        composite_key< submit_reserve_cycles_to_queue_request_object,
          member< submit_reserve_cycles_to_queue_request_object, account_id_type, &submit_reserve_cycles_to_queue_request_object::account >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_unique< tag<by_expiration>,
        composite_key< submit_reserve_cycles_to_queue_request_object,
          member< submit_reserve_cycles_to_queue_request_object, time_point_sec, &submit_reserve_cycles_to_queue_request_object::expiration >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_non_unique< tag<by_issuer_id>,
        member< submit_reserve_cycles_to_queue_request_object, account_id_type, &submit_reserve_cycles_to_queue_request_object::cycle_issuer >
      >
    >
  > cycle_issue_request_multi_index_type;

  typedef generic_index<submit_reserve_cycles_to_queue_request_object, cycle_issue_request_multi_index_type> submit_reserve_cycles_to_queue_request_index;

} }  // namespace graphene::chain

FC_REFLECT_DERIVED( graphene::chain::submit_reserve_cycles_to_queue_request_object, (graphene::db::object),
                    (cycle_issuer)
                    (account)
                    (amount)
                    (frequency_lock)
                    (expiration)
                    (extensions)
                  )
