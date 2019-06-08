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

EXPORT handle_t E_TableANew(char const* publish_path);
EXPORT handle_t E_TableBNew(char const* bulletin_file, char const* public_path);

EXPORT bool E_TableAFree(handle_t h);
EXPORT bool E_TableBFree(handle_t h);

EXPORT bool E_TableABulletin(handle_t h, table_bulletin_t* bulletin);
EXPORT bool E_TableBBulletin(handle_t h, table_bulletin_t* bulletin);

EXPORT bool E_TableBIsKeyUnique(handle_t h, char const* query_key,
                                bool* unique);
#ifdef __cplusplus
}
#endif

// batch
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id);

EXPORT bool E_TableBatchSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file);

EXPORT bool E_TableBatchSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file);

EXPORT bool E_TableBatchSessionSetEvil(handle_t c_session);

EXPORT bool E_TableBatchSessionFree(handle_t h);

EXPORT handle_t E_TableBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t const* c_demand,
                                      uint64_t c_demand_count);

EXPORT bool E_TableBatchClientGetRequest(handle_t c_client,
                                         char const* request_file);

EXPORT bool E_TableBatchClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file);

EXPORT bool E_TableBatchClientOnSecret(handle_t c_client,
                                       char const* secret_file);

EXPORT bool E_TableBatchClientGenerateClaim(handle_t c_client,
                                            char const* claim_file);

EXPORT bool E_TableBatchClientSaveDecrypted(handle_t c_client,
                                            char const* file);

EXPORT bool E_TableBatchClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch
#endif

// batch2
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableBatch2SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_TableBatch2SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_TableBatch2SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_TableBatch2SessionSetEvil(handle_t c_session);

EXPORT bool E_TableBatch2SessionFree(handle_t h);

EXPORT handle_t E_TableBatch2ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count);

EXPORT bool E_TableBatch2ClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_TableBatch2ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_TableBatch2ClientOnSecret(handle_t c_client,
                                        char const* secret_file);

EXPORT bool E_TableBatch2ClientSaveDecrypted(handle_t c_client,
                                             char const* file);

EXPORT bool E_TableBatch2ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch2
#endif

// batch3
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableBatch3SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_TableBatch3SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_TableBatch3SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_TableBatch3SessionFree(handle_t h);

EXPORT handle_t E_TableBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count);

EXPORT bool E_TableBatch3ClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_TableBatch3ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_TableBatch3ClientOnSecret(handle_t c_client,
                                        char const* secret_file);

EXPORT bool E_TableBatch3ClientSaveDecrypted(handle_t c_client,
                                             char const* file);

EXPORT bool E_TableBatch3ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" batch3
#endif

// otbatch3
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableOtBatch3SessionNew(handle_t c_a,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id);

EXPORT bool E_TableOtBatch3SessionGetNegoRequest(handle_t c_session,
                                                 char const* request_file);

EXPORT bool E_TableOtBatch3SessionOnNegoRequest(handle_t c_session,
                                                char const* request_file,
                                                char const* response_file);

EXPORT bool E_TableOtBatch3SessionOnNegoResponse(handle_t c_session,
                                                 char const* response_file);

EXPORT bool E_TableOtBatch3ClientGetNegoRequest(handle_t c_client,
                                                char const* request_file);

EXPORT bool E_TableOtBatch3ClientOnNegoRequest(handle_t c_client,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_TableOtBatch3ClientOnNegoResponse(handle_t c_client,
                                                char const* response_file);

EXPORT bool E_TableOtBatch3SessionOnRequest(handle_t c_session,
                                            char const* request_file,
                                            char const* response_file);

EXPORT bool E_TableOtBatch3SessionOnReceipt(handle_t c_session,
                                            char const* receipt_file,
                                            char const* secret_file);

EXPORT bool E_TableOtBatch3SessionFree(handle_t h);

EXPORT handle_t E_TableOtBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id,
                                         range_t const* c_demand,
                                         uint64_t c_demand_count,
                                         range_t const* c_phantom,
                                         uint64_t c_phantom_count);

EXPORT bool E_TableOtBatch3ClientGetRequest(handle_t c_client,
                                            char const* request_file);

EXPORT bool E_TableOtBatch3ClientOnResponse(handle_t c_client,
                                            char const* response_file,
                                            char const* receipt_file);

EXPORT bool E_TableOtBatch3ClientOnSecret(handle_t c_client,
                                          char const* secret_file);

EXPORT bool E_TableOtBatch3ClientSaveDecrypted(handle_t c_client,
                                               char const* file);

EXPORT bool E_TableOtBatch3ClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" otbatch3
#endif

// otbatch
#ifdef __cplusplus
extern "C" {
#endif

EXPORT handle_t E_TableOtBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id);

EXPORT bool E_TableOtBatchSessionGetNegoRequest(handle_t c_session,
                                                char const* request_file);

EXPORT bool E_TableOtBatchSessionOnNegoRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_TableOtBatchSessionOnNegoResponse(handle_t c_session,
                                                char const* response_file);

EXPORT bool E_TableOtBatchClientGetNegoRequest(handle_t c_client,
                                               char const* request_file);

EXPORT bool E_TableOtBatchClientOnNegoRequest(handle_t c_client,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_TableOtBatchClientOnNegoResponse(handle_t c_client,
                                               char const* response_file);

EXPORT bool E_TableOtBatchSessionOnRequest(handle_t c_session,
                                           char const* request_file,
                                           char const* response_file);

EXPORT bool E_TableOtBatchSessionOnReceipt(handle_t c_session,
                                           char const* receipt_file,
                                           char const* secret_file);

EXPORT bool E_TableOtBatchSessionSetEvil(handle_t c_session);

EXPORT bool E_TableOtBatchSessionFree(handle_t h);

EXPORT handle_t E_TableOtBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t const* c_demand,
                                        uint64_t c_demand_count,
                                        range_t const* c_phantom,
                                        uint64_t c_phantom_count);

EXPORT bool E_TableOtBatchClientGetRequest(handle_t c_client,
                                           char const* request_file);

EXPORT bool E_TableOtBatchClientOnResponse(handle_t c_client,
                                           char const* response_file,
                                           char const* receipt_file);

EXPORT bool E_TableOtBatchClientOnSecret(handle_t c_client,
                                         char const* secret_file);

EXPORT bool E_TableOtBatchClientGenerateClaim(handle_t c_client,
                                              char const* claim_file);

EXPORT bool E_TableOtBatchClientSaveDecrypted(handle_t c_client,
                                              char const* file);

EXPORT bool E_TableOtBatchClientFree(handle_t h);

#ifdef __cplusplus
}  // extern "C" otbatch
#endif

// otvrfq
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
}  // extern "C" otvrfq
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
