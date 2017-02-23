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

  class license_information
  {
    public:
      struct license_history_record
      {
        license_type_id_type license;
        share_type amount;
        frequency_type frequency_lock;

        license_history_record() = default;
        license_history_record(license_type_id_type l, share_type a, frequency_type f) 
          : license(l), amount(a), frequency_lock(f) {}
      };

      struct pending_license_record
      {
        license_type_id_type license;
        license_request_id_type request;

        pending_license_record() = default;
        pending_license_record(license_type_id_type l, license_request_id_type r) : license(l), request(r) {}
      };

      optional<license_type_id_type> active_license() const;
      frequency_type active_frequency_lock() const;
      void add_license(license_type_id_type license_id, share_type amount, frequency_type frequency_lock);

      void set_pending(license_type_id_type license_id, license_request_id_type req)
      {
        pending = pending_license_record(license_id, req);
      }
      void clear_pending()
      {
        pending.reset();
      }

      vector<license_history_record> history;
      optional<pending_license_record> pending;

      upgrade_type balance_upgrade;
      upgrade_type requeue_upgrade;
      upgrade_type return_upgrade;
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

      extensions_type extensions;

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

  /**
   * @class license_request_object
   * @brief A pending request to grant a license to an account.
   * @ingroup object
   *
   * This is an implementation detail.
   */
  class license_request_object : public graphene::db::abstract_object<license_request_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_license_request_object_type;

      account_id_type license_issuer;  // Must be current license authority.

      account_id_type account;
      license_type_id_type license;
      share_type amount;
      frequency_type frequency_lock;
      time_point_sec expiration;

      extensions_type extensions;

      void validate() const;
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

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

  struct by_account;
  struct by_expiration;
  struct by_license_type_id;
  struct by_issuer_id;
  typedef multi_index_container<
    license_request_object,
    indexed_by<
      ordered_unique< tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique< tag<by_account>,
        composite_key< license_request_object,
          member< license_request_object, account_id_type, &license_request_object::account >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_unique< tag<by_expiration>,
        composite_key< license_request_object,
          member< license_request_object, time_point_sec, &license_request_object::expiration >,
          member< object, object_id_type, &object::id>
        >
      >,
      ordered_non_unique< tag<by_issuer_id>,
        member< license_request_object, account_id_type, &license_request_object::license_issuer >
      >,
      ordered_non_unique< tag<by_license_type_id>,
        member< license_request_object, license_type_id_type, &license_request_object::license >
      >
    >
  > license_request_multi_index_type;

  typedef generic_index<license_request_object, license_request_multi_index_type> license_request_index;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT( graphene::chain::license_information::license_history_record,
            (license)
            (amount)
            (frequency_lock)
          )
FC_REFLECT( graphene::chain::license_information::pending_license_record,
            (license)
            (request)
          )
FC_REFLECT( graphene::chain::license_information,
            (history)
            (pending)
            (balance_upgrade)
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
                    (extensions)
                  )

FC_REFLECT_DERIVED( graphene::chain::license_request_object, (graphene::db::object),
                    (license_issuer)
                    (account)
                    (license)
                    (amount)
                    (frequency_lock)
                    (expiration)
                    (extensions)
                  )
