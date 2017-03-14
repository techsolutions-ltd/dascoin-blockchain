#include <graphene/chain/access_layer.hpp>

#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

global_property_object database_access_layer::get_global_properties() const
{
  return _db.get(global_property_id_type());
}

// Balances:
share_type database_access_layer::get_free_cycle_balance(account_id_type id) const
{
  return get<account_id_type, account_cycle_balance_index, by_account_id>(id).balance;
}

share_type database_access_layer::get_dascoin_balance(account_id_type id) const
{
  auto key = boost::make_tuple(id, _db.get_dascoin_asset_id());
  return get<decltype(key), account_balance_index, by_account_asset>(key).balance;
}

vector<cycle_agreement> database_access_layer::get_all_cycle_balances(account_id_type id) const
{
  vector<cycle_agreement> result;

  // First entry is for free cycle balances:
  result.emplace_back(get_free_cycle_balance(id), 0);

  // Rest of the entries are from the queue:
  const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
  const auto& account_idx = queue_multi_idx.get<by_account>();

  const auto& range = account_idx.equal_range(id);
  for ( auto it = range.first; it != range.second; ++it )
      result.emplace_back(it->amount, it->frequency);

  return result;
}

// TODO: refactor with template method.
vector<share_type> database_access_layer::get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
  vector<share_type> result;
  result.reserve(ids.size());
  for ( auto id : ids )
    result.emplace_back(get_free_cycle_balance(id));
  return result;
}

// TODO: refactor with template method.
vector<vector<cycle_agreement>> database_access_layer::get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
  vector<vector<cycle_agreement>> result;
  result.reserve(ids.size());
  for ( auto id : ids )
    result.emplace_back(get_all_cycle_balances(id));
  return result;
}

// TODO: refactor with template method.
vector<share_type> database_access_layer::get_dascoin_balances_for_accounts(vector<account_id_type> ids) const
{
  vector<share_type> result;
  result.reserve(ids.size());
  for ( auto id : ids )
    result.emplace_back(get_dascoin_balance(id));
  return result;
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

// TODO: refactor with template method.
vector<vector<pair<uint32_t, reward_queue_object>>> database_access_layer::get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const
{
  vector<vector<pair<uint32_t, reward_queue_object>>> result;
  result.reserve(ids.size());
  for ( auto id : ids)
    result.emplace_back(get_queue_submissions_with_pos(id));
  return result;
}


} }  // namespace graphene::chain