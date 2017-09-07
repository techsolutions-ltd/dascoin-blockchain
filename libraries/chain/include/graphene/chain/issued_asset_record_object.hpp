/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene {
namespace chain {

  using graphene::db::abstract_object;

  class issued_asset_record_object :  public abstract_object<issued_asset_record_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_issued_asset_record_object_type;

      string unique_id;

      account_id_type issuer;
      account_id_type receiver;

      asset_id_type asset_type;
      share_type amount;
      share_type reserved;

      issued_asset_record_object() = default;
      explicit issued_asset_record_object(const string& unique_id, account_id_type issuer,
                                          account_id_type receiver, asset_id_type asset_type,
                                          share_type amount, share_type reserved)
               : unique_id(unique_id),
                 issuer(issuer),
                 receiver(receiver),
                 asset_type(asset_type),
                 amount(amount),
                 reserved(reserved) {}

      void validate() const;
  };

  struct by_unique_id_asset;
  struct by_receiver_asset;
  typedef multi_index_container<
    issued_asset_record_object,
    indexed_by<
      ordered_unique< 
        tag<by_id>,
        member<object, object_id_type, &object::id> 
      >,
      ordered_unique<
        tag<by_unique_id_asset>,
        composite_key<
          issued_asset_record_object,
          member<issued_asset_record_object, string, &issued_asset_record_object::unique_id>,
          member<issued_asset_record_object, asset_id_type, &issued_asset_record_object::asset_type>
        >
      >,
      ordered_non_unique< 
        tag<by_receiver_asset>,
        composite_key<
          issued_asset_record_object,
          member<issued_asset_record_object, account_id_type, &issued_asset_record_object::receiver>,
          member<issued_asset_record_object, asset_id_type, &issued_asset_record_object::asset_type>
        >
      >
    >
  > issued_asset_record_multi_index_type;

  typedef generic_index<issued_asset_record_object, issued_asset_record_multi_index_type> issued_asset_record_index;

}  // namespace chain
}  // namespace graphene

FC_REFLECT( graphene::chain::issued_asset_record_object,
            (unique_id)
            (issuer)
            (receiver)
            (asset_type)
            (amount)
            (reserved)
          )