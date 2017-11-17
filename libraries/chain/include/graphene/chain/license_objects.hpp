/**
 * DASCOIN!
 */

#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/upgrade_type.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  namespace detail {

    template<typename T>
    struct default_upgrade_policy
    {
      share_type get_amount_to_upgrade(const T& upgradeable) const
      {
        return upgradeable.amount;
      }
    };

    template<typename T>
    struct president_upgrade_policy
    {
      share_type get_amount_to_upgrade(const T& upgradeable) const
      {
        return upgradeable.base_amount + upgradeable.base_amount * upgradeable.bonus_percent / 100;
      }
    };
  }

  class license_information_object : public graphene::db::abstract_object<license_information_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_license_information_object_type;

      struct license_history_record
      {
        license_type_id_type license;
        share_type amount;
        share_type base_amount;
        share_type bonus_percent;
        frequency_type frequency_lock;
        time_point_sec activated_at;
        time_point_sec issued_on_blockchain;
        upgrade_type balance_upgrade;
        vector<pair<upgrade_event_id_type, time_point_sec>> upgrades;

        using license_upgrade_policy = detail::default_upgrade_policy<license_history_record>;
        using predident_license_upgrade_policy = detail::president_upgrade_policy<license_history_record>;

        using upgrade_policy = fc::static_variant<license_upgrade_policy, predident_license_upgrade_policy>;

        upgrade_policy up_policy;

        license_history_record() = default;
        explicit license_history_record(license_type_id_type license,
                                        share_type amount,
                                        share_type base_amount,
                                        share_type bonus_percent,
                                        frequency_type frequency_lock,
                                        time_point_sec activated_at,
                                        time_point_sec issued_on_blockchain,
                                        upgrade_type balance_upgrade,
                                        upgrade_policy up_policy)
            : license(license),
              amount(amount),
              base_amount(base_amount),
              bonus_percent(bonus_percent),
              frequency_lock(frequency_lock),
              activated_at(activated_at),
              issued_on_blockchain(issued_on_blockchain),
              balance_upgrade(balance_upgrade),
              up_policy(up_policy) {}

        share_type amount_to_upgrade()
        {
          struct policy_visitor
          {
            using result_type = void;

            explicit policy_visitor(license_history_record &record)
              : history_record(record) {}

            const license_history_record& history_record;
            share_type amount = 0;

            void operator()( const license_upgrade_policy& policy )
            {
              amount = policy.get_amount_to_upgrade(history_record);
            }

            void operator()( const predident_license_upgrade_policy& policy )
            {
              amount = policy.get_amount_to_upgrade(history_record);
            }
          } visitor(*this);

          up_policy.visit(visitor);

          return visitor.amount;
        }
      };
      typedef vector<license_history_record> array_t;

      account_id_type account;

      array_t history;
      license_type_id_type max_license;
      frequency_type frequency_lock;
      license_kind vault_license_kind;

      upgrade_type requeue_upgrade;
      upgrade_type return_upgrade;

      void add_license(license_type_id_type license_id, share_type amount, share_type base_amount,
                       share_type bonus_percentage, frequency_type f_lock,
                       time_point_sec activated_at,
                       time_point_sec issued_on_blockchain,
                       upgrade_type up_type,
                       license_history_record::upgrade_policy up_policy) {
        history.emplace_back(license_id, amount, base_amount, bonus_percentage, f_lock,
                             activated_at, issued_on_blockchain, up_type, up_policy);
        max_license = license_id;
        frequency_lock = f_lock;
      }

      void subtract_cycles(license_type_id_type license_type, const share_type& amount)
      {
        auto found = find_if(history.begin(), history.end(), [license_type](const license_history_record& record){
          return record.license == license_type;
        });
        if (found != history.end())
        {
          FC_ASSERT( found->amount >= amount, "Insufficient cycle balance" );
          found->amount -= amount;
        }
      }

      optional<license_history_record> get_license(const license_type_id_type& license_type) const
      {
        optional<license_history_record> license{};
        for (const auto& lic_history: history)
        {
          if (lic_history.license == license_type)
          {
            license = lic_history;
            break;
          }
        }
        return license;
      }
  };

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////

  /**
   * @class license_type_object
   * @brief The license type given to an account.
   * @ingroup object
   *
   * Contains a type of license that can be given to an account and the amount of cycles that the account
   * benefits from the license.
   */
  class license_type_object : public graphene::db::abstract_object<license_type_object>
  {
    public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = license_type_object_type;

      string name;                        // Name of the license.
      share_type amount = 0;              // The amount of cycles the license grants.

      license_kind kind = license_kind::regular;

      // All upgrades in the system:
      upgrade_type balance_upgrade;
      upgrade_type requeue_upgrade;
      upgrade_type return_upgrade;

      // The eur limit used parameter used for vault to wallet transfers in the system:
      share_type eur_limit;

      using license_upgrade_policy = license_information_object::license_history_record::license_upgrade_policy;
      using predident_license_upgrade_policy = license_information_object::license_history_record::predident_license_upgrade_policy;

      using upgrade_policy = license_information_object::license_history_record::upgrade_policy;

      // Upgrade policy for this license:
      upgrade_policy up_policy;

      extensions_type extensions;

      license_type_object() = default;
      explicit license_type_object(string name, share_type amount, license_kind kind, upgrade_type balance_upgrade,
                                   upgrade_type requeue_upgrade, upgrade_type return_upgrade,
                                   share_type eur_limit,
                                   upgrade_policy up_policy)
          : name(name), amount(amount), kind(kind), balance_upgrade(balance_upgrade), requeue_upgrade(requeue_upgrade), 
            return_upgrade(return_upgrade), eur_limit(eur_limit), up_policy(up_policy) {}

      void validate() const;
  };

  inline bool operator == ( const license_type_object& lhs, const license_type_object& rhs )
  {
    return lhs.name == rhs.name;
  }
  inline bool operator < ( const license_type_object& lhs, const license_type_object& rhs )
  {
    return lhs.amount < rhs.amount;
  }
  inline bool operator <= ( const license_type_object& lhs, const license_type_object& rhs )
  {
    return lhs.amount <= rhs.amount;
  }
  inline bool operator > ( const license_type_object& lhs, const license_type_object& rhs )
  {
    return lhs.amount > rhs.amount;
  }
  inline bool operator >= ( const license_type_object& lhs, const license_type_object& rhs )
  {
    return lhs.amount >= rhs.amount;
  }

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

  struct by_account_id;
  typedef multi_index_container<
    license_information_object,
    indexed_by<
      ordered_unique< 
        tag<by_id>,
        member<object, object_id_type, &object::id> 
      >,
      ordered_non_unique< 
        tag<by_account_id>,
          composite_key< license_information_object,
              member< license_information_object, account_id_type, &license_information_object::account >,
              member< object, object_id_type, &object::id >
          >
      >
    >
  > license_information_multi_index_type;

  typedef generic_index<license_information_object, license_information_multi_index_type> license_information_index;

  struct by_name;
  struct by_amount;
  typedef multi_index_container<
     license_type_object,
     indexed_by<
        ordered_unique< tag<by_id>,
          member<object, object_id_type, &object::id> >,
        ordered_unique< tag<by_name>,
          member<license_type_object, string, &license_type_object::name>
        >,
        ordered_non_unique< tag<by_amount>,
          member<license_type_object, share_type, &license_type_object::amount>
        >
     >
  > license_type_multi_index_type;

  typedef generic_index<license_type_object, license_type_multi_index_type> license_type_index;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::license_information_object::license_history_record,
            (license)
            (amount)
            (base_amount)
            (bonus_percent)
            (frequency_lock)
            (activated_at)
            (issued_on_blockchain)
            (balance_upgrade)
            (upgrades)
          )

FC_REFLECT_DERIVED( graphene::chain::license_information_object, (graphene::db::object),
            (account)
            (history)
            (max_license)
            (frequency_lock)
            (vault_license_kind)
            (requeue_upgrade)
            (return_upgrade)
          )

FC_REFLECT_DERIVED( graphene::chain::license_type_object, (graphene::db::object),
                    (name)
                    (amount)
                    (kind)
                    (balance_upgrade)
                    (requeue_upgrade)
                    (return_upgrade)
                    (eur_limit)
                    (extensions)
                  )
