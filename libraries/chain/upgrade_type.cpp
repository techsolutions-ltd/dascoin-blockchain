/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
