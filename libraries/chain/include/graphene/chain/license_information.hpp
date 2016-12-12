/**
 * DASCOIN!
 */

#pragma once
namespace graphene { namespace chain {

  class license_information
  {
    public:
      typedef pair<license_type_id_type, frequency_type> license_history_record;

      optional<license_type_id_type> active_license();
      optional<license_type_id_type> active_frequency_lock();

      vector<license_history_record> history;
      upgrade_type balance_upgrade;
      upgrade_type requeue_upgrade;
      upgrade_type balance_upgrade;
  };
