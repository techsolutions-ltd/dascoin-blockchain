#include <graphene/chain/access_layer.hpp>

#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/issued_asset_record_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene {
namespace chain {

global_property_object database_access_layer::get_global_properties() const
{
    return _db.get(global_property_id_type());
}

// Balances:
acc_id_share_t_res database_access_layer::get_free_cycle_balance(account_id_type id) const
{
    auto cycle_balance_obj = get_opt<account_id_type, account_cycle_balance_index, by_account_id>(id);

    optional<share_type> opt_balance;
    if (cycle_balance_obj.valid())
        opt_balance = cycle_balance_obj->balance;

    return {id, opt_balance};
}

acc_id_share_t_res database_access_layer::get_dascoin_balance(account_id_type id) const
{
    auto key = boost::make_tuple(id, _db.get_dascoin_asset_id());
    auto balance_obj = get_opt<decltype(key), account_balance_index, by_account_asset>(key);

    optional<share_type> opt_balance;
    if (balance_obj.valid())
        opt_balance = balance_obj->balance;

    return {id, opt_balance};
}

acc_id_vec_cycle_agreement_res database_access_layer::get_all_cycle_balances(account_id_type id) const
{
    if (!get_opt<account_id_type, account_index, by_id>(id).valid())
        // TODO: ugly, figure out a way to use braces.
        return {id};  // Account with said id does not exist, return empty optional.

    vector<cycle_agreement> result;
    // First entry is for free cycle balances:
    auto cycle_balance_obj = get<account_id_type, account_cycle_balance_index, by_account_id>(id);
    result.emplace_back(cycle_balance_obj.balance, 0);

    // Rest of the entries are from the queue:
    const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
    const auto& account_idx = queue_multi_idx.get<by_account>();

    const auto& range = account_idx.equal_range(id);
    for (auto it = range.first; it != range.second; ++it)
        result.emplace_back(it->amount, it->frequency);

    return {id, {result}};
}

vector<acc_id_share_t_res>
    database_access_layer::get_free_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_share_t_res>(ids, std::bind(&database_access_layer::get_free_cycle_balance,
                                                          this, std::placeholders::_1));
}

vector<acc_id_vec_cycle_agreement_res>
    database_access_layer::get_all_cycle_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_vec_cycle_agreement_res>(ids, std::bind(&database_access_layer::get_all_cycle_balances,
                                                                       this, std::placeholders::_1));
}

vector<acc_id_share_t_res> database_access_layer::get_dascoin_balances_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_share_t_res>(ids, std::bind(&database_access_layer::get_dascoin_balance,
                                                          this, std::placeholders::_1));
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

optional<license_type_object> database_access_layer::get_license_type(license_type_id_type license_id) const
{
  return get_opt<license_type_id_type, license_type_index, by_id>(license_id);
}

vector<pair<string, license_type_id_type>> database_access_layer::get_license_type_names_ids() const
{
    vector<pair<string, license_type_id_type>> result;
    for (const auto& lic : get_license_types())
        result.emplace_back(lic.name, lic.id);
    return result;
}

uint32_t database_access_layer::get_reward_queue_size() const { return size<reward_queue_index>(); }

vector<reward_queue_object> database_access_layer::get_reward_queue() const
{
    return get_all<reward_queue_index, by_time>();
}

vector<reward_queue_object> database_access_layer::get_reward_queue_by_page(uint32_t from, uint32_t amount) const
{
    return get_range<reward_queue_index, by_time>(from, amount);
}

vector<frequency_history_record_object> database_access_layer::get_frequency_history() const
{
    return get_all<frequency_history_record_index, by_time>();
}

vector<frequency_history_record_object> database_access_layer::get_frequency_history_by_page(uint32_t from, uint32_t amount) const
{
    return get_range<frequency_history_record_index, by_time>(from, amount);
}

acc_id_queue_subs_w_pos_res database_access_layer::get_queue_submissions_with_pos(account_id_type account_id) const
{
    if (!get_opt<account_id_type, account_index, by_id>(account_id).valid())
        return {account_id};  // Account does not exist, return null result.

    vector<sub_w_pos> result;

    const auto& queue_multi_idx = _db.get_index_type<reward_queue_index>().indices();
    const auto& account_idx = queue_multi_idx.get<by_account>();
    const auto& time_idx = queue_multi_idx.get<by_time>();

    const auto& range = account_idx.equal_range(account_id);
    for (auto it = range.first; it != range.second; ++it) {
        uint32_t pos = distance(time_idx.begin(), queue_multi_idx.project<by_time>(it));
        result.emplace_back(pos, *it);
    }

    return {account_id, {result}};
}

vector<acc_id_queue_subs_w_pos_res>
    database_access_layer::get_queue_submissions_with_pos_for_accounts(vector<account_id_type> ids) const
{
    return get_balance<acc_id_queue_subs_w_pos_res>(ids, std::bind(&database_access_layer::get_queue_submissions_with_pos,
                                                                   this, std::placeholders::_1));
}

optional<vault_info_res> database_access_layer::get_vault_info(account_id_type vault_id) const
{
    const auto& account = get_opt<account_id_type, account_index, by_id>(vault_id);

    // TODO: re-evaluate this, should we throw an error here?
    if (!account.valid() || !account->is_vault())
        return {};

    const auto& webeur_balance = _db.get_balance_object(vault_id, _db.get_web_asset_id());
    const auto& dascoin_balance = _db.get_balance_object(vault_id, _db.get_dascoin_asset_id());
    const auto& free_cycle_balance = _db.get_cycle_balance(vault_id);
    const auto& license_information = _db.get_license_information(vault_id);
    const auto& eur_limit = _db.get_eur_limit(license_information);

    return vault_info_res{webeur_balance.balance,
                          webeur_balance.reserved,
                          dascoin_balance.balance,
                          free_cycle_balance,
                          dascoin_balance.limit,
                          eur_limit,
                          dascoin_balance.spent,
                          license_information};
}

vector<acc_id_vault_info_res> database_access_layer::get_vaults_info(vector<account_id_type> vault_ids) const
{
    return get_balance<acc_id_vault_info_res>(vault_ids, [this](account_id_type account_id) -> acc_id_vault_info_res {
        return acc_id_vault_info_res{account_id, this->get_vault_info(account_id)};
    });
}

optional<asset_object> database_access_layer::lookup_asset_symbol(const string& symbol_or_id) const
{
    return get_asset_symbol(_db.get_index_type<asset_index>(), symbol_or_id);
}

vector<optional<asset_object>> database_access_layer::lookup_asset_symbols(const vector<string>& symbols_or_ids) const
{
    const auto& assets_by_symbol = _db.get_index_type<asset_index>();
    vector<optional<asset_object>> result;
    result.reserve(symbols_or_ids.size());
    std::transform(symbols_or_ids.begin(), symbols_or_ids.end(), std::back_inserter(result),
                   std::bind(&database_access_layer::get_asset_symbol, this, std::ref(assets_by_symbol), std::placeholders::_1));

    return result;
}

bool database_access_layer::check_issued_asset(const string& unique_id, const string& asset) const
{
    const auto res = lookup_asset_symbol(asset);
    if ( res.valid() )
    {
        const auto record = get_issued_asset_record(unique_id, res->id);
        return record.valid();
    }
    return false;
}

bool database_access_layer::check_issued_webeur(const string& unique_id) const
{
    const auto web_id = _db.get_web_asset_id();
    return get_issued_asset_record(unique_id, web_id).valid();
}

optional<asset_object> database_access_layer::get_asset_symbol(const asset_index &index, const string& symbol_or_id) const
{
    const auto& asset_by_symbol = index.indices().get<by_symbol>();
    if( !symbol_or_id.empty() && std::isdigit(symbol_or_id[0]) )
    {
        auto ptr = _db.find(variant(symbol_or_id).as<asset_id_type>());
        return ptr == nullptr ? optional<asset_object>{} : *ptr;
    }
    auto itr = asset_by_symbol.find(symbol_or_id);
    return itr == asset_by_symbol.end() ? optional<asset_object>{} : *itr;
}

// TODO:
optional<issued_asset_record_object>
database_access_layer::get_issued_asset_record(const string& unique_id, asset_id_type asset_id) const
{
    const auto& idx = _db.get_index_type<issued_asset_record_index>().indices().get<by_unique_id_asset>();
    auto it = idx.find(boost::make_tuple(unique_id, asset_id));
    if (it != idx.end())
        return {*it};
    return {};
}

}  // namespace chain
}  // namespace graphene
