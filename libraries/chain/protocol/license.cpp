/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/license.hpp>

namespace graphene { namespace chain {

  void create_license_type_operation::validate() const
  {

  }

  void issue_license_operation::validate() const
  {
    FC_ASSERT( frequency_lock >= 0 );  // NOTE: for charter licenses, must not be 0.
    FC_ASSERT( bonus_percentage > -100,
               "Illegal bonus percentage ${b}, value would lead to negative amount of cycles",
               ("b", bonus_percentage)
             );
  }

} } // namespace graphene::chain