#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

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
    EXPORT handle_t E_PlainAliceDataNew(char const *publish_path);
    EXPORT handle_t E_PlainBobDataNew(char const *bulletin_file,
                                      char const *public_path);

    EXPORT bool E_PlainAliceDataFree(handle_t c_alice_data);
    EXPORT bool E_PlainBobDataFree(handle_t c_bob_data);

    EXPORT bool E_PlainAliceBulletin(handle_t c_alice_data,
                                     plain_bulletin_t *bulletin);
    EXPORT bool E_PlainBobBulletin(handle_t c_bob_data, plain_bulletin_t *bulletin);

#ifdef __cplusplus
}
#endif

// complaint
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_PlainComplaintAliceNew(handle_t c_alice_data,
                                             uint8_t const *c_self_id,
                                             uint8_t const *c_peer_id);

    EXPORT bool E_PlainComplaintAliceOnRequest(handle_t c_alice,
                                               char const *request_file,
                                               char const *response_file);

    EXPORT bool E_PlainComplaintAliceOnReceipt(handle_t c_alice,
                                               char const *receipt_file,
                                               char const *secret_file);

    EXPORT bool E_PlainComplaintAliceSetEvil(handle_t c_alice);

    EXPORT bool E_PlainComplaintAliceFree(handle_t c_alice);

    EXPORT handle_t E_PlainComplaintBobNew(handle_t c_bob_data,
                                           uint8_t const *c_self_id,
                                           uint8_t const *c_peer_id,
                                           range_t const *c_demand,
                                           uint64_t c_demand_count);

    EXPORT bool E_PlainComplaintBobGetRequest(handle_t c_bob,
                                              char const *request_file);

    EXPORT bool E_PlainComplaintBobOnResponse(handle_t c_bob,
                                              char const *response_file,
                                              char const *receipt_file);

    EXPORT bool E_PlainComplaintBobOnSecret(handle_t c_bob,
                                            char const *secret_file);

    EXPORT bool E_PlainComplaintBobGenerateClaim(handle_t c_bob,
                                                 char const *claim_file);

    EXPORT bool E_PlainComplaintBobSaveDecrypted(handle_t c_bob, char const *file);

    EXPORT bool E_PlainComplaintBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" complaint
#endif

// atomic_swap
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_PlainAtomicSwapAliceNew(handle_t c_alice_data,
                                              uint8_t const *c_self_id,
                                              uint8_t const *c_peer_id);

    EXPORT bool E_PlainAtomicSwapAliceOnRequest(handle_t c_alice,
                                                char const *request_file,
                                                char const *response_file);

    EXPORT bool E_PlainAtomicSwapAliceOnReceipt(handle_t c_alice,
                                                char const *receipt_file,
                                                char const *secret_file);

    EXPORT bool E_PlainAtomicSwapAliceSetEvil(handle_t c_alice);

    EXPORT bool E_PlainAtomicSwapAliceFree(handle_t c_alice);

    EXPORT handle_t E_PlainAtomicSwapBobNew(handle_t c_bob_data,
                                            uint8_t const *c_self_id,
                                            uint8_t const *c_peer_id,
                                            range_t const *c_demand,
                                            uint64_t c_demand_count);

    EXPORT bool E_PlainAtomicSwapBobGetRequest(handle_t c_bob,
                                               char const *request_file);

    EXPORT bool E_PlainAtomicSwapBobOnResponse(handle_t c_bob,
                                               char const *response_file,
                                               char const *receipt_file);

    EXPORT bool E_PlainAtomicSwapBobOnSecret(handle_t c_bob,
                                             char const *secret_file);

    EXPORT bool E_PlainAtomicSwapBobSaveDecrypted(handle_t c_bob, char const *file);

    EXPORT bool E_PlainAtomicSwapBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" atomic_swap
#endif

// ot_complaint
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_PlainOtComplaintAliceNew(handle_t c_alice_data,
                                               uint8_t const *c_self_id,
                                               uint8_t const *c_peer_id);

    EXPORT bool E_PlainOtComplaintAliceGetNegoRequest(handle_t c_alice,
                                                      char const *request_file);

    EXPORT bool E_PlainOtComplaintAliceOnNegoRequest(handle_t c_alice,
                                                     char const *request_file,
                                                     char const *response_file);

    EXPORT bool E_PlainOtComplaintAliceOnNegoResponse(handle_t c_alice,
                                                      char const *response_file);

    EXPORT bool E_PlainOtComplaintBobGetNegoRequest(handle_t c_bob,
                                                    char const *request_file);

    EXPORT bool E_PlainOtComplaintBobOnNegoRequest(handle_t c_bob,
                                                   char const *request_file,
                                                   char const *response_file);

    EXPORT bool E_PlainOtComplaintBobOnNegoResponse(handle_t c_bob,
                                                    char const *response_file);

    EXPORT bool E_PlainOtComplaintAliceOnRequest(handle_t c_alice,
                                                 char const *request_file,
                                                 char const *response_file);

    EXPORT bool E_PlainOtComplaintAliceOnReceipt(handle_t c_alice,
                                                 char const *receipt_file,
                                                 char const *secret_file);

    EXPORT bool E_PlainOtComplaintAliceSetEvil(handle_t c_alice);

    EXPORT bool E_PlainOtComplaintAliceFree(handle_t c_alice);

    EXPORT handle_t E_PlainOtComplaintBobNew(
        handle_t c_bob_data, uint8_t const *c_self_id, uint8_t const *c_peer_id,
        range_t const *c_demand, uint64_t c_demand_count, range_t const *c_phantom,
        uint64_t c_phantom_count);

    EXPORT bool E_PlainOtComplaintBobGetRequest(handle_t c_bob,
                                                char const *request_file);

    EXPORT bool E_PlainOtComplaintBobOnResponse(handle_t c_bob,
                                                char const *response_file,
                                                char const *receipt_file);

    EXPORT bool E_PlainOtComplaintBobOnSecret(handle_t c_bob,
                                              char const *secret_file);

    EXPORT bool E_PlainOtComplaintBobGenerateClaim(handle_t c_bob,
                                                   char const *claim_file);

    EXPORT bool E_PlainOtComplaintBobSaveDecrypted(handle_t c_bob,
                                                   char const *file);

    EXPORT bool E_PlainOtComplaintBobFree(handle_t c_bob);
#ifdef __cplusplus
} // extern "C" ot_complaint
#endif

// atomic_swap_vc
#ifdef __cplusplus
extern "C"
{
#endif
    EXPORT handle_t E_PlainAtomicSwapVcAliceNew(handle_t c_alice_data,
                                                uint8_t const *c_self_id,
                                                uint8_t const *c_peer_id);

    EXPORT bool E_PlainAtomicSwapVcAliceOnRequest(handle_t c_alice,
                                                  char const *request_file,
                                                  char const *response_file);

    EXPORT bool E_PlainAtomicSwapVcAliceOnReceipt(handle_t c_alice,
                                                  char const *receipt_file,
                                                  char const *secret_file);

    EXPORT bool E_PlainAtomicSwapVcAliceSetEvil(handle_t c_alice);

    EXPORT bool E_PlainAtomicSwapVcAliceFree(handle_t c_alice);

    EXPORT handle_t E_PlainAtomicSwapVcBobNew(handle_t c_bob_data,
                                              uint8_t const *c_self_id,
                                              uint8_t const *c_peer_id,
                                              range_t const *c_demand,
                                              uint64_t c_demand_count);

    EXPORT bool E_PlainAtomicSwapVcBobGetRequest(handle_t c_bob,
                                                 char const *request_file);

    EXPORT bool E_PlainAtomicSwapVcBobOnResponse(handle_t c_bob,
                                                 char const *response_file,
                                                 char const *receipt_file);

    EXPORT bool E_PlainAtomicSwapVcBobOnSecret(handle_t c_bob,
                                               char const *secret_file);

    EXPORT bool E_PlainAtomicSwapVcBobSaveDecrypted(handle_t c_bob,
                                                    char const *file);

    EXPORT bool E_PlainAtomicSwapVcBobFree(handle_t c_bob);

#ifdef __cplusplus
} // extern "C" atomic_swap_vc
#endif
