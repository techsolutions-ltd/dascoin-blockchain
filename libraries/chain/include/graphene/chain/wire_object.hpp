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

  class wire_out_holder_object : public graphene::db::abstract_object<wire_out_holder_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_wire_out_holder_object_type;

      account_id_type account;
      share_type amount;
      asset_id_type asset_id;
      string memo;
      time_point_sec timestamp;

      extensions_type extensions;

      void set_balance(asset a) { amount = a.amount; asset_id = a.asset_id; }
      void validate() const;
  };

///////////////////////////////
// MULTI INDEX CONTAINERS:   //
///////////////////////////////

struct by_account_asset;
typedef multi_index_container<
  wire_out_holder_object,
  indexed_by<
    ordered_unique< tag<by_id>,
      member<object, object_id_type, &object::id>
    >,
    ordered_non_unique< tag<by_account_asset>,
       composite_key<
          wire_out_holder_object,
          member<wire_out_holder_object, account_id_type, &wire_out_holder_object::account>,
          member<wire_out_holder_object, asset_id_type, &wire_out_holder_object::asset_id>
       >
    >
  >
> wire_out_holder_multi_index_type;

typedef generic_index<wire_out_holder_object, wire_out_holder_multi_index_type> wire_out_holder_index;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::wire_out_holder_object, (graphene::db::object),
                    (account)
                    (amount)
                    (asset_id)
                    (memo)
                    (timestamp)
                    (extensions)
                  )
