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
EXPORT handle_t E_TableAliceDataNew(char const* publish_path);
EXPORT handle_t E_TableBobDataNew(char const* bulletin_file,
                                  char const* public_path);

EXPORT bool E_TableAliceDataFree(handle_t h);
EXPORT bool E_TableBobDataFree(handle_t h);

EXPORT bool E_TableABulletin(handle_t h, table_bulletin_t* bulletin);
EXPORT bool E_TableBBulletin(handle_t h, table_bulletin_t* bulletin);

EXPORT bool E_TableBIsKeyUnique(handle_t h, char const* query_key,
                                bool* unique);
#ifdef __cplusplus
}
#endif

// complaint
#ifdef __cplusplus
extern "C" {
#endif
EXPORT handle_t E_TableComplaintSessionNew(handle_t c_a,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id);

EXPORT bool E_TableComplaintSessionOnRequest(handle_t c_session,
                                             char const* request_file,
                                             char const* response_file);

EXPORT bool E_TableComplaintSessionOnReceipt(handle_t c_session,
                                             char const* receipt_file,
                                             char const* secret_file);

EXPORT bool E_TableComplaintSessionSetEvil(handle_t c_session);

EXPORT bool E_TableComplaintSessionFree(handle_t h);

EXPORT handle_t E_TableComplaintClientNew(handle_t c_b,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count);

EXPORT bool E_TableComplaintClientGetRequest(handle_t c_client,
                                             char const* request_file);

EXPORT bool E_TableComplaintClientOnResponse(handle_t c_client,
                                             char const* response_file,
                                             char const* receipt_file);

EXPORT bool E_TableComplaintClientOnSecret(handle_t c_client,
                                           char const* secret_file);

EXPORT bool E_TableComplaintClientGenerateClaim(handle_t c_client,
                                                char const* claim_file);

EXPORT bool E_TableComplaintClientSaveDecrypted(handle_t c_client,
                                                char const* file);

EXPORT bool E_TableComplaintClientFree(handle_t h);
#ifdef __cplusplus
}  // extern "C" complaint
#endif

// atomic_swap
#ifdef __cplusplus
extern "C" {
#endif
EXPORT handle_t E_TableAtomicSwapSessionNew(handle_t c_a,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id);

EXPORT bool E_TableAtomicSwapSessionOnRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_TableAtomicSwapSessionOnReceipt(handle_t c_session,
                                              char const* receipt_file,
                                              char const* secret_file);

EXPORT bool E_TableAtomicSwapSessionSetEvil(handle_t c_session);

EXPORT bool E_TableAtomicSwapSessionFree(handle_t h);

EXPORT handle_t E_TableAtomicSwapClientNew(handle_t c_b,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id,
                                           range_t const* c_demand,
                                           uint64_t c_demand_count);

EXPORT bool E_TableAtomicSwapClientGetRequest(handle_t c_client,
                                              char const* request_file);

EXPORT bool E_TableAtomicSwapClientOnResponse(handle_t c_client,
                                              char const* response_file,
                                              char const* receipt_file);

EXPORT bool E_TableAtomicSwapClientOnSecret(handle_t c_client,
                                            char const* secret_file);

EXPORT bool E_TableAtomicSwapClientSaveDecrypted(handle_t c_client,
                                                 char const* file);

EXPORT bool E_TableAtomicSwapClientFree(handle_t h);
#ifdef __cplusplus
}  // extern "C" atomic_swap
#endif

// ot_complaint
#ifdef __cplusplus
extern "C" {
#endif
EXPORT handle_t E_TableOtComplaintSessionNew(handle_t c_a,
                                             uint8_t const* c_self_id,
                                             uint8_t const* c_peer_id);

EXPORT bool E_TableOtComplaintSessionGetNegoRequest(handle_t c_session,
                                                    char const* request_file);

EXPORT bool E_TableOtComplaintSessionOnNegoRequest(handle_t c_session,
                                                   char const* request_file,
                                                   char const* response_file);

EXPORT bool E_TableOtComplaintSessionOnNegoResponse(handle_t c_session,
                                                    char const* response_file);

EXPORT bool E_TableOtComplaintClientGetNegoRequest(handle_t c_client,
                                                   char const* request_file);

EXPORT bool E_TableOtComplaintClientOnNegoRequest(handle_t c_client,
                                                  char const* request_file,
                                                  char const* response_file);

EXPORT bool E_TableOtComplaintClientOnNegoResponse(handle_t c_client,
                                                   char const* response_file);

EXPORT bool E_TableOtComplaintSessionOnRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_TableOtComplaintSessionOnReceipt(handle_t c_session,
                                               char const* receipt_file,
                                               char const* secret_file);

EXPORT bool E_TableOtComplaintSessionSetEvil(handle_t c_session);

EXPORT bool E_TableOtComplaintSessionFree(handle_t h);

EXPORT handle_t E_TableOtComplaintClientNew(
    handle_t c_b, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count);

EXPORT bool E_TableOtComplaintClientGetRequest(handle_t c_client,
                                               char const* request_file);

EXPORT bool E_TableOtComplaintClientOnResponse(handle_t c_client,
                                               char const* response_file,
                                               char const* receipt_file);

EXPORT bool E_TableOtComplaintClientOnSecret(handle_t c_client,
                                             char const* secret_file);

EXPORT bool E_TableOtComplaintClientGenerateClaim(handle_t c_client,
                                                  char const* claim_file);

EXPORT bool E_TableOtComplaintClientSaveDecrypted(handle_t c_client,
                                                  char const* file);

EXPORT bool E_TableOtComplaintClientFree(handle_t h);
#ifdef __cplusplus
}  // extern "C" ot_complaint
#endif

// ot_vrfq
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableOtVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_TableOtVrfqSessionGetNegoRequest(handle_t c_session,
                                               char const* request_file);

EXPORT bool E_TableOtVrfqSessionOnNegoRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_TableOtVrfqSessionOnNegoResponse(handle_t c_session,
                                               char const* response_file);

EXPORT bool E_TableOtVrfqSessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_TableOtVrfqSessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_TableOtVrfqSessionFree(handle_t h);

EXPORT handle_t E_TableOtVrfqClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       char const* c_query_key,
                                       char const* c_query_values[],
                                       uint64_t c_query_value_count,
                                       char const* c_phantoms[],
                                       uint64_t c_phantom_count);

EXPORT bool E_TableOtVrfqClientGetNegoRequest(handle_t c_client,
                                              char const* request_file);

EXPORT bool E_TableOtVrfqClientOnNegoRequest(handle_t c_client,
                                             char const* request_file,
                                             char const* response_file);

EXPORT bool E_TableOtVrfqClientOnNegoResponse(handle_t c_client,
                                              char const* response_file);

EXPORT bool E_TableOtVrfqClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_TableOtVrfqClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_TableOtVrfqClientOnSecret(handle_t c_client,
                                        char const* secret_file,
                                        char const* positions_file);

EXPORT bool E_TableOtVrfqClientFree(handle_t h);
#ifdef __cplusplus
}  // extern "C" ot_vrfq
#endif

// vrfq
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id);

EXPORT bool E_TableVrfqSessionOnRequest(handle_t c_session,
                                        char const* request_file,
                                        char const* response_file);

EXPORT bool E_TableVrfqSessionOnReceipt(handle_t c_session,
                                        char const* receipt_file,
                                        char const* secret_file);

EXPORT bool E_TableVrfqSessionFree(handle_t h);

EXPORT handle_t E_TableVrfqClientNew(handle_t c_b, uint8_t const* c_self_id,
                                     uint8_t const* c_peer_id,
                                     char const* c_query_key,
                                     char const* c_query_values[],
                                     uint64_t c_query_value_count);

EXPORT bool E_TableVrfqClientGetRequest(handle_t c_client,
                                        char const* request_file);

EXPORT bool E_TableVrfqClientOnResponse(handle_t c_client,
                                        char const* response_file,
                                        char const* receipt_file);

EXPORT bool E_TableVrfqClientOnSecret(handle_t c_client,
                                      char const* secret_file,
                                      char const* positions_file);

EXPORT bool E_TableVrfqClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" vrfq
#endif
