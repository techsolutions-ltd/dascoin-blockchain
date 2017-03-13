#include <graphene/chain/access_layer.hpp>

#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

global_property_object database_access_layer::get_global_properties() const
{
  return _db.get(global_property_id_type());
}

// License:
optional<license_type_object> database_access_layer::get_license_type(string name) const
{
  return get_opt<string, license_type_index, by_name>(name);
}

vector<license_type_object> database_access_layer::get_license_types() const
{
  return get_all<license_type_index, by_id>();
}

uint32_t database_access_layer::get_reward_queue_size() const
{
  return size<reward_queue_index>();
}

vector<pair<uint32_t, reward_queue_object>> database_access_layer::get_queue_submissions_with_pos(account_id_type account_id) const
{
  vector<pair<uint32_t, reward_queue_object>> result;

  const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
  const auto& account_idx = queue_multi_idx.get<by_account>();
  const auto& time_idx = queue_multi_idx.get<by_time>();

  const auto& range = account_idx.equal_range(account_id);
  for ( auto it = range.first; it != range.second; ++it )
  {
      uint32_t pos = distance(time_idx.begin(), queue_multi_idx.project<by_time>(it));
      result.emplace_back(pos, *it);
  }

  return result;
}

} }  // namespace graphene::chain