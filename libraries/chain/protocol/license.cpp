/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/license.hpp>

namespace graphene { namespace chain {

  void license_type_create_operation::validate() const
  {

  }

  void license_type_edit_operation::validate() const
  {

  }

  void license_type_delete_operation::validate() const
  {

  }

  void license_request_operation::validate() const
  {
    FC_ASSERT( bonus_percentage > -100,
               "Illegal bonus percentage ${b}, value would lead to negative amount of cycles",
               ("b", bonus_percentage)
             );
  }

  void license_approve_operation::validate() const
  {

  }

} } // namespace graphene::chain