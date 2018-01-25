/**
 * DASCOIN!
 */

#include <graphene/chain/upgrade_type.hpp>

namespace graphene { namespace chain {

  share_type upgrade_type::operator() (share_type x)
  {
    if ( used < multipliers.size() )
    {
      x = x * multipliers[used++];
    }
    return x;
  }

  void upgrade_type::improve(const upgrade_type& new_upgrade)
  {
    if ( multipliers.size() < new_upgrade.multipliers.size() )
    {
      multipliers = new_upgrade.multipliers;
      max = static_cast<uint8_t>(new_upgrade.multipliers.size());
    }
  }

  bool upgrade_type::based_on(const upgrade_type& base_upgrade) const
  {
    return max == base_upgrade.max
      && std::equal(std::begin(multipliers), std::end(multipliers), std::begin(base_upgrade.multipliers));
  }

} }  // namespace graphene::chain
