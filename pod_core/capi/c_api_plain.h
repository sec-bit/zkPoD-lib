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
extern "C" {
#endif

EXPORT handle_t E_PlainAliceDataNew(char const* publish_path);
EXPORT handle_t E_PlainBobDataNew(char const* bulletin_file,
                                  char const* public_path);

EXPORT bool E_PlainAliceDataFree(handle_t c_alice_data);
EXPORT bool E_PlainBobDataFree(handle_t c_bob_data);

EXPORT bool E_PlainAliceBulletin(handle_t c_alice_data,
                                 plain_bulletin_t* bulletin);
EXPORT bool E_PlainBobBulletin(handle_t c_bob_data, plain_bulletin_t* bulletin);

#ifdef __cplusplus
}
#endif

// complaint

#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainComplaintSessionNew(handle_t c_a,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id);

EXPORT bool E_PlainComplaintSessionOnRequest(handle_t c_session,
                                             char const* request_file,
                                             char const* response_file);

EXPORT bool E_PlainComplaintSessionOnReceipt(handle_t c_session,
                                             char const* receipt_file,
                                             char const* secret_file);

EXPORT bool E_PlainComplaintSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainComplaintSessionFree(handle_t h);

EXPORT handle_t E_PlainComplaintClientNew(handle_t c_b,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count);

EXPORT bool E_PlainComplaintClientGetRequest(handle_t c_client,
                                             char const* request_file);

EXPORT bool E_PlainComplaintClientOnResponse(handle_t c_client,
                                             char const* response_file,
                                             char const* receipt_file);

EXPORT bool E_PlainComplaintClientOnSecret(handle_t c_client,
                                           char const* secret_file);

EXPORT bool E_PlainComplaintClientGenerateClaim(handle_t c_client,
                                                char const* claim_file);

EXPORT bool E_PlainComplaintClientSaveDecrypted(handle_t c_client,
                                                char const* file);

EXPORT bool E_PlainComplaintClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" complaint
#endif

// atomic_swap
#ifdef __cplusplus
extern "C" {
#endif
EXPORT handle_t E_PlainAtomicSwapSessionNew(handle_t c_a,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id);

EXPORT bool E_PlainAtomicSwapSessionOnRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_PlainAtomicSwapSessionOnReceipt(handle_t c_session,
                                              char const* receipt_file,
                                              char const* secret_file);

EXPORT bool E_PlainAtomicSwapSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainAtomicSwapSessionFree(handle_t h);

EXPORT handle_t E_PlainAtomicSwapClientNew(handle_t c_b,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id,
                                           range_t const* c_demand,
                                           uint64_t c_demand_count);

EXPORT bool E_PlainAtomicSwapClientGetRequest(handle_t c_client,
                                              char const* request_file);

EXPORT bool E_PlainAtomicSwapClientOnResponse(handle_t c_client,
                                              char const* response_file,
                                              char const* receipt_file);

EXPORT bool E_PlainAtomicSwapClientOnSecret(handle_t c_client,
                                            char const* secret_file);

EXPORT bool E_PlainAtomicSwapClientSaveDecrypted(handle_t c_client,
                                                 char const* file);

EXPORT bool E_PlainAtomicSwapClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" 
#endif

// ot_complaint
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainOtComplaintSessionNew(handle_t c_a,
                                             uint8_t const* c_self_id,
                                             uint8_t const* c_peer_id);

EXPORT bool E_PlainOtComplaintSessionGetNegoRequest(handle_t c_session,
                                                    char const* request_file);

EXPORT bool E_PlainOtComplaintSessionOnNegoRequest(handle_t c_session,
                                                   char const* request_file,
                                                   char const* response_file);

EXPORT bool E_PlainOtComplaintSessionOnNegoResponse(handle_t c_session,
                                                    char const* response_file);

EXPORT bool E_PlainOtComplaintClientGetNegoRequest(handle_t c_client,
                                                   char const* request_file);

EXPORT bool E_PlainOtComplaintClientOnNegoRequest(handle_t c_client,
                                                  char const* request_file,
                                                  char const* response_file);

EXPORT bool E_PlainOtComplaintClientOnNegoResponse(handle_t c_client,
                                                   char const* response_file);

EXPORT bool E_PlainOtComplaintSessionOnRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_PlainOtComplaintSessionOnReceipt(handle_t c_session,
                                               char const* receipt_file,
                                               char const* secret_file);

EXPORT bool E_PlainOtComplaintSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainOtComplaintSessionFree(handle_t h);

EXPORT handle_t E_PlainOtComplaintClientNew(
    handle_t c_b, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count);

EXPORT bool E_PlainOtComplaintClientGetRequest(handle_t c_client,
                                               char const* request_file);

EXPORT bool E_PlainOtComplaintClientOnResponse(handle_t c_client,
                                               char const* response_file,
                                               char const* receipt_file);

EXPORT bool E_PlainOtComplaintClientOnSecret(handle_t c_client,
                                             char const* secret_file);

EXPORT bool E_PlainOtComplaintClientGenerateClaim(handle_t c_client,
                                                  char const* claim_file);

EXPORT bool E_PlainOtComplaintClientSaveDecrypted(handle_t c_client,
                                                  char const* file);

EXPORT bool E_PlainOtComplaintClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" ot_complaint
#endif