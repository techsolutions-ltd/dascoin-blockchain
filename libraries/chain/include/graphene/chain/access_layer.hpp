/**
 * DASCOIN!
*/
#pragma once

#include <graphene/chain/database.hpp>

#include <fc/optional.hpp>

#include <vector>

namespace graphene { namespace chain {

class database;
class global_property_object;
class reward_queue_object;

using fc::optional;

class database_access_layer
{
public:

  explicit database_access_layer(const database& db) : _db(db) {};
  ~database_access_layer() {}

  template<typename IndexType>
  uint32_t size() const
  {
    return _db.get_index_type<IndexType>().indices().size();
  }

  template<typename IdType, typename IndexType, typename IndexBy>
  optional<typename IndexType::object_type> get_opt(IdType id) const
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

  // Global objects:
  global_property_object get_global_properties() const;

  // Queue:
  uint32_t get_reward_queue_size() const;
  vector<pair<uint32_t, reward_queue_object>> get_queue_submissions_with_pos(account_id_type account_id) const;

private:
  const database& _db;
};

} }  // namespace graphene::chain