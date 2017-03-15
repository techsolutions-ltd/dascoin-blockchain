/**
 * DASCOIN!
*/
#pragma once

#include <graphene/chain/database.hpp>

#include <fc/optional.hpp>
#include <fc/string.hpp>

#include <vector>

namespace graphene { namespace chain {

class database;
class global_property_object;
class reward_queue_object;

using fc::optional;
using fc::string;

class database_access_layer
{
public:

  explicit database_access_layer(const database& db) : _db(db) {};
  ~database_access_layer() {}

  // Global objects:
  global_property_object get_global_properties() const;

  // Balances:
  share_type get_free_cycle_balance(account_id_type id) const;
  vector<cycle_agreement> get_all_cycle_balances(account_id_type id) const;
  share_type get_dascoin_balance(account_id_type id) const;

  vector<share_type> get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const;
  vector<vector<cycle_agreement>> get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const;
  vector<share_type> get_dascoin_balances_for_accounts(vector<account_id_type> ids) const;

  // License:
  vector<license_type_object> get_license_types() const;
  optional<license_type_object> get_license_type(string name) const;

  // Queue:
  uint32_t get_reward_queue_size() const;
  vector<pair<uint32_t, reward_queue_object>> get_queue_submissions_with_pos(account_id_type account_id) const;
  vector<vector<pair<uint32_t, reward_queue_object>>> get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const;

  // TODO: template methods should be private.
  template<typename IndexType>
  uint32_t size() const
  {
    return _db.get_index_type<IndexType>().indices().size();
  }

  template<typename QueryType, typename IndexType, typename IndexBy>
  typename IndexType::object_type get(QueryType id) const
  {
    const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
    return *(idx.find(id));
  }

  template<typename QueryType, typename IndexType, typename IndexBy>
  optional<typename IndexType::object_type> get_opt(QueryType id) const
  {
    const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
    auto it = idx.find(id);
    if ( it != idx.end() ) 
      return {*it};
    return {};
  }

  template<typename IndexType, typename IndexBy>
  vector<typename IndexType::object_type> get_all() const
  {
    const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
    return vector<typename IndexType::object_type>(idx.begin(), idx.end());
  }

private:
  const database& _db;
};

} }  // namespace graphene::chain
