/**
 * DASCOIN!
*/
#pragma once

#include <graphene/chain/database.hpp>
#include <graphene/chain/queue_objects.hpp>

#include <fc/optional.hpp>
#include <fc/string.hpp>

#include <vector>

namespace graphene {
namespace chain {

// Structs for serializing data:

struct acc_id_res {

    acc_id_res() = default;
    acc_id_res(account_id_type account_id)
        : account_id(account_id)
    {
    }

    account_id_type account_id;
};

struct cycle_agreement {

    cycle_agreement() = default;
    cycle_agreement(share_type cycles, frequency_type frequency_lock)
        : cycles(cycles)
        , frequency_lock(frequency_lock)
    {
    }

    share_type cycles = 0;
    frequency_type frequency_lock = 0;
};

struct acc_id_share_t_res : public acc_id_res {

    using result_t = optional<share_type>;

    acc_id_share_t_res() = default;
    acc_id_share_t_res(account_id_type account_id, result_t result)
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result = 0;
};

struct acc_id_cycle_agreement_res : public acc_id_res {

    using result_t = optional<cycle_agreement>;

    acc_id_cycle_agreement_res() = default;
    acc_id_cycle_agreement_res(account_id_type account_id, result_t result = {})
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result;
};

struct acc_id_vec_cycle_agreement_res : public acc_id_res {

    using result_t = optional<vector<cycle_agreement>>;

    acc_id_vec_cycle_agreement_res() = default;
    acc_id_vec_cycle_agreement_res(account_id_type id, result_t result = {})
        : acc_id_res(id)
        , result(result)
    {
    }

    result_t result;
};

struct sub_w_pos {

    using pos_t = uint32_t;

    sub_w_pos() = default;
    sub_w_pos(pos_t position, reward_queue_object submission)
        : position(position)
        , submission(submission)
    {
    }

    pos_t position;
    reward_queue_object submission;
};

struct acc_id_queue_subs_w_pos_res : public acc_id_res {

    using result_t = optional<vector<sub_w_pos>>;

    acc_id_queue_subs_w_pos_res() = default;
    acc_id_queue_subs_w_pos_res(account_id_type account_id, result_t result = {})
        : acc_id_res(account_id)
        , result(result)
    {
    }

    result_t result;
};

class database;
class global_property_object;
class reward_queue_object;

using fc::optional;
using fc::string;

class database_access_layer {
  public:
    explicit database_access_layer(const database& db)
        : _db(db){};
    ~database_access_layer() {}

    // Global objects:
    global_property_object get_global_properties() const;

    // Balances:
    acc_id_share_t_res get_free_cycle_balance(account_id_type id) const;
    acc_id_vec_cycle_agreement_res get_all_cycle_balances(account_id_type id) const;
    acc_id_share_t_res get_dascoin_balance(account_id_type id) const;

    vector<acc_id_share_t_res> get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const;
    vector<acc_id_vec_cycle_agreement_res> get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const;
    vector<acc_id_share_t_res> get_dascoin_balances_for_accounts(vector<account_id_type> ids) const;

    // License:
    vector<pair<string, license_type_id_type>> get_license_type_names_ids() const;
    vector<license_type_object> get_license_types() const;
    optional<license_type_object> get_license_type(string name) const;
    optional<license_type_object> get_license_type(license_type_id_type license_id) const;

    // Queue:
    uint32_t get_reward_queue_size() const;
    vector<reward_queue_object> get_reward_queue() const;
    acc_id_queue_subs_w_pos_res get_queue_submissions_with_pos(account_id_type account_id) const;
    vector<acc_id_queue_subs_w_pos_res> get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const;

  private:
    template <typename IndexType>
    uint32_t size() const
    {
        return _db.get_index_type<IndexType>().indices().size();
    }

    template <typename QueryType, typename IndexType, typename IndexBy>
    typename IndexType::object_type get(QueryType id) const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
        return *(idx.find(id));
    }

    template <typename QueryType, typename IndexType, typename IndexBy>
    optional<typename IndexType::object_type> get_opt(QueryType id) const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
        auto it = idx.find(id);
        if (it != idx.end())
            return {*it};
        return {};
    }

    template <typename IndexType, typename IndexBy>
    vector<typename IndexType::object_type> get_all() const
    {
        const auto& idx = _db.get_index_type<IndexType>().indices().get<IndexBy>();
        return vector<typename IndexType::object_type>(idx.begin(), idx.end());
    }

    const database& _db;
};
}
}  // namespace graphene::chain

FC_REFLECT(graphene::chain::cycle_agreement, (cycles)(frequency_lock))

FC_REFLECT(graphene::chain::acc_id_res, (account_id))
FC_REFLECT_DERIVED(graphene::chain::acc_id_share_t_res, (graphene::chain::acc_id_res), (result))
FC_REFLECT_DERIVED(graphene::chain::acc_id_cycle_agreement_res, (graphene::chain::acc_id_res), (result))
FC_REFLECT_DERIVED(graphene::chain::acc_id_vec_cycle_agreement_res, (graphene::chain::acc_id_res), (result))

FC_REFLECT(graphene::chain::sub_w_pos, (position)(submission))
FC_REFLECT_DERIVED(graphene::chain::acc_id_queue_subs_w_pos_res, (graphene::chain::acc_id_res), (result))
