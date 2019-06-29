#pragma once

#include <stdint.h>
#include "c_api_types.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableAliceDataNew(char const *publish_path);
    EXPORT handle_t E_TableBobDataNew(char const *bulletin_file,
                                      char const *public_path);

    EXPORT bool E_TableAliceDataFree(handle_t c_alice_data);
    EXPORT bool E_TableBobDataFree(handle_t c_bob_data);

    EXPORT bool E_TableAliceBulletin(handle_t c_alice_data,
                                     table_bulletin_t *bulletin);
    EXPORT bool E_TableBobBulletin(handle_t c_bob_data, table_bulletin_t *bulletin);

    EXPORT bool E_TableBIsKeyUnique(handle_t c_bob_data, char const *query_key,
                                    bool *unique);
#ifdef __cplusplus
}
#endif

// complaint
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableComplaintAliceNew(handle_t c_alice_data,
                                             uint8_t const *c_self_id,
                                             uint8_t const *c_peer_id);

    EXPORT bool E_TableComplaintAliceOnRequest(handle_t c_alice,
                                               char const *request_file,
                                               char const *response_file);

    EXPORT bool E_TableComplaintAliceOnReceipt(handle_t c_alice,
                                               char const *receipt_file,
                                               char const *secret_file);

    EXPORT bool E_TableComplaintAliceSetEvil(handle_t c_alice);

    EXPORT bool E_TableComplaintAliceFree(handle_t c_alice);

    EXPORT handle_t E_TableComplaintBobNew(handle_t c_bob_data,
                                           uint8_t const *c_self_id,
                                           uint8_t const *c_peer_id,
                                           range_t const *c_demand,
                                           uint64_t c_demand_count);

    EXPORT bool E_TableComplaintBobGetRequest(handle_t c_bob,
                                              char const *request_file);

    EXPORT bool E_TableComplaintBobOnResponse(handle_t c_bob,
                                              char const *response_file,
                                              char const *receipt_file);

    EXPORT bool E_TableComplaintBobOnSecret(handle_t c_bob,
                                            char const *secret_file);

    EXPORT bool E_TableComplaintBobGenerateClaim(handle_t c_bob,
                                                 char const *claim_file);

    EXPORT bool E_TableComplaintBobSaveDecrypted(handle_t c_bob, char const *file);

    EXPORT bool E_TableComplaintBobFree(handle_t c_bob);
#ifdef __cplusplus
} // extern "C" complaint
#endif

// atomic_swap
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableAtomicSwapAliceNew(handle_t c_alice_data,
                                              uint8_t const *c_self_id,
                                              uint8_t const *c_peer_id);

    EXPORT bool E_TableAtomicSwapAliceOnRequest(handle_t c_alice,
                                                char const *request_file,
                                                char const *response_file);

    EXPORT bool E_TableAtomicSwapAliceOnReceipt(handle_t c_alice,
                                                char const *receipt_file,
                                                char const *secret_file);

    EXPORT bool E_TableAtomicSwapAliceSetEvil(handle_t c_alice);

    EXPORT bool E_TableAtomicSwapAliceFree(handle_t c_alice);

    EXPORT handle_t E_TableAtomicSwapBobNew(handle_t c_bob_data,
                                            uint8_t const *c_self_id,
                                            uint8_t const *c_peer_id,
                                            range_t const *c_demand,
                                            uint64_t c_demand_count);

    EXPORT bool E_TableAtomicSwapBobGetRequest(handle_t c_bob,
                                               char const *request_file);

    EXPORT bool E_TableAtomicSwapBobOnResponse(handle_t c_bob,
                                               char const *response_file,
                                               char const *receipt_file);

    EXPORT bool E_TableAtomicSwapBobOnSecret(handle_t c_bob,
                                             char const *secret_file);

    EXPORT bool E_TableAtomicSwapBobSaveDecrypted(handle_t c_bob, char const *file);

    EXPORT bool E_TableAtomicSwapBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" atomic_swap
#endif

// ot_complaint
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableOtComplaintAliceNew(handle_t c_alice_data,
                                               uint8_t const *c_self_id,
                                               uint8_t const *c_peer_id);

    EXPORT bool E_TableOtComplaintAliceGetNegoRequest(handle_t c_alice,
                                                      char const *request_file);

    EXPORT bool E_TableOtComplaintAliceOnNegoRequest(handle_t c_alice,
                                                     char const *request_file,
                                                     char const *response_file);

    EXPORT bool E_TableOtComplaintAliceOnNegoResponse(handle_t c_alice,
                                                      char const *response_file);

    EXPORT bool E_TableOtComplaintBobGetNegoRequest(handle_t c_bob,
                                                    char const *request_file);

    EXPORT bool E_TableOtComplaintBobOnNegoRequest(handle_t c_bob,
                                                   char const *request_file,
                                                   char const *response_file);

    EXPORT bool E_TableOtComplaintBobOnNegoResponse(handle_t c_bob,
                                                    char const *response_file);

    EXPORT bool E_TableOtComplaintAliceOnRequest(handle_t c_alice,
                                                 char const *request_file,
                                                 char const *response_file);

    EXPORT bool E_TableOtComplaintAliceOnReceipt(handle_t c_alice,
                                                 char const *receipt_file,
                                                 char const *secret_file);

    EXPORT bool E_TableOtComplaintAliceSetEvil(handle_t c_alice);

    EXPORT bool E_TableOtComplaintAliceFree(handle_t c_alice);

    EXPORT handle_t E_TableOtComplaintBobNew(
        handle_t c_bob_data, uint8_t const *c_self_id, uint8_t const *c_peer_id,
        range_t const *c_demand, uint64_t c_demand_count, range_t const *c_phantom,
        uint64_t c_phantom_count);

    EXPORT bool E_TableOtComplaintBobGetRequest(handle_t c_bob,
                                                char const *request_file);

    EXPORT bool E_TableOtComplaintBobOnResponse(handle_t c_bob,
                                                char const *response_file,
                                                char const *receipt_file);

    EXPORT bool E_TableOtComplaintBobOnSecret(handle_t c_bob,
                                              char const *secret_file);

    EXPORT bool E_TableOtComplaintBobGenerateClaim(handle_t c_bob,
                                                   char const *claim_file);

    EXPORT bool E_TableOtComplaintBobSaveDecrypted(handle_t c_bob,
                                                   char const *file);

    EXPORT bool E_TableOtComplaintBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" ot_complaint
#endif

// ot_vrfq
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableOtVrfqAliceNew(handle_t c_alice_data,
                                          uint8_t const *c_self_id,
                                          uint8_t const *c_peer_id);

    EXPORT bool E_TableOtVrfqAliceGetNegoRequest(handle_t c_alice,
                                                 char const *request_file);

    EXPORT bool E_TableOtVrfqAliceOnNegoRequest(handle_t c_alice,
                                                char const *request_file,
                                                char const *response_file);

    EXPORT bool E_TableOtVrfqAliceOnNegoResponse(handle_t c_alice,
                                                 char const *response_file);

    EXPORT bool E_TableOtVrfqAliceOnRequest(handle_t c_alice,
                                            char const *request_file,
                                            char const *response_file);

    EXPORT bool E_TableOtVrfqAliceOnReceipt(handle_t c_alice,
                                            char const *receipt_file,
                                            char const *secret_file);

    EXPORT bool E_TableOtVrfqAliceFree(handle_t c_alice);

    EXPORT handle_t
    E_TableOtVrfqBobNew(handle_t c_bob_data, uint8_t const *c_self_id,
                        uint8_t const *c_peer_id, char const *c_query_key,
                        char const *c_query_values[], uint64_t c_query_value_count,
                        char const *c_phantoms[], uint64_t c_phantom_count);

    EXPORT bool E_TableOtVrfqBobGetNegoRequest(handle_t c_bob,
                                               char const *request_file);

    EXPORT bool E_TableOtVrfqBobOnNegoRequest(handle_t c_bob,
                                              char const *request_file,
                                              char const *response_file);

    EXPORT bool E_TableOtVrfqBobOnNegoResponse(handle_t c_bob,
                                               char const *response_file);

    EXPORT bool E_TableOtVrfqBobGetRequest(handle_t c_bob,
                                           char const *request_file);

    EXPORT bool E_TableOtVrfqBobOnResponse(handle_t c_bob,
                                           char const *response_file,
                                           char const *receipt_file);

    EXPORT bool E_TableOtVrfqBobOnSecret(handle_t c_bob, char const *secret_file,
                                         char const *positions_file);

    EXPORT bool E_TableOtVrfqBobFree(handle_t c_bob);
#ifdef __cplusplus
} // extern "C" ot_vrfq
#endif

// vrfq
#ifdef __cplusplus
extern "C"
{
#endif

    EXPORT handle_t E_TableVrfqAliceNew(handle_t c_alice_data,
                                        uint8_t const *c_self_id,
                                        uint8_t const *c_peer_id);

    EXPORT bool E_TableVrfqAliceOnRequest(handle_t c_alice,
                                          char const *request_file,
                                          char const *response_file);

    EXPORT bool E_TableVrfqAliceOnReceipt(handle_t c_alice,
                                          char const *receipt_file,
                                          char const *secret_file);

    EXPORT bool E_TableVrfqAliceFree(handle_t c_alice);

    EXPORT handle_t E_TableVrfqBobNew(handle_t c_bob_data, uint8_t const *c_self_id,
                                      uint8_t const *c_peer_id,
                                      char const *c_query_key,
                                      char const *c_query_values[],
                                      uint64_t c_query_value_count);

    EXPORT bool E_TableVrfqBobGetRequest(handle_t c_bob, char const *request_file);

    EXPORT bool E_TableVrfqBobOnResponse(handle_t c_bob, char const *response_file,
                                         char const *receipt_file);

    EXPORT bool E_TableVrfqBobOnSecret(handle_t c_bob, char const *secret_file,
                                       char const *positions_file);

    EXPORT bool E_TableVrfqBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" vrfq
#endif

// atomic_swap_vc
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_TableAtomicSwapVcAliceNew(handle_t c_alice_data,
                                                uint8_t const *c_self_id,
                                                uint8_t const *c_peer_id);

    EXPORT bool E_TableAtomicSwapVcAliceOnRequest(handle_t c_alice,
                                                  char const *request_file,
                                                  char const *response_file);

    EXPORT bool E_TableAtomicSwapVcAliceOnReceipt(handle_t c_alice,
                                                  char const *receipt_file,
                                                  char const *secret_file);

    EXPORT bool E_TableAtomicSwapVcAliceSetEvil(handle_t c_alice);

    EXPORT bool E_TableAtomicSwapVcAliceFree(handle_t c_alice);

    EXPORT handle_t E_TableAtomicSwapVcBobNew(handle_t c_bob_data,
                                              uint8_t const *c_self_id,
                                              uint8_t const *c_peer_id,
                                              range_t const *c_demand,
                                              uint64_t c_demand_count);

    EXPORT bool E_TableAtomicSwapVcBobGetRequest(handle_t c_bob,
                                                 char const *request_file);

    EXPORT bool E_TableAtomicSwapVcBobOnResponse(handle_t c_bob,
                                                 char const *response_file,
                                                 char const *receipt_file);

    EXPORT bool E_TableAtomicSwapVcBobOnSecret(handle_t c_bob,
                                               char const *secret_file);

    EXPORT bool E_TableAtomicSwapVcBobSaveDecrypted(handle_t c_bob,
                                                    char const *file);

    EXPORT bool E_TableAtomicSwapVcBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" atomic_swap_vc
#endif
