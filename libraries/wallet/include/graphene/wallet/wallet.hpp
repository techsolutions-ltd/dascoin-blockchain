/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/app/api.hpp>
#include <graphene/utilities/key_conversion.hpp>

using namespace graphene::app;
using namespace graphene::chain;
using namespace graphene::utilities;
using namespace std;

namespace fc
{
   void to_variant( const account_multi_index_type& accts, variant& vo, uint32_t max_depth );
   void from_variant( const variant &var, account_multi_index_type &vo, uint32_t max_depth );
}

namespace graphene { namespace wallet {

typedef uint16_t transaction_handle_type;

/**
 * This class takes a variant and turns it into an object
 * of the given type, with the new operator.
 */

object* create_object( const variant& v );

struct plain_keys
{
   map<public_key_type, string>  keys;
   fc::sha512                    checksum;
};

struct brain_key_info
{
   string brain_priv_key;
   string wif_priv_key;
   public_key_type pub_key;
};


/**
 *  Contains the confirmation receipt the sender must give the receiver and
 *  the meta data about the receipt that helps the sender identify which receipt is
 *  for the receiver and which is for the change address.
 */
struct blind_confirmation
{
   struct output
   {
      string                          label;
      public_key_type                 pub_key;
      stealth_confirmation::memo_data decrypted_memo;
      stealth_confirmation            confirmation;
      authority                       auth;
      string                          confirmation_receipt;
   };

   signed_transaction     trx;
   vector<output>         outputs;
};

struct blind_balance
{
   asset                     amount;
   public_key_type           from; ///< the account this balance came from
   public_key_type           to; ///< the account this balance is logically associated with
   public_key_type           one_time_key; ///< used to derive the authority key and blinding factor
   fc::sha256                blinding_factor;
   fc::ecc::commitment_type  commitment;
   bool                      used = false;
};

struct blind_receipt
{
   std::pair<public_key_type,fc::time_point>        from_date()const { return std::make_pair(from_key,date); }
   std::pair<public_key_type,fc::time_point>        to_date()const   { return std::make_pair(to_key,date);   }
   std::tuple<public_key_type,asset_id_type,bool>   to_asset_used()const   { return std::make_tuple(to_key,amount.asset_id,used);   }
   const commitment_type& commitment()const        { return data.commitment; }

   fc::time_point                  date;
   public_key_type                 from_key;
   string                          from_label;
   public_key_type                 to_key;
   string                          to_label;
   asset                           amount;
   string                          memo;
   authority                       control_authority;
   stealth_confirmation::memo_data data;
   bool                            used = false;
   stealth_confirmation            conf;
};

struct by_from;
struct by_to;
struct by_to_asset_used;
struct by_commitment;

typedef multi_index_container< blind_receipt,
   indexed_by<
      ordered_unique< tag<by_commitment>, const_mem_fun< blind_receipt, const commitment_type&, &blind_receipt::commitment > >,
      ordered_unique< tag<by_to>, const_mem_fun< blind_receipt, std::pair<public_key_type,fc::time_point>, &blind_receipt::to_date > >,
      ordered_non_unique< tag<by_to_asset_used>, const_mem_fun< blind_receipt, std::tuple<public_key_type,asset_id_type,bool>, &blind_receipt::to_asset_used > >,
      ordered_unique< tag<by_from>, const_mem_fun< blind_receipt, std::pair<public_key_type,fc::time_point>, &blind_receipt::from_date > >
   >
> blind_receipt_index_type;


struct key_label
{
   string          label;
   public_key_type key;
};


struct by_label;
struct by_key;
typedef multi_index_container<
   key_label,
   indexed_by<
      ordered_unique< tag<by_label>, member< key_label, string, &key_label::label > >,
      ordered_unique< tag<by_key>, member< key_label, public_key_type, &key_label::key > >
   >
> key_label_index_type;


struct wallet_data
{
   /** Chain ID this wallet is used with */
   chain_id_type chain_id;
   account_multi_index_type my_accounts;
   /// @return IDs of all accounts in @ref my_accounts
   vector<object_id_type> my_account_ids()const
   {
      vector<object_id_type> ids;
      ids.reserve(my_accounts.size());
      std::transform(my_accounts.begin(), my_accounts.end(), std::back_inserter(ids),
                     [](const account_object& ao) { return ao.id; });
      return ids;
   }
   /// Add acct to @ref my_accounts, or update it if it is already in @ref my_accounts
   /// @return true if the account was newly inserted; false if it was only updated
   bool update_account(const account_object& acct)
   {
      auto& idx = my_accounts.get<by_id>();
      auto itr = idx.find(acct.get_id());
      if( itr != idx.end() )
      {
         idx.replace(itr, acct);
         return false;
      } else {
         idx.insert(acct);
         return true;
      }
   }

   /** encrypted keys */
   vector<char>              cipher_keys;

   /** map an account to a set of extra keys that have been imported for that account */
   map<account_id_type, set<public_key_type> >  extra_keys;

   // map of account_name -> base58_private_key for
   //    incomplete account regs
   map<string, vector<string> > pending_account_registrations;
   map<string, string> pending_witness_registrations;

   key_label_index_type                                              labeled_keys;
   blind_receipt_index_type                                          blind_receipts;

   string                    ws_server = "ws://localhost:8090";
   string                    ws_user;
   string                    ws_password;
};

struct exported_account_keys
{
    string account_name;
    vector<vector<char>> encrypted_private_keys;
    vector<public_key_type> public_keys;
};

struct exported_keys
{
    fc::sha512 password_checksum;
    vector<exported_account_keys> account_keys;
};

struct approval_delta
{
   vector<string> active_approvals_to_add;
   vector<string> active_approvals_to_remove;
   vector<string> owner_approvals_to_add;
   vector<string> owner_approvals_to_remove;
   vector<string> key_approvals_to_add;
   vector<string> key_approvals_to_remove;
};

struct worker_vote_delta
{
   flat_set<worker_id_type> vote_for;
   flat_set<worker_id_type> vote_against;
   flat_set<worker_id_type> vote_abstain;
};

struct signed_block_with_info : public signed_block
{
   signed_block_with_info( const signed_block& block );
   signed_block_with_info( const signed_block_with_info& block ) = default;

   block_id_type block_id;
   public_key_type signing_key;
   vector< transaction_id_type > transaction_ids;
};

struct vesting_balance_object_with_info : public vesting_balance_object
{
   vesting_balance_object_with_info( const vesting_balance_object& vbo, fc::time_point_sec now );
   vesting_balance_object_with_info( const vesting_balance_object_with_info& vbo ) = default;

   /**
    * How much is allowed to be withdrawn.
    */
   asset allowed_withdraw;

   /**
    * The time at which allowed_withdrawal was calculated.
    */
   fc::time_point_sec allowed_withdraw_time;
};

namespace detail {
class wallet_api_impl;
}

/***
 * A utility class for performing various state-less actions that are related to wallets
 */
    class utility {
    public:
      /**
       * Derive any number of *possible* owner keys from a given brain key.
       *
       * NOTE: These keys may or may not match with the owner keys of any account.
       * This function is merely intended to assist with account or key recovery.
       *
       * @see suggest_brain_key()
       *
       * @param brain_key    Brain key
       * @param number_of_desired_keys  Number of desired keys
       * @return A list of keys that are deterministically derived from the brainkey
       */
      static vector<brain_key_info> derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys = 1);

      /** Suggests a safe brain key to use for creating your account.
       * \c create_account_with_brain_key() requires you to specify a 'brain key',
       * a long passphrase that provides enough entropy to generate cyrptographic
       * keys.  This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * @returns a suggested brain_key
       */
      static brain_key_info suggest_brain_key();
    };

struct operation_detail {
   string                   memo;
   string                   description;
   operation_history_object op;
};

/**
 * This wallet assumes it is connected to the database server with a high-bandwidth, low-latency connection and
 * performs minimal caching. This API could be provided locally to be used by a web interface.
 */
class wallet_api
{
   public:
      wallet_api( const wallet_data& initial_data, fc::api<login_api> rapi );
      virtual ~wallet_api();

      bool copy_wallet_file( string destination_filename );

      fc::ecc::private_key derive_private_key(const std::string& prefix_string, int sequence_number) const;

      variant                           info();
      /** Returns info such as client version, git version of graphene/fc, version of boost, openssl.
       * @returns compile time info and client and dependencies versions
       */
      variant_object                    about() const;
      optional<signed_block_with_info>    get_block( uint32_t num );
      /** Returns the number of accounts registered on the blockchain
       * @returns the number of registered accounts
       */
      uint64_t                          get_account_count()const;
      /** Lists all accounts controlled by this wallet.
       * This returns a list of the full account objects for all accounts whose private keys
       * we possess.
       * @returns a list of account objects
       */
      vector<account_object>            list_my_accounts();
      /** Lists all accounts registered in the blockchain.
       * This returns a list of all account names and their account ids, sorted by account name.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all accounts,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last account name returned as the \c lowerbound for the next \c list_accounts() call.
       *
       * @param lowerbound the name of the first account to return.  If the named account does not exist,
       *                   the list will start at the account that comes after \c lowerbound
       * @param limit the maximum number of accounts to return (max: 1000)
       * @returns a list of accounts mapping account names to account ids
       */
      map<string, account_id_type>      list_accounts(const string& lowerbound, uint32_t limit);
      /** List the balances of an account.
       * Each account can have multiple balances, one for each type of asset owned by that
       * account.  The returned list will only contain assets for which the account has a
       * nonzero balance
       * @param id the name or id of the account whose balances you want
       * @returns a list of the given account's balances
       */
      vector<asset_reserved>            list_account_balances(const string& id);
      /** Lists all assets registered on the blockchain.
       *
       * To list all assets, pass the empty string \c "" for the lowerbound to start
       * at the beginning of the list, and iterate as necessary.
       *
       * @param lowerbound  the symbol of the first asset to include in the list.
       * @param limit the maximum number of assets to return (max: 100)
       * @returns the list of asset objects, ordered by symbol
       */
      vector<asset_object>              list_assets(const string& lowerbound, uint32_t limit)const;
      /** Returns the most recent operations on the named account.
       *
       * This returns a list of operation history objects, which describe activity on the account.
       *
       * @note this API doesn't give a way to retrieve more than the most recent 100 transactions,
       *       you can interface directly with the blockchain to get more history
       * @param name the name or id of the account
       * @param limit the number of entries to return (starting from the most recent) (max 100)
       * @returns a list of \c operation_history_objects
       */
      vector<operation_detail>  get_account_history(string name, int limit)const;
      /** Returns the most recent operations on the named account filtered by operations.
       *
       * This returns a list of required operation history objects, which describe activity on the account.
       *
       * @note this API doesn't give a way to retrieve more than the most recent 100 transactions,
       *       you can interface directly with the blockchain to get more history
       * @param name the name or id of the account
       * @param operations the list of operations to filter on
       * @param limit the number of entries to return (starting from the most recent) (max 100)
       * @returns a list of \c operation_history_objects
       */
      vector<operation_detail>  get_account_history_by_operation(string name, flat_set<uint32_t> operations, int limit)const;

      vector<operation_detail> get_account_history_by_operation2(string name, flat_set<uint32_t> operations
            , string start_str, string end_str, int limit)const;


      vector<bucket_object>             get_market_history(string symbol, string symbol2, uint32_t bucket)const;
      vector<limit_order_object>        get_limit_orders(string a, string b, uint32_t limit)const;
      vector<call_order_object>         get_call_orders(string a, uint32_t limit)const;
      vector<force_settlement_object>   get_settle_orders(string a, uint32_t limit)const;

      /** Returns the block chain's slowly-changing settings.
       * This object contains all of the properties of the blockchain that are fixed
       * or that change only once per maintenance interval (daily) such as the
       * current list of witnesses, committee_members, block interval, etc.
       * @see \c get_dynamic_global_properties() for frequently changing properties
       * @returns the global properties
       */
      global_property_object            get_global_properties() const;

      /** Returns the block chain's rapidly-changing properties.
       * The returned object contains information that changes every block interval
       * such as the head block number, the next witness, etc.
       * @see \c get_global_properties() for less-frequently changing properties
       * @returns the dynamic global properties
       */
      dynamic_global_property_object    get_dynamic_global_properties() const;

      /** Returns information about the given account.
       *
       * @param account_name_or_id the name or id of the account to provide information about
       * @returns the public account data stored in the blockchain
       */
      account_object                    get_account(string account_name_or_id) const;

      /** Returns information about the given asset.
       * @param asset_name_or_id the symbol or id of the asset in question
       * @returns the information about the asset stored in the block chain
       */
      asset_object                      get_asset(string asset_name_or_id) const;

      /** Returns the BitAsset-specific data for a given asset.
       * Market-issued assets's behavior are determined both by their "BitAsset Data" and
       * their basic asset data, as returned by \c get_asset().
       * @param asset_name_or_id the symbol or id of the BitAsset in question
       * @returns the BitAsset-specific data for this asset
       */
      asset_bitasset_data_object        get_bitasset_data(string asset_name_or_id)const;

      /** Lookup the id of a named account.
       * @param account_name_or_id the name of the account to look up
       * @returns the id of the named account
       */
      account_id_type                   get_account_id(string account_name_or_id) const;

      /**
       * Lookup the id of a named asset.
       * @param asset_name_or_id the symbol of an asset to look up
       * @returns the id of the given asset
       */
      asset_id_type                     get_asset_id(string asset_name_or_id) const;

      /**
       * Returns the blockchain object corresponding to the given id.
       *
       * This generic function can be used to retrieve any object from the blockchain
       * that is assigned an ID.  Certain types of objects have specialized convenience
       * functions to return their objects -- e.g., assets have \c get_asset(), accounts
       * have \c get_account(), but this function will work for any object.
       *
       * @param id the id of the object to return
       * @returns the requested object
       */
      variant                           get_object(object_id_type id) const;

      /** Returns the current wallet filename.
       *
       * This is the filename that will be used when automatically saving the wallet.
       *
       * @see set_wallet_filename()
       * @return the wallet filename
       */
      string                            get_wallet_filename() const;

      /**
       * Get the WIF private key corresponding to a public key.  The
       * private key must already be in the wallet.
       */
      string                            get_private_key( public_key_type pubkey )const;

      /**
       * @ingroup Transaction Builder API
       */
      transaction_handle_type begin_builder_transaction();
      /**
       * @ingroup Transaction Builder API
       */
      void add_operation_to_builder_transaction(transaction_handle_type transaction_handle, const operation& op);
      /**
       * @ingroup Transaction Builder API
       */
      void replace_operation_in_builder_transaction(transaction_handle_type handle,
                                                    unsigned operation_index,
                                                    const operation& new_op);
      /**
       * @ingroup Transaction Builder API
       */
      asset set_fees_on_builder_transaction(transaction_handle_type handle, string fee_asset = GRAPHENE_SYMBOL);
      /**
       * @ingroup Transaction Builder API
       */
      transaction preview_builder_transaction(transaction_handle_type handle);
      /**
       * @ingroup Transaction Builder API
       */
      signed_transaction sign_builder_transaction(transaction_handle_type transaction_handle,
                                                  optional<vector<string>> wif_keys, bool broadcast = true);
      /**
       * @ingroup Transaction Builder API
       */
      signed_transaction propose_builder_transaction(
          transaction_handle_type handle,
          time_point_sec expiration = time_point::now() + fc::minutes(1),
          uint32_t review_period_seconds = 0,
          bool broadcast = true
         );

      signed_transaction propose_builder_transaction2(
         transaction_handle_type handle,
         string account_name_or_id,
         time_point_sec expiration = time_point::now() + fc::minutes(1),
         uint32_t review_period_seconds = 0,
         bool broadcast = true
        );

      /**
       * @ingroup Transaction Builder API
       */
      void remove_builder_transaction(transaction_handle_type handle);

      /** Checks whether the wallet has just been created and has not yet had a password set.
       *
       * Calling \c set_password will transition the wallet to the locked state.
       * @return true if the wallet is new
       * @ingroup Wallet Management
       */
      bool    is_new()const;

      /** Checks whether the wallet is locked (is unable to use its private keys).
       *
       * This state can be changed by calling \c lock() or \c unlock().
       * @return true if the wallet is locked
       * @ingroup Wallet Management
       */
      bool    is_locked()const;

      /** Locks the wallet immediately.
       * @ingroup Wallet Management
       */
      void    lock();

      /** Unlocks the wallet.
       *
       * The wallet remain unlocked until the \c lock is called
       * or the program exits.
       * @param password the password previously set with \c set_password()
       * @ingroup Wallet Management
       */
      void    unlock(string password);

      /** Sets a new password on the wallet.
       *
       * The wallet must be either 'new' or 'unlocked' to
       * execute this command.
       * @ingroup Wallet Management
       */
      void    set_password(string password);

      /** Dumps all private keys owned by the wallet.
       *
       * The keys are printed in WIF format.  You can import these keys into another wallet
       * using \c import_key()
       * @returns a map containing the private keys, indexed by their public key
       */
      map<public_key_type, string> dump_private_keys();

      /** Returns a list of all commands supported by the wallet API.
       *
       * This lists each command, along with its arguments and return types.
       * For more detailed help on a single command, use \c get_help()
       *
       * @returns a multi-line string suitable for displaying on a terminal
       */
      string  help()const;

      /** Returns detailed help on a single API command.
       * @param method the name of the API command you want help with
       * @returns a multi-line string suitable for displaying on a terminal
       */
      string  gethelp(const string& method)const;

      /** Loads a specified Graphene wallet.
       *
       * The current wallet is closed before the new wallet is loaded.
       *
       * @warning This does not change the filename that will be used for future
       * wallet writes, so this may cause you to overwrite your original
       * wallet unless you also call \c set_wallet_filename()
       *
       * @param wallet_filename the filename of the wallet JSON file to load.
       *                        If \c wallet_filename is empty, it reloads the
       *                        existing wallet file
       * @returns true if the specified wallet is loaded
       */
      bool    load_wallet_file(string wallet_filename = "");

      /** Saves the current wallet to the given filename.
       *
       * @warning This does not change the wallet filename that will be used for future
       * writes, so think of this function as 'Save a Copy As...' instead of
       * 'Save As...'.  Use \c set_wallet_filename() to make the filename
       * persist.
       * @param wallet_filename the filename of the new wallet JSON file to create
       *                        or overwrite.  If \c wallet_filename is empty,
       *                        save to the current filename.
       */
      void    save_wallet_file(string wallet_filename = "");

      /** Sets the wallet filename used for future writes.
       *
       * This does not trigger a save, it only changes the default filename
       * that will be used the next time a save is triggered.
       *
       * @param wallet_filename the new filename to use for future saves
       */
      void    set_wallet_filename(string wallet_filename);

      /** Suggests a safe brain key to use for creating your account.
       * \c create_account_with_brain_key() requires you to specify a 'brain key',
       * a long passphrase that provides enough entropy to generate cyrptographic
       * keys.  This function will suggest a suitably random string that should
       * be easy to write down (and, with effort, memorize).
       * @returns a suggested brain_key
       */
      brain_key_info suggest_brain_key()const;

     /**
      * Derive any number of *possible* owner keys from a given brain key.
      *
      * NOTE: These keys may or may not match with the owner keys of any account.
      * This function is merely intended to assist with account or key recovery.
      *
      * @see suggest_brain_key()
      *
      * @param brain_key    Brain key
      * @param number_of_desired_keys  Number of desired keys
      * @return A list of keys that are deterministically derived from the brainkey
      */
     vector<brain_key_info> derive_owner_keys_from_brain_key(string brain_key, int number_of_desired_keys = 1) const;

      /** Converts a signed_transaction in JSON form to its binary representation.
       *
       * TODO: I don't see a broadcast_transaction() function, do we need one?
       *
       * @param tx the transaction to serialize
       * @returns the binary form of the transaction.  It will not be hex encoded,
       *          this returns a raw string that may have null characters embedded
       *          in it
       */
      string serialize_transaction(signed_transaction tx) const;

      /** Imports the private key for an existing account.
       *
       * The private key must match either an owner key or an active key for the
       * named account.
       *
       * @see dump_private_keys()
       *
       * @param account_name_or_id the account owning the key
       * @param wif_key the private key in WIF format
       * @returns true if the key was imported
       */
      bool import_key(string account_name_or_id, string wif_key);

      map<string, bool> import_accounts( string filename, string password );

      bool import_account_keys( string filename, string password, string src_account_name, string dest_account_name );

      /**
       * This call will construct transaction(s) that will claim all balances controled
       * by wif_keys and deposit them into the given account.
       */
      vector< signed_transaction > import_balance( string account_name_or_id, const vector<string>& wif_keys, bool broadcast );

      /** Transforms a brain key to reduce the chance of errors when re-entering the key from memory.
       *
       * This takes a user-supplied brain key and normalizes it into the form used
       * for generating private keys.  In particular, this upper-cases all ASCII characters
       * and collapses multiple spaces into one.
       * @param s the brain key as supplied by the user
       * @returns the brain key in its normalized form
       */
      string normalize_brain_key(string s) const;

      /** Registers a third party's account on the blockchain.
       *
       * This function is used to register an account for which you do not own the private keys.
       * When acting as a registrar, an end user will generate their own private keys and send
       * you the public keys.  The registrar will use this function to register the account
       * on behalf of the end user.
       *
       * @see create_account_with_brain_key()
       *
       * @param name the name of the account, must be unique on the blockchain.  Shorter names
       *             are more expensive to register; the rules are still in flux, but in general
       *             names of more than 8 characters with at least one digit will be cheap.
       * @param owner the owner key for the new account
       * @param active the active key for the new account
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction register_account(string name,
                                          public_key_type owner,
                                          public_key_type active,
                                          bool broadcast = false);

      /** Registers a third party's account on the blockchain.
       *
       * This function is used to register an account for which you do not own the private keys.
       * When acting as a registrar, an end user will generate their own private keys and send
       * you the public keys.  The registrar will use this function to register the account
       * on behalf of the end user.
       *
       * @see create_account_with_brain_key()
       *
       * @param kind the kind of the account, i.e. vault
       * @param name the name of the account, must be unique on the blockchain.  Shorter names
       *             are more expensive to register; the rules are still in flux, but in general
       *             names of more than 8 characters with at least one digit will be cheap.
       * @param owner the owner key for the new account
       * @param active the active key for the new account
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction create_account(account_kind kind,
                                        string name,
                                        public_key_type owner,
                                        public_key_type active,
                                        bool broadcast = false);

      /** Registers a third party's vault account on the blockchain.
       *
       * This function is used to register a vault account for which you do not own the private keys.
       * When acting as a registrar, an end user will generate their own private keys and send
       * you the public keys.  The registrar will use this function to register the account
       * on behalf of the end user.
       *
       * @see register_account()
       *
       * @param name the name of the account, must be unique on the blockchain.  Shorter names
       *             are more expensive to register; the rules are still in flux, but in general
       *             names of more than 8 characters with at least one digit will be cheap.
       * @param owner the owner key for the new account
       * @param active the active key for the new account
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction register_vault_account(string name,
                                                public_key_type owner,
                                                public_key_type active,
                                                bool broadcast = false);

      /** Tethers a wallet account to a vault account on the blockchain.
       *
       * This function is used to tether two accounts (a wallet and a vault).
       *
       * @param wallet the name or id of the wallet account to tether
       * @param vault the name or id of the vault account to tether
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction tethering two accounts
       */
      signed_transaction tether_accounts(string wallet,
                                         string vault,
                                         bool broadcast = false);

      /**
       *  Upgrades an account to prime status.
       *  This makes the account holder a 'lifetime member'.
       *
       *  @todo there is no option for annual membership
       *  @param name the name or id of the account to upgrade
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction upgrading the account
       */
      signed_transaction upgrade_account(string name, bool broadcast);

      /** Creates a new account and registers it on the blockchain.
       *
       * @todo why no referrer_percent here?
       *
       * @see suggest_brain_key()
       * @see register_account()
       *
       * @param brain_key the brain key used for generating the account's private keys
       * @param account_name the name of the account, must be unique on the blockchain.  Shorter names
       *                     are more expensive to register; the rules are still in flux, but in general
       *                     names of more than 8 characters with at least one digit will be cheap.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering the account
       */
      signed_transaction create_account_with_brain_key(string brain_key,
                                                       string account_name,
                                                       bool broadcast = false);

      /** Transfer an amount from one account to another.
       * @param from the name or id of the account sending the funds
       * @param to the name or id of the account receiving the funds
       * @param amount the amount to send (in nominal units -- to send half of a BTS, specify 0.5)
       * @param asset_symbol the symbol or id of the asset to send
       * @param memo a memo to attach to the transaction.  The memo will be encrypted in the
       *             transaction and readable for the receiver.  There is no length limit
       *             other than the limit imposed by maximum transaction size, but transaction
       *             increase with transaction size
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction transferring funds
       */
      signed_transaction transfer(string from,
                                  string to,
                                  string amount,
                                  string asset_symbol,
                                  string memo,
                                  bool broadcast = false);

      /**
       *  This method works just like transfer, except it always broadcasts and
       *  returns the transaction ID along with the signed transaction.
       */
      pair<transaction_id_type,signed_transaction> transfer2(string from,
                                                             string to,
                                                             string amount,
                                                             string asset_symbol,
                                                             string memo ) {
         auto trx = transfer( from, to, amount, asset_symbol, memo, true );
         return std::make_pair(trx.id(),trx);
      }

      /** Transfer an amount from one vault to wallet.
       * @param vault the name or id of the vault sending the funds
       * @param wallet the name or id of the wallet receiving the funds
       * @param amount the amount to send (in nominal units -- to send half of a BTS, specify 0.5)
       * @param asset_symbol the symbol or id of the asset to send
       * @param reserved_amount the amount to send from reserved balance
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction transferring funds
       */
      signed_transaction transfer_vault_to_wallet(string vault,
                                  string wallet,
                                  string amount,
                                  string asset_symbol,
                                  share_type reserved_amount,
                                  bool broadcast = false);

      /**
       *  This method is used to convert a JSON transaction to its transactin ID.
       */
      transaction_id_type get_transaction_id( const signed_transaction& trx )const { return trx.id(); }


      /** Sign a memo message.
       *
       * @param from the name or id of signing account; or a public key.
       * @param to the name or id of receiving account; or a public key.
       * @param memo text to sign.
       */
      memo_data sign_memo(string from, string to, string memo);

      /** Read a memo.
       *
       * @param memo JSON-enconded memo.
       * @returns string with decrypted message..
       */
      string read_memo(const memo_data& memo);

      /** These methods are used for stealth transfers */
      ///@{
      /**
       *  This method can be used to set the label for a public key
       *
       *  @note No two keys can have the same label.
       *
       *  @return true if the label was set, otherwise false
       */
      bool                        set_key_label( public_key_type, string label );
      string                      get_key_label( public_key_type )const;

      /**
       *  Generates a new blind account for the given brain key and assigns it the given label.
       */
      public_key_type             create_blind_account( string label, string brain_key  );

      /**
       * @return the total balance of all blinded commitments that can be claimed by the
       * given account key or label
       */
      vector<asset>                get_blind_balances( string key_or_label );
      /** @return all blind accounts */
      map<string,public_key_type> get_blind_accounts()const;
      /** @return all blind accounts for which this wallet has the private key */
      map<string,public_key_type> get_my_blind_accounts()const;
      /** @return the public key associated with the given label */
      public_key_type             get_public_key( string label )const;
      ///@}

      /**
       * @return all blind receipts to/form a particular account
       */
      vector<blind_receipt> blind_history( string key_or_account );

      /**
       *  Given a confirmation receipt, this method will parse it for a blinded balance and confirm
       *  that it exists in the blockchain.  If it exists then it will report the amount received and
       *  who sent it.
       *
       *  @param opt_from - if not empty and the sender is a unknown public key, then the unknown public key will be given the label opt_from
       *  @param confirmation_receipt - a base58 encoded stealth confirmation
       */
      blind_receipt receive_blind_transfer( string confirmation_receipt, string opt_from, string opt_memo );

      /**
       *  Transfers a public balance from @from to one or more blinded balances using a
       *  stealth transfer.
       */
      blind_confirmation transfer_to_blind( string from_account_id_or_name,
                                            string asset_symbol,
                                            /** map from key or label to amount */
                                            vector<pair<string, string>> to_amounts,
                                            bool broadcast = false );

      /**
       * Transfers funds from a set of blinded balances to a public account balance.
       */
      blind_confirmation transfer_from_blind(
                                            string from_blind_account_key_or_label,
                                            string to_account_id_or_name,
                                            string amount,
                                            string asset_symbol,
                                            bool broadcast = false );

      /**
       *  Used to transfer from one set of blinded balances to another
       */
      blind_confirmation blind_transfer( string from_key_or_label,
                                         string to_key_or_label,
                                         string amount,
                                         string symbol,
                                         bool broadcast = false );

      /** Place a limit order attempting to sell one asset for another.
       *
       * Buying and selling are the same operation on Graphene; if you want to buy BTS
       * with USD, you should sell USD for BTS.
       *
       * The blockchain will attempt to sell the \c symbol_to_sell for as
       * much \c symbol_to_receive as possible, as long as the price is at
       * least \c min_to_receive / \c amount_to_sell.
       *
       * In addition to the transaction fees, market fees will apply as specified
       * by the issuer of both the selling asset and the receiving asset as
       * a percentage of the amount exchanged.
       *
       * If either the selling asset or the receiving asset is whitelist
       * restricted, the order will only be created if the seller is on
       * the whitelist of the restricted asset type.
       *
       * Market orders are matched in the order they are included
       * in the block chain.
       *
       * @todo Allow order expiration to be set here.  Document default/max expiration time
       *
       * @param seller_account the account providing the asset being sold, and which will
       *                       receive the proceeds of the sale.
       * @param amount_to_sell the amount of the asset being sold to sell (in nominal units)
       * @param symbol_to_sell the name or id of the asset to sell
       * @param min_to_receive the minimum amount you are willing to receive in return for
       *                       selling the entire amount_to_sell
       * @param symbol_to_receive the name or id of the asset you wish to receive
       * @param timeout_sec if the order does not fill immediately, this is the length of
       *                    time the order will remain on the order books before it is
       *                    cancelled and the un-spent funds are returned to the seller's
       *                    account
       * @param fill_or_kill if true, the order will only be included in the blockchain
       *                     if it is filled immediately; if false, an open order will be
       *                     left on the books to fill any amount that cannot be filled
       *                     immediately.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction selling the funds
       */
      signed_transaction sell_asset(string seller_account,
                                    string amount_to_sell,
                                    string   symbol_to_sell,
                                    string min_to_receive,
                                    string   symbol_to_receive,
                                    uint32_t timeout_sec = 0,
                                    bool     fill_or_kill = false,
                                    bool     broadcast = false);

      /** Place a limit order attempting to sell one asset for another.
       *
       * This API call abstracts away some of the details of the sell_asset call to be more
       * user friendly. All orders placed with sell never timeout and will not be killed if they
       * cannot be filled immediately. If you wish for one of these parameters to be different,
       * then sell_asset should be used instead.
       *
       * @param seller_account the account providing the asset being sold, and which will
       *                       receive the processed of the sale.
       * @param base The name or id of the asset to sell.
       * @param quote The name or id of the asset to recieve.
       * @param rate The rate in base:quote at which you want to sell.
       * @param amount The amount of base you want to sell.
       * @param broadcast true to broadcast the transaction on the network.
       * @returns The signed transaction selling the funds.
       */
      signed_transaction sell( string seller_account,
                               string base,
                               string quote,
                               double rate,
                               double amount,
                               bool broadcast );

      /** Place a limit order attempting to buy one asset with another.
       *
       * This API call abstracts away some of the details of the sell_asset call to be more
       * user friendly. All orders placed with buy never timeout and will not be killed if they
       * cannot be filled immediately. If you wish for one of these parameters to be different,
       * then sell_asset should be used instead.
       *
       * @param buyer_account The account buying the asset for another asset.
       * @param base The name or id of the asset to buy.
       * @param quote The name or id of the assest being offered as payment.
       * @param rate The rate in base:quote at which you want to buy.
       * @param amount the amount of base you want to buy.
       * @param broadcast true to broadcast the transaction on the network.
       * @param The signed transaction selling the funds.
       */
      signed_transaction buy( string buyer_account,
                              string base,
                              string quote,
                              double rate,
                              double amount,
                              bool broadcast );

      /** Borrow an asset or update the debt/collateral ratio for the loan.
       *
       * This is the first step in shorting an asset.  Call \c sell_asset() to complete the short.
       *
       * @param borrower_name the name or id of the account associated with the transaction.
       * @param amount_to_borrow the amount of the asset being borrowed.  Make this value
       *                         negative to pay back debt.
       * @param asset_symbol the symbol or id of the asset being borrowed.
       * @param amount_of_collateral the amount of the backing asset to add to your collateral
       *        position.  Make this negative to claim back some of your collateral.
       *        The backing asset is defined in the \c bitasset_options for the asset being borrowed.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction borrowing the asset
       */
      signed_transaction borrow_asset(string borrower_name, string amount_to_borrow, string asset_symbol,
                                      string amount_of_collateral, bool broadcast = false);

      /** Cancel an existing order
       *
       * @param order_id the id of order to be cancelled
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction canceling the order
       */
      signed_transaction cancel_order(object_id_type order_id, bool broadcast = false);

      /** Creates a new user-issued or market-issued asset.
       *
       * Many options can be changed later using \c update_asset()
       *
       * Right now this function is difficult to use because you must provide raw JSON data
       * structures for the options objects, and those include prices and asset ids.
       *
       * @param issuer the name or id of the account who will pay the fee and become the
       *               issuer of the new asset.  This can be updated later
       * @param symbol the ticker symbol of the new asset
       * @param precision the number of digits of precision to the right of the decimal point,
       *                  must be less than or equal to 12
       * @param common asset options required for all new assets.
       *               Note that core_exchange_rate technically needs to store the asset ID of
       *               this new asset. Since this ID is not known at the time this operation is
       *               created, create this price as though the new asset has instance ID 1, and
       *               the chain will overwrite it with the new asset's ID.
       * @param bitasset_opts options specific to BitAssets.  This may be null unless the
       *               \c market_issued flag is set in common.flags
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction creating a new asset
       */
      signed_transaction create_asset(string issuer,
                                      string symbol,
                                      uint8_t precision,
                                      asset_options common,
                                      fc::optional<bitasset_options> bitasset_opts,
                                      bool broadcast = false);

      /** Issue new shares of an asset.
       *
       * @param to_account the name or id of the account to receive the new shares
       * @param amount the amount to issue, in nominal units
       * @param symbol the ticker symbol of the asset to issue
       * @param memo a memo to include in the transaction, readable by the recipient
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction issuing the new shares
       */
      signed_transaction issue_asset(string to_account, string amount,
                                     string symbol,
                                     string memo,
                                     bool broadcast = false);

      /** Issue webasset to an account's balance.
       *
       * @param to_account the name or id of the account to receive the webasset
       * @param amount the amount to issue, in nominal units
       * @param reserved reserved amount to issue, in nominal units
       * @param unique_id unique identifier of this issue
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction issuing the webasset
       */
      signed_transaction issue_webasset(string to_account,
                                        string amount,
                                        string reserved,
                                        string unique_id,
                                        bool broadcast = false);

      /** Update the core options on an asset.
       * There are a number of options which all assets in the network use. These options are
       * enumerated in the asset_object::asset_options struct. This command is used to update
       * these options for an existing asset.
       *
       * @note This operation cannot be used to update BitAsset-specific options. For these options,
       * \c update_bitasset() instead.
       *
       * @param symbol the name or id of the asset to update
       * @param new_issuer if changing the asset's issuer, the name or id of the new issuer.
       *                   null if you wish to remain the issuer of the asset
       * @param new_options the new asset_options object, which will entirely replace the existing
       *                    options.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the asset
       */
      signed_transaction update_asset(string symbol,
                                      optional<string> new_issuer,
                                      asset_options new_options,
                                      bool broadcast = false);

      /** Update the options specific to a BitAsset.
       *
       * BitAssets have some options which are not relevant to other asset types. This operation is used to update those
       * options an an existing BitAsset.
       *
       * @see update_asset()
       *
       * @param symbol the name or id of the asset to update, which must be a market-issued asset
       * @param new_options the new bitasset_options object, which will entirely replace the existing
       *                    options.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the bitasset
       */
      signed_transaction update_bitasset(string symbol,
                                         bitasset_options new_options,
                                         bool broadcast = false);

      /** Update the set of feed-producing accounts for a BitAsset.
       *
       * BitAssets have price feeds selected by taking the median values of recommendations from a set of feed producers.
       * This command is used to specify which accounts may produce feeds for a given BitAsset.
       * @param symbol the name or id of the asset to update
       * @param new_feed_producers a list of account names or ids which are authorized to produce feeds for the asset.
       *                           this list will completely replace the existing list
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the bitasset's feed producers
       */
      signed_transaction update_asset_feed_producers(string symbol,
                                                     flat_set<string> new_feed_producers,
                                                     bool broadcast = false);

      /** Publishes a price feed for the named asset.
       *
       * Price feed providers use this command to publish their price feeds for market-issued assets. A price feed is
       * used to tune the market for a particular market-issued asset. For each value in the feed, the median across all
       * committee_member feeds for that asset is calculated and the market for the asset is configured with the median of that
       * value.
       *
       * The feed object in this command contains three prices: a call price limit, a short price limit, and a settlement price.
       * The call limit price is structured as (collateral asset) / (debt asset) and the short limit price is structured
       * as (asset for sale) / (collateral asset). Note that the asset IDs are opposite to eachother, so if we're
       * publishing a feed for USD, the call limit price will be CORE/USD and the short limit price will be USD/CORE. The
       * settlement price may be flipped either direction, as long as it is a ratio between the market-issued asset and
       * its collateral.
       *
       * @param publishing_account the account publishing the price feed
       * @param symbol the name or id of the asset whose feed we're publishing
       * @param feed the price_feed object containing the three prices making up the feed
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction updating the price feed for the given asset
       */
      signed_transaction publish_asset_feed(string publishing_account,
                                            string symbol,
                                            price_feed feed,
                                            bool broadcast = false);

      /** Pay into the fee pool for the given asset.
       *
       * User-issued assets can optionally have a pool of the core asset which is
       * automatically used to pay transaction fees for any transaction using that
       * asset (using the asset's core exchange rate).
       *
       * This command allows anyone to deposit the core asset into this fee pool.
       *
       * @param from the name or id of the account sending the core asset
       * @param symbol the name or id of the asset whose fee pool you wish to fund
       * @param amount the amount of the core asset to deposit
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction funding the fee pool
       */
      signed_transaction fund_asset_fee_pool(string from,
                                             string symbol,
                                             string amount,
                                             bool broadcast = false);

      /** Claim funds from the accumulated fees pool for the given asset.
       *
       * User-issued assets can optionally have a pool of the accumulated fees which are
       * paid for market fees
       *
       * This command allows the issuer to withdraw those funds from the fee pool.
       *
       * @param symbol the name or id of the asset whose accumulated fees pool you wish to claim
       * @param amount the amount of the core asset to withdraw
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction claiming from the fee pool
       */
      signed_transaction claim_asset_accumulated_fees_pool(string symbol,
                                                           string amount,
                                                           bool broadcast = false);

      /** Burns the given user-issued asset.
       *
       * This command burns the user-issued asset to reduce the amount in circulation.
       * @note you cannot burn market-issued assets.
       * @param from the account containing the asset you wish to burn
       * @param amount the amount to burn, in nominal units
       * @param symbol the name or id of the asset to burn
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction burning the asset
       */
      signed_transaction reserve_asset(string from,
                                    string amount,
                                    string symbol,
                                    bool broadcast = false);

      /** Forces a global settling of the given asset (black swan or prediction markets).
       *
       * In order to use this operation, asset_to_settle must have the global_settle flag set
       *
       * When this operation is executed all balances are converted into the backing asset at the
       * settle_price and all open margin positions are called at the settle price.  If this asset is
       * used as backing for other bitassets, those bitassets will be force settled at their current
       * feed price.
       *
       * @note this operation is used only by the asset issuer, \c settle_asset() may be used by
       *       any user owning the asset
       *
       * @param symbol the name or id of the asset to force settlement on
       * @param settle_price the price at which to settle
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction settling the named asset
       */
      signed_transaction global_settle_asset(string symbol,
                                             price settle_price,
                                             bool broadcast = false);

      /** Schedules a market-issued asset for automatic settlement.
       *
       * Holders of market-issued assests may request a forced settlement for some amount of their asset. This means that
       * the specified sum will be locked by the chain and held for the settlement period, after which time the chain will
       * choose a margin posision holder and buy the settled asset using the margin's collateral. The price of this sale
       * will be based on the feed price for the market-issued asset being settled. The exact settlement price will be the
       * feed price at the time of settlement with an offset in favor of the margin position, where the offset is a
       * blockchain parameter set in the global_property_object.
       *
       * @param account_to_settle the name or id of the account owning the asset
       * @param amount_to_settle the amount of the named asset to schedule for settlement
       * @param symbol the name or id of the asset to settlement on
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction settling the named asset
       */
      signed_transaction settle_asset(string account_to_settle,
                                      string amount_to_settle,
                                      string symbol,
                                      bool broadcast = false);

      /** Whitelist and blacklist accounts, primarily for transacting in whitelisted assets.
       *
       * Accounts can freely specify opinions about other accounts, in the form of either whitelisting or blacklisting
       * them. This information is used in chain validation only to determine whether an account is authorized to transact
       * in an asset type which enforces a whitelist, but third parties can use this information for other uses as well,
       * as long as it does not conflict with the use of whitelisted assets.
       *
       * An asset which enforces a whitelist specifies a list of accounts to maintain its whitelist, and a list of
       * accounts to maintain its blacklist. In order for a given account A to hold and transact in a whitelisted asset S,
       * A must be whitelisted by at least one of S's whitelist_authorities and blacklisted by none of S's
       * blacklist_authorities. If A receives a balance of S, and is later removed from the whitelist(s) which allowed it
       * to hold S, or added to any blacklist S specifies as authoritative, A's balance of S will be frozen until A's
       * authorization is reinstated.
       *
       * @param authorizing_account the account who is doing the whitelisting
       * @param account_to_list the account being whitelisted
       * @param new_listing_status the new whitelisting status
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction changing the whitelisting status
       */
      signed_transaction whitelist_account(string authorizing_account,
                                           string account_to_list,
                                           account_whitelist_operation::account_listing new_listing_status,
                                           bool broadcast = false);

      /** Creates a committee_member object owned by the given account.
       *
       * An account can have at most one committee_member object.
       *
       * @param owner_account the name or id of the account which is creating the committee_member
       * @param url a URL to include in the committee_member record in the blockchain.  Clients may
       *            display this when showing a list of committee_members.  May be blank.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a committee_member
       */
      signed_transaction create_committee_member(string owner_account,
                                         string url,
                                         bool broadcast = false);

      /** Lists all witnesses registered in the blockchain.
       * This returns a list of all account names that own witnesses, and the associated witness id,
       * sorted by name.  This lists witnesses whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all witnesss,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last witness name returned as the \c lowerbound for the next \c list_witnesss() call.
       *
       * @param lowerbound the name of the first witness to return.  If the named witness does not exist,
       *                   the list will start at the witness that comes after \c lowerbound
       * @param limit the maximum number of witnesss to return (max: 1000)
       * @returns a list of witnesss mapping witness names to witness ids
       */
      map<string,witness_id_type>       list_witnesses(const string& lowerbound, uint32_t limit);

      /** Lists all committee_members registered in the blockchain.
       * This returns a list of all account names that own committee_members, and the associated committee_member id,
       * sorted by name.  This lists committee_members whether they are currently voted in or not.
       *
       * Use the \c lowerbound and limit parameters to page through the list.  To retrieve all committee_members,
       * start by setting \c lowerbound to the empty string \c "", and then each iteration, pass
       * the last committee_member name returned as the \c lowerbound for the next \c list_committee_members() call.
       *
       * @param lowerbound the name of the first committee_member to return.  If the named committee_member does not exist,
       *                   the list will start at the committee_member that comes after \c lowerbound
       * @param limit the maximum number of committee_members to return (max: 1000)
       * @returns a list of committee_members mapping committee_member names to committee_member ids
       */
      map<string, committee_member_id_type>       list_committee_members(const string& lowerbound, uint32_t limit);

      /** Returns information about the given witness.
       * @param owner_account the name or id of the witness account owner, or the id of the witness
       * @returns the information about the witness stored in the block chain
       */
      witness_object get_witness(string owner_account);

      /** Returns information about the given committee_member.
       * @param owner_account the name or id of the committee_member account owner, or the id of the committee_member
       * @returns the information about the committee_member stored in the block chain
       */
      committee_member_object get_committee_member(string owner_account);

      /** Creates a witness object owned by the given account.
       *
       * An account can have at most one witness object.
       *
       * @param owner_account the name or id of the account which is creating the witness
       * @param url a URL to include in the witness record in the blockchain.  Clients may
       *            display this when showing a list of witnesses.  May be blank.
       * @param broadcast true to broadcast the transaction on the network
       * @returns the signed transaction registering a witness
       */
      signed_transaction create_witness(string owner_account,
                                        string url,
                                        bool broadcast = false);

      /**
       * Update a witness object owned by the given account.
       *
       * @param witness The name of the witness's owner account.  Also accepts the ID of the owner account or the ID of the witness.
       * @param url Same as for create_witness.  The empty string makes it remain the same.
       * @param block_signing_key The new block signing public key.  The empty string makes it remain the same.
       * @param broadcast true if you wish to broadcast the transaction.
       */
      signed_transaction update_witness(string witness_name,
                                        string url,
                                        string block_signing_key,
                                        bool broadcast = false);


      /**
       * Create a worker object.
       *
       * @param owner_account The account which owns the worker and will be paid
       * @param work_begin_date When the work begins
       * @param work_end_date When the work ends
       * @param daily_pay Amount of pay per day (NOT per maint interval)
       * @param name Any text
       * @param url Any text
       * @param worker_settings {"type" : "burn"|"refund"|"vesting", "pay_vesting_period_days" : x}
       * @param broadcast true if you wish to broadcast the transaction.
       */
      signed_transaction create_worker(
         string owner_account,
         time_point_sec work_begin_date,
         time_point_sec work_end_date,
         share_type daily_pay,
         string name,
         string url,
         variant worker_settings,
         bool broadcast = false
         );

      /**
       * Update your votes for a worker
       *
       * @param account The account which will pay the fee and update votes.
       * @param worker_vote_delta {"vote_for" : [...], "vote_against" : [...], "vote_abstain" : [...]}
       * @param broadcast true if you wish to broadcast the transaction.
       */
      signed_transaction update_worker_votes(
         string account,
         worker_vote_delta delta,
         bool broadcast = false
         );

      /**
       * Get information about a vesting balance object.
       *
       * @param account_name An account name, account ID, or vesting balance object ID.
       */
      vector< vesting_balance_object_with_info > get_vesting_balances( string account_name );

      /**
       * Withdraw a vesting balance.
       *
       * @param witness_name The account name of the witness, also accepts account ID or vesting balance ID type.
       * @param amount The amount to withdraw.
       * @param asset_symbol The symbol of the asset to withdraw.
       * @param broadcast true if you wish to broadcast the transaction
       */
      signed_transaction withdraw_vesting(
         string witness_name,
         string amount,
         string asset_symbol,
         bool broadcast = false);

      /** Vote for a given committee_member.
       *
       * An account can publish a list of all committee_memberes they approve of.  This
       * command allows you to add or remove committee_memberes from this list.
       * Each account's vote is weighted according to the number of shares of the
       * core asset owned by that account at the time the votes are tallied.
       *
       * @note you cannot vote against a committee_member, you can only vote for the committee_member
       *       or not vote for the committee_member.
       *
       * @param voting_account the name or id of the account who is voting with their shares
       * @param committee_member the name or id of the committee_member' owner account
       * @param approve true if you wish to vote in favor of that committee_member, false to
       *                remove your vote in favor of that committee_member
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote for the given committee_member
       */
      signed_transaction vote_for_committee_member(string voting_account,
                                           string committee_member,
                                           bool approve,
                                           bool broadcast = false);

      /** Vote for a given witness.
       *
       * An account can publish a list of all witnesses they approve of.  This
       * command allows you to add or remove witnesses from this list.
       * Each account's vote is weighted according to the number of shares of the
       * core asset owned by that account at the time the votes are tallied.
       *
       * @note you cannot vote against a witness, you can only vote for the witness
       *       or not vote for the witness.
       *
       * @param voting_account the name or id of the account who is voting with their shares
       * @param witness the name or id of the witness' owner account
       * @param approve true if you wish to vote in favor of that witness, false to
       *                remove your vote in favor of that witness
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote for the given witness
       */
      signed_transaction vote_for_witness(string voting_account,
                                          string witness,
                                          bool approve,
                                          bool broadcast = false);

      /** Set the voting proxy for an account.
       *
       * If a user does not wish to take an active part in voting, they can choose
       * to allow another account to vote their stake.
       *
       * Setting a vote proxy does not remove your previous votes from the blockchain,
       * they remain there but are ignored.  If you later null out your vote proxy,
       * your previous votes will take effect again.
       *
       * This setting can be changed at any time.
       *
       * @param account_to_modify the name or id of the account to update
       * @param voting_account the name or id of an account authorized to vote account_to_modify's shares,
       *                       or null to vote your own shares
       *
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote proxy settings
       */
      signed_transaction set_voting_proxy(string account_to_modify,
                                          optional<string> voting_account,
                                          bool broadcast = false);

      /** Set your vote for the number of witnesses and committee_members in the system.
       *
       * Each account can voice their opinion on how many committee_members and how many
       * witnesses there should be in the active committee_member/active witness list.  These
       * are independent of each other.  You must vote your approval of at least as many
       * committee_members or witnesses as you claim there should be (you can't say that there should
       * be 20 committee_members but only vote for 10).
       *
       * There are maximum values for each set in the blockchain parameters (currently
       * defaulting to 1001).
       *
       * This setting can be changed at any time.  If your account has a voting proxy
       * set, your preferences will be ignored.
       *
       * @param account_to_modify the name or id of the account to update
       * @param number_of_committee_members the number
       *
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed transaction changing your vote proxy settings
       */
      signed_transaction set_desired_witness_and_committee_member_count(string account_to_modify,
                                                                uint16_t desired_number_of_witnesses,
                                                                uint16_t desired_number_of_committee_members,
                                                                bool broadcast = false);

      /** Signs a transaction.
       *
       * Given a fully-formed transaction that is only lacking signatures, this signs
       * the transaction with the necessary keys and optionally broadcasts the transaction
       * @param tx the unsigned transaction
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction sign_transaction(signed_transaction tx, bool broadcast = false);

      signed_transaction sign_transaction_with_keys(signed_transaction tx, std::vector<string> wif_keys, bool broadcast = false);

      /** Returns an uninitialized object representing a given blockchain operation.
       *
       * This returns a default-initialized object of the given type; it can be used
       * during early development of the wallet when we don't yet have custom commands for
       * creating all of the operations the blockchain supports.
       *
       * Any operation the blockchain supports can be created using the transaction builder's
       * \c add_operation_to_builder_transaction() , but to do that from the CLI you need to
       * know what the JSON form of the operation looks like.  This will give you a template
       * you can fill in.  It's better than nothing.
       *
       * @param operation_type the type of operation to return, must be one of the
       *                       operations defined in `graphene/chain/operations.hpp`
       *                       (e.g., "global_parameters_update_operation")
       * @return a default-constructed operation of the given type
       */
      operation get_prototype_operation(string operation_type);

      /** Creates a transaction to propose a parameter change.
       *
       * Multiple parameters can be specified if an atomic change is
       * desired.
       *
       * @param proposing_account The account paying the fee to propose the tx
       * @param expiration_time Timestamp specifying when the proposal will either take effect or expire.
       * @param changed_values The values to change; all other chain parameters are filled in with default values
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction propose_parameter_change(
         const string& proposing_account,
         fc::time_point_sec expiration_time,
         const variant_object& changed_values,
         bool broadcast = false);

      /** Propose a fee change.
       *
       * @param proposing_account The account paying the fee to propose the tx
       * @param expiration_time Timestamp specifying when the proposal will either take effect or expire.
       * @param changed_values Map of operation type to new fee.  Operations may be specified by name or ID.
       *    The "scale" key changes the scale.  All other operations will maintain current values.
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction propose_fee_change(
         const string& proposing_account,
         fc::time_point_sec expiration_time,
         const variant_object& changed_values,
         bool broadcast = false);

      /** Approve or disapprove a proposal.
       *
       * @param fee_paying_account The account paying the fee for the op.
       * @param proposal_id The proposal to modify.
       * @param delta Members contain approvals to create or remove.  In JSON you can leave empty members undefined.
       * @param broadcast true if you wish to broadcast the transaction
       * @return the signed version of the transaction
       */
      signed_transaction approve_proposal(
         const string& fee_paying_account,
         const string& proposal_id,
         const approval_delta& delta,
         bool broadcast /* = false */
         );

      ///////////////////////////////
      /// LICENSES:               ///
      ///////////////////////////////

      /**
       * Issue a license to an account. This will create a license request object that can be denied by the license
       * authentication authority.
       *
       * @param issuer            This MUST be the license issuing chain authority.
       * @param account           The account that will benefit the license.
       * @param license           The id of the license that will be granted to the account.
       * @param bonus_percentage  Bonus percentage of license base cycles to be issued. Value must be greater than -100.
       * @param frequency         Frequency lock for this license.
       * @param broadcast         true if you wish to broadcast the transaction.
       * @return                  The signed version of the transaction.
       */
      signed_transaction issue_license(
        const string& issuer,
        const string& account,
        const string& license,
        share_type bonus_percentage,
        frequency_type frequency,
        bool broadcast /* false */
        );

      /**
       * Submit cycles from a license to the minting queue.
       *
       * @param account           The account that submits cycles.
       * @param amount            The amount submitted.
       * @param license           The name or id of the license from which to submit.
       * @param frequency         Frequency lock for this license.
       * @param comment           Comment for this submissions.
       * @param broadcast         true if you wish to broadcast the transaction.
       * @return                  The signed version of the transaction.
       */
      signed_transaction submit_cycles_to_queue_by_license(
        const string& account,
        share_type amount,
        const string& license,
        frequency_type frequency,
        const string& comment,
        bool broadcast /* false */
      );

      /**
       * Get all license type ids found on the blockchain
       *
       * @return Vector of license type ids
       */
      vector<license_type_object> get_license_types() const;

      /**
       * Get names and license type ids found on the blockchain
       *
       * @return Vector of license name/type-ids pairs
       */
      vector<pair<string, license_type_id_type>> get_license_type_names_ids() const;

      /**
       * Get a list of account issued license types. This function has semantics identical to get_objects
       *
       * @param account_ids IDs of the accounts to retrieve
       * @return            Vector of issued license information objects
       */
      vector<optional<license_information_object>> get_license_information(const vector<account_id_type>& account_ids) const;

      ///////////////////////////////
      /// CYCLES:                 ///
      ///////////////////////////////

      /**
       * Gets total amount of cycles distributed to all vault accounts and maximum amount of DasCoin that could be minted using these cycles.
       * @return An object containig total amounts of cycles and dascoin.
       */
      optional<total_cycles_res> get_total_cycles() const;

      /**
       * Get the amount of cycles in the account.
       * @param  account Account name or stringified id.
       * @return         Cycle balance of the account.
       */
      acc_id_share_t_res get_account_cycle_balance(const string& account) const;

      /**
       * Deprecated
       */
      acc_id_vec_cycle_agreement_res get_full_cycle_balances(const string& account) const;

      /**
       * Get amount of DasCoin for on an account.
       *
       * @param  account Account name or stringified id.
       * @return         An object containing dascoin balance of an account
       */
      acc_id_share_t_res get_dascoin_balance(const string& account) const;

      /**
       * Transfer cycles from license to wallet.
       * @param vault Vault - account name or id
       * @param license License to transfer cycles from
       * @param amount_of_cycles_to_transfer Symbol of asset to sell
       * @param wallet Wallet - account name or id
       */
      signed_transaction transfer_cycles_from_licence_to_wallet(string vault,
                                              license_type_id_type license,
                                              share_type amount_of_cycles_to_transfer,
                                              string wallet,
                                              bool broadcast = false);
      /**
       * Purchase cycles.
       * @param account Account name or id
       * @param amount_to_sell Amount of asset to sell
       * @param symbol_to_sell Symbol of asset to sell
       * @param frequency Frequency at which we buy
       * @param amount_of_cycles_to_receive Amount of cycles to receive by this buy
       */
      signed_transaction purchase_cycle_asset(string account,
                                              string amount_to_sell,
                                              string symbol_to_sell,
                                              double frequency,
                                              double amount_of_cycles_to_receive,
                                              bool broadcast = false);

      /**
       * Retrieve calculation of cycle price per asset
       * @param cycle_amount Amount of cycles we are buying
       * @param asset_symbol_or_id Symbol or symbol asset of paying asset
       */
      optional<cycle_price> calculate_cycle_price(share_type cycle_amount, string asset_symbol_or_id) const;

      /**
       * Update various reward queue parameters
       * 
       * @param enable_dascoin_queue         true if minting is enabled
       * @param reward_interval_time_seconds the time interval between DasCoin reward events
       * @param dascoin_reward_amount        the amount of DasCoins produced on the DasCoin reward event
       * @param broadcast                    true to broadcast the transaction on the network.
       * @return                             signed transaction updating the queue
       */
      signed_transaction update_queue_parameters(optional<bool> enable_dascoin_queue,
                                                 optional<uint32_t> reward_interval_time_seconds,
                                                 optional<share_type> dascoin_reward_amount,
                                                 bool broadcast) const;

      order_book get_order_book( const string& base, const string& quote, unsigned limit = 50);

      ///////////////////////////////
      /// WIRE:                   ///
      ///////////////////////////////
      /**
       * Wire out some WebAsset.
       * @param account Account ID.
       * @param amount  Amount to wire.
       */
      signed_transaction wire_out(const string& account, share_type amount, bool broadcast) const;

      /**
      * Wire out with fee some WebAsset.
      * @param account             Account ID.
      * @param amount              Amount to wire.
      * @param currency_of_choice  Currency of choice (string abbreviation) in which user wants wire out.
      * @param to_address          Destination blockchain address to which the amount needs to be wired.
      * @param memo                Optional note.
      * @param broadcast           True to broadcast the transaction on the network.
      */
      signed_transaction wire_out_with_fee(const string& account, share_type amount, const string& currency_of_choice,
                                           const string& to_address, const string& memo, bool broadcast) const;

      /**
      * Toggle roll-back enabled.
      * @param account             Account ID.
      * @param roll_back_enabled   New value for roll_back_enabled flag.
      * @param broadcast           True to broadcast the transaction on the network.
      */
      signed_transaction set_roll_back_enabled(const string& account, bool roll_back_enabled, bool broadcast) const;

      /**
      * Roll-back public keys.
      * @param authority           This MUST be personal information validation authority.
      * @param account             Account ID.
      * @param broadcast           True to broadcast the transaction on the network.
      */
      signed_transaction roll_back_public_keys(const string& authority, const string& account, bool broadcast) const;

      /**
      * Set chain authority
      * @param issuer             Account that is issuing this operation, must be root administrator
      * @param account            Account that will become authority, must be special account
      * @param kind               What authority will be set
      * @param broadcast          True to broadcast the transaction on the network.
      */
      signed_transaction set_chain_authority(const string& issuer, const string& account, const string& kind, bool broadcast) const;

      ///////////////////////////////
      /// DASPAY:                 ///
      ///////////////////////////////

      /**
      * Set DasPay Transaction Ratio.
      * @param authority           This MUST be daspay authority.
      * @param debit_ratio         New ratio for DEBIT transactions.
      * @param credit_ratio        New ratio for CREDIT transactions.
      * @param broadcast           True to broadcast the transaction on the network.
      */
      signed_transaction set_daspay_transaction_ratio(const string& authority, share_type debit_ratio, share_type credit_ratio, bool broadcast = false) const;
      //////////////////////////

      /**
      * Create payment service provider.
      * @param authority                                               This MUST be daspay authority.
      * @param payment_service_provider_account                        Account used to identify payment_service_provider.
      * @param payment_service_provider_clearing_accounts              List of clearing accounts for payment_service_provider_account.
      * @param broadcast                                               True to broadcast the transaction on the network.
      */
      signed_transaction create_payment_service_provider(const string& authority, const string& payment_service_provider_account, const vector<string>& payment_service_provider_clearing_accounts, bool broadcast = false) const;
      //////////////////////////

      /**
      * Update payment service provider.
      * @param authority                                               This MUST be daspay authority.
      * @param payment_service_provider_account                        Account used to identify payment_service_provider.
      * @param payment_service_provider_clearing_accounts              List of clearing accounts for payment_service_provider_account.
      * @param broadcast                                               True to broadcast the transaction on the network.
      */
       signed_transaction update_payment_service_provider(const string& authority, const string& payment_service_provider_account, const vector<string>& payment_service_provider_clearing_accounts, bool broadcast = false) const;
      //////////////////////////

      /**
      * Delete payment service provider.
      * @param authority                                               This MUST be daspay authority.
      * @param payment_service_provider_account                        Account used to identify payment_service_provider.
      * @param broadcast                                               True to broadcast the transaction on the network.
      */
      signed_transaction delete_payment_service_provider(const string& authority, const string& payment_service_provider_account, bool broadcast = false) const;

      /**
      * @brief Get all clearing accounts for all payment service providers.
      * @return List of payment service provider accounts with their respective clearing accounts.
      */
      vector<payment_service_provider_object> get_payment_service_providers() const;

      /**
       * Register daspay authority.
       * @param account                                                 Account ID.
       * @param payment_provider                                        Account of payment provider.
       * @param public_key_type                                         Public key to register to this Account.
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction register_daspay_authority(const string& account, const string& payment_provider, public_key_type daspay_public_key, bool broadcast = false) const;

      /**
       * Unregister daspay authority.
       * @param account                                                 Account ID.
       * @param payment_provider                                        Account of payment provider.
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction unregister_daspay_authority(const string& account, const string& payment_provider, bool broadcast = false) const;

      /**
       * Reserve asset on account.
       * @param account                                                 Account ID.
       * @param asset_amount                                            Asset amount
       * @param asset_symbol                                            Asset symbol
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction reserve_asset_on_account(const string& account, const string& asset_amount, const string& asset_symbol, bool broadcast = false) const;

      /**
       * Unreserve asset on account.
       * @param account                                                 Account ID.
       * @param asset_amount                                            Asset amount
       * @param asset_symbol                                            Asset symbol
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction unreserve_asset_on_account(const string& account, const string& asset_amount, const string& asset_symbol, bool broadcast = false) const;

      /**
       * DasPay debit user account.
       * @param payment_service_provider_account                        Account of payment service provider.
       * @param auth_key                                                Public key authorized for reserved assets on user account.
       * @param user_account                                            User account to debit.
       * @param asset_amount                                            Amount to debit (ie 12.50).
       * @param asset_symbol                                            Symbol or id of the asset to debit.
       * @param clearing_account                                        Payment service provider clearing account.
       * @param transaction_id                                          Payment service provider transaction id.
       * @param details                                                 Transaction details (optional).
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction daspay_debit_account(const string& payment_service_provider_account,
                                              const public_key_type& auth_key,
                                              const string& user_account,
                                              const string& asset_amount,
                                              const string& asset_symbol,
                                              const string& clearing_account,
                                              const string& transaction_id,
                                              optional<string> details,
                                              bool broadcast = false) const;

      /**
       * DasPay credit user account.
       * @param payment_service_provider_account                        Account of payment service provider.
       * @param user_account                                            User account to debit.
       * @param asset_amount                                            Amount to debit (ie 12.50).
       * @param asset_symbol                                            Symbol or id of the asset to debit.
       * @param clearing_account                                        Payment service provider clearing account.
       * @param transaction_id                                          Payment service provider transaction id.
       * @param details                                                 Transaction details (optional).
       * @param broadcast                                               True to broadcast the transaction on the network.
       */
      signed_transaction daspay_credit_account(const string& payment_service_provider_account,
                                               const string& user_account,
                                               const string& asset_amount,
                                               const string& asset_symbol,
                                               const string& clearing_account,
                                               const string& transaction_id,
                                               optional<string> details,
                                               bool broadcast = false) const;

      /**
       * Retrieve DasPay data for account
       * @param account                                                 Account ID.
       * @return An object containing daspay data of an account
       */
      optional<vector<daspay_authority>> get_daspay_authority_for_account(const string& account) const;

      /**
       * Update various daspay clearing parameters
       * 
       * @param authority                                               This MUST be daspay authority.
       * @param clearing_enabled                                        true if clearing is enabled
       * @param clearing_interval_time_seconds                          time in seconds between DasPay clearing events
       * @param collateral_dascoin                                      the amount of DasCoins for credit transactions collateral
       * @param collateral_webeur                                       the amount of WebEur for clearing collateral
       * @param broadcast                                               true to broadcast the transaction on the network.
       */
      signed_transaction update_daspay_clearing_parameters(const string& authority,
                                                           optional<bool> clearing_enabled,
                                                           optional<uint32_t> clearing_interval_time_seconds,
                                                           optional<share_type> collateral_dascoin,
                                                           optional<share_type> collateral_webeur,
                                                           bool broadcast) const;


      ///////////////////////////////
      /// DELAYED OPERATIONS:     ///
      ///////////////////////////////

      /**
       * Update various delayed operations resolver parameters
       *
       * @param authority                                               This MUST be root authority.
       * @param delayed_operations_resolver_enabled                     true if delayed operations resolver is enabled
       * @param delayed_operations_resolver_interval_time_seconds       time in seconds between two delayed operations resolver checks
       * @param broadcast                                               true to broadcast the transaction on the network.
       */
      signed_transaction update_delayed_operations_resolver_parameters(const string& authority, optional<bool> delayed_operations_resolver_enabled,
                                                           optional<uint32_t> delayed_operations_resolver_interval_time_seconds,
                                                           bool broadcast) const;

      /**
       * Retrieve delayed operations for account
       * @param account                                                 Account ID.
       * @return A list of delayed operation objects.
       */
      vector<delayed_operation_object> get_delayed_operations_for_account(account_id_type account) const;

      //////////////////////////
      // DAS33:               //
      //////////////////////////

      /**
       * @brief Create new das33 project
       *
       * @param authority       authority that is issuing this operation, must be das33_administrator
       * @param name            name of a project
       * @param owner           acccount id of project owner
       * @param token           id of a token that will be issued by this project
       * @param discounts       array of discounts for project tokens
       * @param goal_amount     minimum amount of tokens needed for project to be successful
       * @param min_pledge      minimum amount allowed in a pledge
       * @param max_pledge      maximum amount allowed in a pledge
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction create_das33_project(const string& authority,
                                              const string& name,
                                              const string& owner,
                                              const string& token,
                                              vector<pair<string, share_type>> discounts,
                                              share_type goal_amount,
                                              share_type min_pledge,
                                              share_type max_pledge,
                                              bool broadcast) const;

      /**
       * @brief Update exsisting das33 project
       *
       * @param authority       authority that is issuing this operation, must be das33_administrator
       * @param project_id      id of a project to edit
       * @param name            optional new name of a project
       * @param owner           optional new project owner
       * @param goal_amount     optional new minimum amount
       * @param toke_price      optional new token price
       * @param discounts       optional new array of discounts for project tokens
       * @param min_pledge      optional new minimum amount allowed in a pledge
       * @param max_pledge      optional new maximum amount allowed in a pledge
       * @param phase_limit     optional phase limit amount
       * @param phase_end       optional time point until phase lasts
       * @param status          optional status of a project
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction update_das33_project(const string& authority,
                                              const string& project_id,
                                              optional<string> name,
                                              optional<string> owner,
                                              optional<share_type> goal_amount,
                                              optional<price> token_price,
                                              optional<vector<pair<string, share_type>>> discounts,
                                              optional<share_type> min_pledge,
                                              optional<share_type> max_pledge,
                                              optional<share_type> phase_limit,
                                              optional<time_point_sec> phase_end,
                                              optional<uint8_t> status,
                                              bool broadcast) const;

      /**
       * @brief Delete a das33 project
       *
       * @param authority       authority that is issuing this operation, must be das33_administrator
       * @param project_id      id of a project to delete
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction delete_das33_project(const string& authority,
                                              const string& project_id,
                                              bool broadcast) const;

      /**
       * @brief Get das33 projects ordered by project name
       *
       * @param lower_bound_name name of the first project
       * @param limit            the number of projects to return (max 1000)
       * @returns                a list of das33 project objects
       */
      vector<das33_project_object> get_das33_projects(const string& lower_bound_name, uint32_t limit) const;

      /**
       * Pledge some asset to das33 project
       * @param account         Account name or id
       * @param amount          Amount of asset to pledge
       * @param symbol          Symbol of asset to pledge
       * @param license         License to transfer asset from (Optional - Not null when pledging cycles, null otherwise)
       * @param project         Project id
       * @param broadcast       True to broadcast the transaction on the network.
       */
      signed_transaction das33_pledge_asset(const string& account,
                                            const string& amount,
                                            const string& symbol,
                                            optional<license_type_id_type> license,
                                            das33_project_id_type project,
                                            bool broadcast = false) const;

      /**
       * Reject a single pledge
       * @param authority       authority that is issuing this operation, must be das33_administrator
       * @param pledge_id       pledge id
       * @param broadcast       true to broadcast the transaction on the network.
       */
      signed_transaction das33_pledge_reject(const string& authority,
                                             const string& pledge_id,
                                             bool broadcast = false) const;

      /**
       * Distribute assets of a single pledge
       * @param authority        authority that is issuing this operation, must be das33_administrator
       * @param pledge_id        pledge id
       * @param to_escrow        percent of pledged amount to distribute to project owner
       * @param base_to_pledger  percent of expected base tokens to distribute to pledger
       * @param bonus_to_pledger percent of expected bonus tokens to distribute to pledger
       * @param broadcast        true to broadcast the transaction on the network.
       */
      signed_transaction das33_distribute_pledge(const string& authority,
                                                 const string& pledge_id,
                                                 share_type to_escrow,
                                                 share_type base_to_pledger,
                                                 share_type bonus_to_pledger,
                                                 bool broadcast = false) const;

      /**
       * Reject das33 project
       * @param authority       authority that is issuing this operation, must be das33_administrator
       * @param project_id      project id
       * @param broadcast       true to broadcast the transaction on the network.
      */
      signed_transaction das33_project_reject(const string& authority,
                                              const string& project_id,
                                              bool broadcast = false) const;

      /**
       * Distribute assets from a project phase
       * @param authority        authority that is issuing this operation, must be das33_administrator
       * @param project_id       project id
       * @param phase_number     optional project phase number. If not provided, pledges from all phases will be distributed
       * @param to_escrow        percent of pledged amount to distribute to project owner
       * @param base_to_pledger  percent of expected base tokens to distribute to pledger
       * @param bonus_to_pledger percent of expected bonus tokens to distribute to pledger
       * @param broadcast        true to broadcast the transaction on the network.
       */
      signed_transaction das33_distribute_project_pledges(const string& authority,
                                                          const string& project_id,
                                                          optional<share_type> phase_number,
                                                          share_type to_escrow,
                                                          share_type base_to_pledger,
                                                          share_type bonus_to_pledger,
                                                          bool broadcast = false) const;

      /**
       * Sets value of use_external_btc_price flag
       * @param authority               authority that is issuing this operation, must be das33_administrator
       * @param use_exteranl_btc_price  new value for flag
       * @param broadcast               true to broadcast the transaction on the network.
       */
      signed_transaction das33_set_use_external_btc_price (const string& authority,
                                                           bool use_exteranl_btc_price,
                                                           bool broadcast = false) const;

      /**
       * Sets value of use_market_token_price array
       * @param authority               authority that is issuing this operation, must be das33_administrator
       * @param use_mnarket_token_price new value for array
       * @param broadcast               true to broadcast the transaction on the network.
       */
      signed_transaction das33_set_use_market_token_price (const string& authority,
                                                           vector<asset_id_type> use_mnarket_token_price,
                                                           bool broadcast = false) const;

      /**
       * @brief Return a part of the pledges table.
       *
       * @param from            id of the pledge
       * @param limit           the number of entries to return (starting from the most recent) (max 100)
       * @returns               a list of pledge holder objects.
       */
      vector<das33_pledge_holder_object> get_das33_pledges(das33_pledge_holder_id_type from, uint32_t limit) const;

      /**
       * @brief Return a list of pledges for specified account.
       *
       * @param account         name or id of the account
       * @returns               a list of pledge holder objects.
       */
      das33_pledges_by_account_result get_das33_pledges_by_account(const string& account) const;

      /**
       * @brief Return a list of pledges for specified project.
       *
       * @param project         name or id of das33 project
       * @param from            id of the first pledge
       * @param limit           the number of entries to return (starting from the most recent) (max 100)
       * @returns               a list of pledge holder objects.
       */
      vector<das33_pledge_holder_object> get_das33_pledges_by_project(const string& project, das33_pledge_holder_id_type from, uint32_t limit) const;

      /**
      * @brief Gets a sum of all pledges made to project
      * @param project id of a project
      * @return vector of assets, each with total sum of that asset pledged
      */
      vector<asset> get_amount_of_assets_pledged_to_project(das33_project_id_type project) const;

      /**
      * @brief Gets the amount of project tokens that a pledger can get for pledging a certain amount of asset
      * @param project id of a project
      * @param to_pledge asset user is pledging
      * @return amount of project tokens to get
      */
      das33_project_tokens_amount get_amount_of_project_tokens_received_for_asset(das33_project_id_type project, asset to_pledge) const;

      /**
      * @brief Gets the amount of assets needed to be pledge to get given amount of base project tokens
      * @param project id of a project
      * @param asset_id id of an asset user wants to get amount for
      * @param to_pledge project token user wants to get
      * @return amount of project tokens to get
      */
      das33_project_tokens_amount get_amount_of_asset_needed_for_project_token(das33_project_id_type project, asset_id_type asset_id, asset tokens) const;

      //////////////////////////
      // Prices:              //
      //////////////////////////

      /**
      * @brief Gets all last prices from index
      * @return Vector of last price objects
      */
      vector<last_price_object> get_last_prices() const;

      /**
      * @brief Gets all external prices from index
      * @return Vector of external price objects
      */
      vector<external_price_object> get_external_prices() const;

      //////////////////////////
      // GLOBALS:             //
      //////////////////////////

      /**
       * @param authority       This MUST be root authority.
       * @param changed_values  The values to change; all other chain parameters are filled in with default values
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction update_global_parameters(const string& authority,
                                                  const variant_object& changed_values,
                                                  bool broadcast) const;

      /**
       * @param authority       This MUST be root authority.
       * @param new_fee         New operation fee
       * @param op_num          The operation id whose fee we are changing
       * @param string          Comment
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction change_operation_fee(const string& authority,
                                              share_type new_fee,
                                              unsigned op_num,
                                              string comment,
                                              bool broadcast) const;

      /**
       * @param btc_issuer      Account of BTC issuer
       * @param new_price       New BTC price
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction update_external_btc_price(const string& btc_issuer,
                                                   price new_price,
                                                   bool broadcast) const;

      /**
       * @param token_issuer    Account of asset issuer
       * @param token_id        Id of asset to set price for
       * @param new_price       New token price
       * @param broadcast       true to broadcast transaction to network
       */
      signed_transaction update_external_token_price(const string& token_issuer,
                                                   asset_id_type token_id,
                                                   price new_price,
                                                   bool broadcast) const;

      //////////////////////////
      // REQUESTS:            //
      //////////////////////////

      /**
       * @brief Get the size of the DASCoin reward queue.
       * @return Number of elements in the DASCoin queue.
       */
      uint32_t get_reward_queue_size() const;

      /**
       * @brief Get all webasset issue request objects, sorted by expiration.
       * @return Vector of webasset issue request objects.
       */
      vector<issue_asset_request_object> get_all_webasset_issue_requests() const;

      /**
       * @brief Get all wire out holder objects.
       * @return Vector of wire out holder objects.
       */
      vector<wire_out_holder_object> get_all_wire_out_holders() const;

      /**
      * @brief Get all wire out holder objects.
      * @return Vector of wire out holder objects.
       */
      vector<wire_out_with_fee_holder_object> get_all_wire_out_with_fee_holders() const;

      /**
       * @brief Return the entire reward queue.
       * @return Vector of all reward queue objects.
       */
      vector<reward_queue_object> get_reward_queue() const;

      /**
       * @brief Return a part of the reward queue.
       * @return Vector of reward queue objects.
       */
      vector<reward_queue_object> get_reward_queue_by_page(uint32_t from, uint32_t amount) const;

      /**
       * Get all current submissions to reward queue by account id.
       *
       * @param account_id Id of account whose submissions should be returned.
       * @return           All elements on DasCoin reward queue submitted by given account.
       */
      acc_id_queue_subs_w_pos_res get_queue_submissions_with_pos(account_id_type account_id) const;

      void dbg_make_uia(string creator, string symbol);
      void dbg_make_mia(string creator, string symbol);
      void dbg_push_blocks( std::string src_filename, uint32_t count );
      void dbg_generate_blocks( std::string debug_wif_key, uint32_t count );
      void dbg_stream_json_objects( const std::string& filename );
      void dbg_update_object( fc::variant_object update );

      void flood_network(string prefix, uint32_t number_of_transactions);



  /**
       * Connect to a new peer
       *
       * @param nodes List of the IP addresses and ports of new nodes
       */
      void network_add_nodes( const vector<string>& nodes );
      
      /**
       * Get status of all current connections to peers
       */
      vector< variant > network_get_connected_peers();

      /**
       *  Used to transfer from one set of blinded balances to another
       */
      blind_confirmation blind_transfer_help( string from_key_or_label,
                                         string to_key_or_label,
                                         string amount,
                                         string symbol,
                                         bool broadcast = false,
                                         bool to_temp = false );


      std::map<string,std::function<string(fc::variant,const fc::variants&)>> get_result_formatters() const;

      fc::signal<void(bool)> lock_changed;
      std::shared_ptr<detail::wallet_api_impl> my;
      void encrypt_keys();
};

} }

FC_REFLECT( graphene::wallet::key_label, (label)(key) )
FC_REFLECT( graphene::wallet::blind_balance, (amount)(from)(to)(one_time_key)(blinding_factor)(commitment)(used) )
FC_REFLECT( graphene::wallet::blind_confirmation::output, (label)(pub_key)(decrypted_memo)(confirmation)(auth)(confirmation_receipt) )
FC_REFLECT( graphene::wallet::blind_confirmation, (trx)(outputs) )

FC_REFLECT( graphene::wallet::plain_keys, (keys)(checksum) )

FC_REFLECT( graphene::wallet::wallet_data,
            (chain_id)
            (my_accounts)
            (cipher_keys)
            (extra_keys)
            (pending_account_registrations)(pending_witness_registrations)
            (labeled_keys)
            (blind_receipts)
            (ws_server)
            (ws_user)
            (ws_password)
          )

FC_REFLECT( graphene::wallet::brain_key_info,
            (brain_priv_key)
            (wif_priv_key)
            (pub_key)
          )

FC_REFLECT( graphene::wallet::exported_account_keys, (account_name)(encrypted_private_keys)(public_keys) )

FC_REFLECT( graphene::wallet::exported_keys, (password_checksum)(account_keys) )

FC_REFLECT( graphene::wallet::blind_receipt,
            (date)(from_key)(from_label)(to_key)(to_label)(amount)(memo)(control_authority)(data)(used)(conf) )

FC_REFLECT( graphene::wallet::approval_delta,
   (active_approvals_to_add)
   (active_approvals_to_remove)
   (owner_approvals_to_add)
   (owner_approvals_to_remove)
   (key_approvals_to_add)
   (key_approvals_to_remove)
)

FC_REFLECT( graphene::wallet::worker_vote_delta,
   (vote_for)
   (vote_against)
   (vote_abstain)
)

FC_REFLECT_DERIVED( graphene::wallet::signed_block_with_info, (graphene::chain::signed_block),
   (block_id)(signing_key)(transaction_ids) )

FC_REFLECT_DERIVED( graphene::wallet::vesting_balance_object_with_info, (graphene::chain::vesting_balance_object),
   (allowed_withdraw)(allowed_withdraw_time) )

FC_REFLECT( graphene::wallet::operation_detail,
            (memo)(description)(op) )

FC_API( graphene::wallet::wallet_api,
        (help)
        (gethelp)
        (info)
        (about)
        (begin_builder_transaction)
        (add_operation_to_builder_transaction)
        (replace_operation_in_builder_transaction)
        (set_fees_on_builder_transaction)
        (preview_builder_transaction)
        (sign_builder_transaction)
        (propose_builder_transaction)
        (propose_builder_transaction2)
        (remove_builder_transaction)
        (is_new)
        (is_locked)
        (lock)(unlock)(set_password)
        (dump_private_keys)
        (list_my_accounts)
        (list_accounts)
        (list_account_balances)
        (list_assets)
        (get_license_types)
        (import_key)
        (import_accounts)
        (import_account_keys)
        (import_balance)
        (suggest_brain_key)
        (derive_owner_keys_from_brain_key)
        (register_account)
        (create_account)
        (tether_accounts)
        (register_vault_account)
        (upgrade_account)
        (create_account_with_brain_key)
        (sell_asset)
        (sell)
        (buy)
        (borrow_asset)
        (cancel_order)
        (transfer)
        (transfer2)
        (transfer_vault_to_wallet)
        (get_transaction_id)
        (create_asset)
        (update_asset)
        (update_bitasset)
        (update_asset_feed_producers)
        (publish_asset_feed)
        (issue_asset)
        (get_asset)
        (get_bitasset_data)
        (fund_asset_fee_pool)
        (claim_asset_accumulated_fees_pool)
        (reserve_asset)
        (global_settle_asset)
        (settle_asset)
        (whitelist_account)
        (create_committee_member)
        (get_witness)
        (get_committee_member)
        (list_witnesses)
        (list_committee_members)
        (create_witness)
        (update_witness)
        (create_worker)
        (update_worker_votes)
        (get_vesting_balances)
        (withdraw_vesting)
        (vote_for_committee_member)
        (vote_for_witness)
        (set_voting_proxy)
        (set_desired_witness_and_committee_member_count)
        (get_account)
        (get_account_id)
        (get_block)
        (get_account_count)
        (get_account_history)
        (get_account_history_by_operation)
        (get_account_history_by_operation2)
        (get_market_history)
        (get_global_properties)
        (get_dynamic_global_properties)
        (get_object)
        (get_private_key)
        (load_wallet_file)
        (normalize_brain_key)
        (get_limit_orders)
        (get_call_orders)
        (get_settle_orders)
        (save_wallet_file)
        (serialize_transaction)
        (sign_transaction)
        (sign_transaction_with_keys)
        (get_prototype_operation)
        (propose_parameter_change)
        (propose_fee_change)
        (approve_proposal)
        (dbg_make_uia)
        (dbg_make_mia)
        (dbg_push_blocks)
        (dbg_generate_blocks)
        (dbg_stream_json_objects)
        (dbg_update_object)
        (flood_network)
        (network_add_nodes)
        (network_get_connected_peers)
        (sign_memo)
        (read_memo)
        (set_key_label)
        (get_key_label)
        (get_public_key)
        (get_blind_accounts)
        (get_my_blind_accounts)
        (get_blind_balances)
        (create_blind_account)
        (transfer_to_blind)
        (transfer_from_blind)
        (blind_transfer)
        (blind_history)
        (receive_blind_transfer)
        // Licenses:
        (issue_license)
        (submit_cycles_to_queue_by_license)
        (get_license_information)
        (get_license_type_names_ids)

        // Web assets:
        (issue_webasset)

        // Cycles:
        (get_total_cycles)
        (get_account_cycle_balance)
        (get_full_cycle_balances)
        (get_dascoin_balance)
        (get_order_book)
        (update_queue_parameters)
        (wire_out)
        (transfer_cycles_from_licence_to_wallet)
        (purchase_cycle_asset)
        (calculate_cycle_price)

        // DasPay:
        (set_daspay_transaction_ratio)
        (create_payment_service_provider)
        (update_payment_service_provider)
        (delete_payment_service_provider)
        (get_payment_service_providers)
        (register_daspay_authority)
        (unregister_daspay_authority)
        (reserve_asset_on_account)
        (unreserve_asset_on_account)
        (daspay_debit_account)
        (daspay_credit_account)
        (get_daspay_authority_for_account)
        (update_daspay_clearing_parameters)

        // Das33
        (das33_pledge_asset)
        (get_das33_pledges)
        (get_das33_pledges_by_account)
        (get_das33_pledges_by_project)
        (das33_pledge_reject)
        (das33_distribute_pledge)
        (das33_project_reject)
        (das33_distribute_project_pledges)
        (create_das33_project)
        (update_das33_project)
        (delete_das33_project)
        (get_das33_projects)
        (get_amount_of_assets_pledged_to_project)
        (get_amount_of_project_tokens_received_for_asset)
        (get_amount_of_asset_needed_for_project_token)
        (das33_set_use_external_btc_price)
        (das33_set_use_market_token_price)

        // Prices
        (get_last_prices)
        (get_external_prices)

        // Delayed operations resolver:
        (update_delayed_operations_resolver_parameters)
        (get_delayed_operations_for_account)

        (update_global_parameters)
        (change_operation_fee)
        (update_external_btc_price)
        (update_external_token_price)

        // Requests:
        (get_all_webasset_issue_requests)
        (get_all_wire_out_holders)
        // Queue:
        (get_reward_queue)
        (get_reward_queue_by_page)
        (get_reward_queue_size)
        (get_queue_submissions_with_pos)

        (set_chain_authority)
      )
