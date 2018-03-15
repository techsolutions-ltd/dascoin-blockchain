/**
 * DASCOIN!
 */
#pragma once

#include <fc/noncopyable.hpp>

namespace graphene { namespace chain { namespace detail {

  class submit_cycles_evaluator_helper : fc::noncopyable
  {
  public:
    explicit submit_cycles_evaluator_helper(const database& db)
      : _db{db}{}

    template<typename OperationType>
    const license_information_object* do_evaluate(const OperationType& op, const license_type_id_type &license_type,
                                                  frequency_type frequency) const
    {
      const auto& account_obj = op.account(_db);

      // Only vault accounts are allowed to submit cycles:
      FC_ASSERT( account_obj.is_vault(),
                 "Account '${n}' is not a vault account",
                 ("n", account_obj.name)
               );

      FC_ASSERT( account_obj.license_information.valid(),
                 "Cannot submit cycles, account '${n}' does not have any licenses",
                 ("n", account_obj.name)
               );

      const auto& license_information_obj = (*account_obj.license_information)(_db);

      // Check if this account has a license of required type:
      optional<license_information_object::license_history_record> license{license_information_obj.get_license(license_type)};

      FC_ASSERT( license.valid(), "Account '${n}' does not have a license of type ${l}",
                 ("n", account_obj.name)
                 ("l", license_type)
               );

      // Assure we have enough funds to submit:
      FC_ASSERT( license->total_cycles() >= op.amount,
                 "Cannot submit ${am} cycles, account '${n}' license cycle balance is ${b}",
                 ("am", op.amount)
                 ("n", account_obj.name)
                 ("b", license->amount)
               );

      // Assure frequency submitted is the same as in the license:
      FC_ASSERT( license->frequency_lock == frequency,
                 "Cannot submit ${am} cycles, frequency set (${f}) is not equal to license's frequency (${lf})",
                 ("am", op.amount)
                 ("f", frequency)
                 ("lf", license->frequency_lock)
               );

      // Assure that amount of cycles submitted would not exceed DASCOIN_MAX_DASCOIN_SUPPLY limit.
      FC_ASSERT(_db.cycles_to_dascoin(op.amount, frequency) + _db.get_total_dascoin_amount_in_system() <= (DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION),
                "Cannot submit ${am} cycles, on frequency (${f}), cause that amount of dascoins(${dsc}), would exceed DASCOIN_MAX_DASCOIN_SUPPLY limit",
                ("am", op.amount)
                ("f", frequency)
                ("dsc", _db.cycles_to_dascoin(op.amount, frequency))
               );

	    return &license_information_obj;
    }

    template<typename OperationType>
    object_id_type do_apply(const OperationType& op, const license_information_object* lio,
                            const license_type_id_type &license_type, frequency_type frequency)
    {
      auto& d = const_cast<database &>(_db);

      // Spend cycles, decrease balance and supply:
      d.reserve_cycles(op.account, op.amount);
      auto origin = fc::reflector<dascoin_origin_kind>::to_string(user_submit);
      d.modify(*lio, [&](license_information_object& lio){
        lio.subtract_cycles(license_type, op.amount);
      });

      return d.push_queue_submission(origin, license_type, op.account, op.amount, frequency, op.comment);
    }

  private:
    const database& _db;
  };

} } }  // namespace graphene::chain::detail
