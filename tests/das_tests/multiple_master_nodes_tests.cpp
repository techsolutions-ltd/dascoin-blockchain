#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/witness_schedule_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( multiple_master_tests, database_fixture )

BOOST_AUTO_TEST_CASE( successful_removal_of_root_authority_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   remove_root_authority_operation rrao;
   rrao.root_account = root_id;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   do_op(rrao);

   BOOST_CHECK( !db.get_dynamic_global_properties().is_root_authority_enabled_flag );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( successful_creation_update_and_removal_of_witness_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   const string name("witnesss1");
   const fc::ecc::private_key new_key = fc::ecc::private_key::generate();
   const public_key_type key = new_key.get_public_key();

   auto ao = make_new_account_base( account_kind::special, db.get_chain_authorities().registrar, name, key );

   const auto& witnesses_bi = db.get_index_type<witness_index>().indices().get<by_id>();
   size_t count = witnesses_bi.size();

   create_witness_operation cwao;
   cwao.authority = root_id;
   cwao.block_signing_key = key;
   cwao.witness_account = ao.id;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   do_op(cwao);

   const auto& idx = db.get_index_type<witness_index>().indices().get<by_account>();
   auto itr = idx.find(ao.id);
   // we check if we have created witness sucessfuly
   BOOST_CHECK(itr != idx.end());

   const auto& witnesses_by_id = db.get_index_type<witness_index>().indices().get<by_id>();
   size_t st = witnesses_by_id.size();
   BOOST_CHECK( st == count + 1);

   const witness_object& wobj = *itr;

   const fc::ecc::private_key new_key1 = fc::ecc::private_key::generate();
   public_key_type key1 = new_key.get_public_key();

   update_witness_operation uwao;
   uwao.authority = root_id;
   uwao.witness = wobj.id;
   uwao.url = "pera";
   uwao.block_signing_key = key1;
   do_op(uwao);

   const auto& idx1 = db.get_index_type<witness_index>().indices().get<by_account>();
   auto itr1 = idx1.find(ao.id);
   // we check if we have updated witness sucessfuly
   BOOST_CHECK(itr1 != idx1.end());

   const witness_object& wobj1 = *itr1;
   // check if we have updated witness sucessfuly
   BOOST_CHECK(wobj1.signing_key == key1);


   remove_witness_operation rwao;
   rwao.authority = root_id;
   rwao.witness = wobj1.id;
   do_op(rwao);

   const auto& idx2 = db.get_index_type<witness_index>().indices().get<by_account>();
   auto itr2 = idx2.find(ao.id);
   // check if we have removed witness sucessfuly
   BOOST_CHECK(itr2 == idx2.end());


} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( successful_activate_deactivate_witness_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   const string name("witnesss2");
   const fc::ecc::private_key new_key = fc::ecc::private_key::generate();
   const public_key_type key = new_key.get_public_key();

   auto ao = make_new_account_base( account_kind::special, db.get_chain_authorities().registrar, name, key );

   create_witness_operation cwao;
   cwao.authority = root_id;
   cwao.block_signing_key = key;
   cwao.witness_account = ao.id;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   do_op(cwao);

   const auto& idx = db.get_index_type<witness_index>().indices().get<by_account>();
   auto itr = idx.find(ao.id);
   // we check if we have created witness sucessfuly
   BOOST_CHECK(itr != idx.end());

   witness_object wobj = *itr;

   activate_witness_operation awao;
   awao.authority = root_id;
   awao.witness = wobj.id;

   auto old_size = get_global_properties().active_witnesses.size();
   do_op(awao);
   auto size = get_global_properties().active_witnesses.size();
   BOOST_CHECK(size == old_size + 1);

   // this is check if witnes is in schedule queue
   const witness_schedule_object& wso = witness_schedule_id_type()(db);
   bool contains_activated_witness = false;
   for(auto& w : wso.current_shuffled_witnesses)
   {
      if(w.instance == awao.witness.instance)
      {
         contains_activated_witness = true;
         break;
      }
   }
   BOOST_CHECK(contains_activated_witness);


} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
