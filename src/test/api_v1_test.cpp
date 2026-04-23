#include <test/api_v1_test.hpp>

TEST(wigwag_api_v1_test, signals) { wigwag_api_v1_test::test_signals(); }
TEST(wigwag_api_v1_test, signal_connector) { wigwag_api_v1_test::test_signal_connector(); }
TEST(wigwag_api_v1_test, listenable) { wigwag_api_v1_test::test_listenable(); }
TEST(wigwag_api_v1_test, token) { wigwag_api_v1_test::test_token(); }
TEST(wigwag_api_v1_test, token_pool) { wigwag_api_v1_test::test_token_pool(); }
TEST(wigwag_api_v1_test, connect_from_handler) { wigwag_api_v1_test::test_connect_from_handler(); }
TEST(wigwag_api_v1_test, disconnect_from_handler) { wigwag_api_v1_test::test_disconnect_from_handler(); }
TEST(wigwag_api_v1_test, signal_attributes) { wigwag_api_v1_test::test_signal_attributes(); }
TEST(wigwag_api_v1_test, handler_attributes) { wigwag_api_v1_test::test_handler_attributes(); }
TEST(wigwag_api_v1_test, exception_handling_default) { wigwag_api_v1_test::test__exception_handling__default(); }
TEST(wigwag_api_v1_test, exception_handling_none) { wigwag_api_v1_test::test__exception_handling__none(); }
TEST(wigwag_api_v1_test, exception_handling_print_to_stderr) { wigwag_api_v1_test::test__exception_handling__print_to_stderr(); }
TEST(wigwag_api_v1_test, state_populating_default) { wigwag_api_v1_test::test__state_populating__default(); }
TEST(wigwag_api_v1_test, state_populating_populator_only) { wigwag_api_v1_test::test__state_populating__populator_only(); }
TEST(wigwag_api_v1_test, state_populating_populator_and_withdrawer) { wigwag_api_v1_test::test__state_populating__populator_and_withdrawer(); }
TEST(wigwag_api_v1_test, state_populating_none) { wigwag_api_v1_test::test__state_populating__none(); }
TEST(wigwag_api_v1_test, life_assurance_default) { wigwag_api_v1_test::test__life_assurance__default(); }
TEST(wigwag_api_v1_test, life_assurance_intrusive_life_tokens) { wigwag_api_v1_test::test__life_assurance__intrusive_life_tokens(); }
TEST(wigwag_api_v1_test, life_assurance_single_threaded) { wigwag_api_v1_test::test__life_assurance__single_threaded(); }
TEST(wigwag_api_v1_test, life_assurance_none) { wigwag_api_v1_test::test__life_assurance__none(); }
TEST(wigwag_api_v1_test, creation_ahead_of_time) { wigwag_api_v1_test::test__creation__ahead_of_time(); }
TEST(wigwag_api_v1_test, creation_lazy) { wigwag_api_v1_test::test__creation__lazy(); }
TEST(wigwag_api_v1_test, life_token) { wigwag_api_v1_test::test_life_token(); }
TEST(wigwag_api_v1_test, task_executors) { wigwag_api_v1_test::test_task_executors(); }
TEST(wigwag_api_v1_test, task_executor_function_copying) { wigwag_api_v1_test::test_task_executor_function_copying(); }
TEST(wigwag_api_v1_test, signal_handler_copying) { wigwag_api_v1_test::test_signal_handler_copying(); }
TEST(wigwag_api_v1_test, signal_parameters_copying) { wigwag_api_v1_test::test_signal_parameters_copying(); }

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
