/**
 * DASCOIN!
 */

#include <graphene/chain/license_objects.hpp>

namespace graphene { namespace chain {

  void license_type_object::validate() const
  {
    FC_ASSERT( name.size() >= GRAPHENE_MIN_ACCOUNT_NAME_LENGTH );
    FC_ASSERT( name.size() <= GRAPHENE_MAX_ACCOUNT_NAME_LENGTH );
  }

  void license_request_object::validate() const
  {

  }

  optional<license_type_id_type> license_information::active_license() const
  {
    if ( history.empty() )
      return {};
    return {history.back().first};
  }

  frequency_type license_information::active_frequency_lock() const
  {
    if ( history.empty() )
      return 0;
    return history.back().second;
  }

  void license_information::add_license(license_type_id_type license_id, frequency_type frequency_lock)
  {
    history.emplace_back(license_id, frequency_lock);
  }

} } // namespace graphene::chain
