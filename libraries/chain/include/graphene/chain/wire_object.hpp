/**
 * DASCOIN!
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {

  class wire_out_holder_object : public graphene::db::abstract_object<wire_out_holder_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_wire_out_holder_object_type;

      account_id_type account;
      asset asset_to_wire;

      extensions_type extensions;
      void validate() const;
  };

///////////////////////////////
// MULTI INDEX CONTAINERS:   //
///////////////////////////////

struct by_account;
typedef multi_index_container<
  wire_out_holder_object,
  indexed_by<
    ordered_unique< tag<by_id>,
      member<object, object_id_type, &object::id>
    >,
    ordered_unique< tag<by_account>,
      member<wire_out_holder_object, account_id_type, &wire_out_holder_object::account>
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
                    (asset_to_wire)
                    (extensions)
                  )
