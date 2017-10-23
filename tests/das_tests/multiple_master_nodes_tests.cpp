#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/market_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( exchange_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( successful_removal_of_root_authority_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   remove_root_authority_operation rrao;
   rrao.root_account = root_id;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   do_op(rrao);

   BOOST_CHECK( !db.get_dynamic_global_properties().is_root_authority_enabled_flag );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
