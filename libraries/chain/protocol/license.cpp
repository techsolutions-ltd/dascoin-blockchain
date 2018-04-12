/**
 * DASCOIN!
 */

#include <graphene/chain/protocol/license.hpp>

namespace graphene { namespace chain {

  void create_license_type_operation::validate() const
  {

  }

  void edit_license_type_operation::validate() const
  {
    if (name.valid())
    {
      FC_ASSERT( name->size() <= DASCOIN_MAX_LICENSE_NAME_LEN, "License's name cannot be longer than ${len}", ("len", DASCOIN_MAX_LICENSE_NAME_LEN) );
      FC_ASSERT( name->size() > 0, "Cannot set license's name to an empty string" );
    }
    if (amount.valid())
    {
      FC_ASSERT( amount->value > 0, "Cannot set cycle amount to negative or zero number ${amount}", ("amount", *amount) );
    }
    if (eur_limit.valid())
    {
      FC_ASSERT( eur_limit->value > 0, "Cannot set eur limit to negative or zero number ${amount}", ("amount", *eur_limit) );
    }
  }

  void issue_license_operation::validate() const
  {
    FC_ASSERT( frequency_lock >= 0 );  // NOTE: for charter licenses, must not be 0.
    FC_ASSERT( bonus_percentage > -100,
               "Illegal bonus percentage ${b}, value would lead to negative amount of cycles",
               ("b", bonus_percentage)
             );
  }

  void update_license_operation::validate() const
  {

  }

} } // namespace graphene::chain
