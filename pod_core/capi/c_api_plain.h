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

EXPORT handle_t E_PlainANew(char const* publish_path);
EXPORT handle_t E_PlainBNew(char const* bulletin_file, char const* public_path);

EXPORT bool E_PlainAFree(handle_t h);
EXPORT bool E_PlainBFree(handle_t h);

EXPORT bool E_PlainABulletin(handle_t h, plain_bulletin_t* bulletin);
EXPORT bool E_PlainBBulletin(handle_t h, plain_bulletin_t* bulletin);

#ifdef __cplusplus
}
#endif

// batch

#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id);

EXPORT bool E_PlainBatchSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file);

EXPORT bool E_PlainBatchSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file);

EXPORT bool E_PlainBatchSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainBatchSessionFree(handle_t h);

EXPORT handle_t E_PlainBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t const* c_demand,
                                      uint64_t c_demand_count);

EXPORT bool E_PlainBatchClientGetRequest(handle_t c_client,
                                         char const* request_file);

EXPORT bool E_PlainBatchClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file);

EXPORT bool E_PlainBatchClientOnSecret(handle_t c_client,
                                       char const* secret_file);

EXPORT bool E_PlainBatchClientGenerateClaim(handle_t c_client,
                                            char const* claim_file);

EXPORT bool E_PlainBatchClientSaveDecrypted(handle_t c_client,
                                            char const* file);

EXPORT bool E_PlainBatchClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch
#endif

// batch2
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainBatch2SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_PlainBatch2SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_PlainBatch2SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_PlainBatch2SessionSetEvil(handle_t c_session);

EXPORT bool E_PlainBatch2SessionFree(handle_t h);

EXPORT handle_t E_PlainBatch2ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count);

EXPORT bool E_PlainBatch2ClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_PlainBatch2ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_PlainBatch2ClientOnSecret(handle_t c_client,
                                        char const* secret_file);

EXPORT bool E_PlainBatch2ClientSaveDecrypted(handle_t c_client,
                                             char const* file);

EXPORT bool E_PlainBatch2ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch2
#endif

// batch3
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainBatch3SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_PlainBatch3SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_PlainBatch3SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_PlainBatch3SessionFree(handle_t h);

EXPORT handle_t E_PlainBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count);

EXPORT bool E_PlainBatch3ClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_PlainBatch3ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_PlainBatch3ClientOnSecret(handle_t c_client,
                                        char const* secret_file);

EXPORT bool E_PlainBatch3ClientSaveDecrypted(handle_t c_client,
                                             char const* file);

EXPORT bool E_PlainBatch3ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch3
#endif

// otbatch3
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainOtBatch3SessionNew(handle_t c_a,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id);

EXPORT bool E_PlainOtBatch3SessionGetNegoRequest(handle_t c_session,
                                                 char const* request_file);

EXPORT bool E_PlainOtBatch3SessionOnNegoRequest(handle_t c_session,
                                                char const* request_file,
                                                char const* response_file);

EXPORT bool E_PlainOtBatch3SessionOnNegoResponse(handle_t c_session,
                                                 char const* response_file);

EXPORT bool E_PlainOtBatch3ClientGetNegoRequest(handle_t c_client,
                                                char const* request_file);

EXPORT bool E_PlainOtBatch3ClientOnNegoRequest(handle_t c_client,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_PlainOtBatch3ClientOnNegoResponse(handle_t c_client,
                                                char const* response_file);

EXPORT bool E_PlainOtBatch3SessionOnRequest(handle_t c_session,
                                            char const* request_file,
                                            char const* response_file);

EXPORT bool E_PlainOtBatch3SessionOnReceipt(handle_t c_session,
                                            char const* receipt_file,
                                            char const* secret_file);

EXPORT bool E_PlainOtBatch3SessionFree(handle_t h);

EXPORT handle_t E_PlainOtBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id,
                                         range_t const* c_demand,
                                         uint64_t c_demand_count,
                                         range_t const* c_phantom,
                                         uint64_t c_phantom_count);

EXPORT bool E_PlainOtBatch3ClientGetRequest(handle_t c_client,
                                            char const* request_file);

EXPORT bool E_PlainOtBatch3ClientOnResponse(handle_t c_client,
                                            char const* response_file,
                                            char const* receipt_file);

EXPORT bool E_PlainOtBatch3ClientOnSecret(handle_t c_client,
                                          char const* secret_file);

EXPORT bool E_PlainOtBatch3ClientSaveDecrypted(handle_t c_client,
                                               char const* file);

EXPORT bool E_PlainOtBatch3ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" otbatch3
#endif

// otbatch
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_PlainOtBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id);

EXPORT bool E_PlainOtBatchSessionGetNegoRequest(handle_t c_session,
                                                char const* request_file);

EXPORT bool E_PlainOtBatchSessionOnNegoRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_PlainOtBatchSessionOnNegoResponse(handle_t c_session,
                                                char const* response_file);

EXPORT bool E_PlainOtBatchClientGetNegoRequest(handle_t c_client,
                                               char const* request_file);

EXPORT bool E_PlainOtBatchClientOnNegoRequest(handle_t c_client,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_PlainOtBatchClientOnNegoResponse(handle_t c_client,
                                               char const* response_file);

EXPORT bool E_PlainOtBatchSessionOnRequest(handle_t c_session,
                                           char const* request_file,
                                           char const* response_file);

EXPORT bool E_PlainOtBatchSessionOnReceipt(handle_t c_session,
                                           char const* receipt_file,
                                           char const* secret_file);

EXPORT bool E_PlainOtBatchSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainOtBatchSessionFree(handle_t h);

EXPORT handle_t E_PlainOtBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t const* c_demand,
                                        uint64_t c_demand_count,
                                        range_t const* c_phantom,
                                        uint64_t c_phantom_count);

EXPORT bool E_PlainOtBatchClientGetRequest(handle_t c_client,
                                           char const* request_file);

EXPORT bool E_PlainOtBatchClientOnResponse(handle_t c_client,
                                           char const* response_file,
                                           char const* receipt_file);

EXPORT bool E_PlainOtBatchClientOnSecret(handle_t c_client,
                                         char const* secret_file);

EXPORT bool E_PlainOtBatchClientGenerateClaim(handle_t c_client,
                                              char const* claim_file);

EXPORT bool E_PlainOtBatchClientSaveDecrypted(handle_t c_client,
                                              char const* file);

EXPORT bool E_PlainOtBatchClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" otbatch
#endif
