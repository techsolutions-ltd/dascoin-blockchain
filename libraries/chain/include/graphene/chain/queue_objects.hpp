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

  //////////////////////////////
  // OBJECTS:                 //
  //////////////////////////////

  /**
   * @class reward_queue_object
   * @brief The object holding the reward queue submission of cycles.
   * @ingroup object
   *
   * Contains an amount of cycles submitted to the reward queue by an account.
   */
  class reward_queue_object : public graphene::db::abstract_object<reward_queue_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id = impl_reward_queue_object_type;

      string origin;  // Formed from dascoin_origin_kind.
      optional<license_type_id_type> license;  // Valid when origin is chartered.
      account_id_type account;
      share_type amount;
      frequency_type frequency;
      time_point_sec time;
      string comment;

      extensions_type extensions;

      void validate() const;
  };

  //////////////////////////////
  // MULTI INDEX CONTAINERS:  //
  //////////////////////////////

  struct by_account;
  struct by_time;
  typedef multi_index_container<
    reward_queue_object,
    indexed_by<
      ordered_unique< tag<by_id>,
        member<object, object_id_type, &object::id>
      >,
      ordered_unique< tag<by_time>,
        composite_key< reward_queue_object,
          member< reward_queue_object, time_point_sec, &reward_queue_object::time>,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_unique< tag<by_account>,
        composite_key< reward_queue_object,
          member< reward_queue_object, account_id_type, &reward_queue_object::account>,
          member< object, object_id_type, &object::id>
        >
      >
    >
  > reward_queue_multi_index_type;

  typedef generic_index<reward_queue_object, reward_queue_multi_index_type> reward_queue_index;

} }  // namespace graphene::chain

//////////////////////////////
// REFLECTIONS:             //
//////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::reward_queue_object, (graphene::db::object),
                    (origin)
                    (license)
                    (account)
                    (amount)
                    (frequency)
                    (time)
                    (extensions)
                  )
