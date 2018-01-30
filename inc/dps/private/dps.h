/*
 *******************************************************************
 *
 * Copyright 2016 Intel Corporation All rights reserved.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef _DPS_INTERNAL_H
#define _DPS_INTERNAL_H

#include <dps/dps.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Map keys for CBOR serialization of DPS messages
 */
#define DPS_CBOR_KEY_PORT           1   /* uint */
#define DPS_CBOR_KEY_TTL            2   /* int */
#define DPS_CBOR_KEY_PUB_ID         3   /* bstr (UUID) */
#define DPS_CBOR_KEY_SEQ_NUM        4   /* uint */
#define DPS_CBOR_KEY_ACK_REQ        5   /* bool */
#define DPS_CBOR_KEY_BLOOM_FILTER   6   /* bstr */
#define DPS_CBOR_KEY_SUB_FLAGS      7   /* uint */
#define DPS_CBOR_KEY_MESH_ID        8   /* bstr (UUID) */
#define DPS_CBOR_KEY_NEEDS          9   /* bstr */
#define DPS_CBOR_KEY_INTERESTS     10   /* bstr */
#define DPS_CBOR_KEY_TOPICS        11   /* array (tstr) */
#define DPS_CBOR_KEY_DATA          12   /* bstr */

#define DPS_SECS_TO_MS(t)   ((uint64_t)(t) * 1000ull)
#define DPS_MS_TO_SECS(t)   ((uint32_t)((t) / 1000ull))

/**
 * Address type
 */
typedef struct _DPS_NodeAddress {
    struct sockaddr_storage inaddr;
} DPS_NodeAddress;

/**
 * For managing data that has been received
 */
typedef struct _DPS_RxBuffer {
    uint8_t* base;   /**< base address for buffer */
    uint8_t* eod;    /**< end of data */
    uint8_t* rxPos;  /**< current read location in buffer */
} DPS_RxBuffer;

/**
 * Initialize a receive buffer
 *
 * @param buffer    Buffer to initialized
 * @param storage   The storage for the buffer. The storage cannot be NULL
 * @param size      The size of the storage
 *
 * @return   DPS_OK or DP_ERR_RESOURCES if storage is needed and could not be allocated.
 */
DPS_Status DPS_RxBufferInit(DPS_RxBuffer* buffer, uint8_t* storage, size_t size);

/**
 * Free resources allocated for a buffer and nul out the buffer pointers.
 *
 * @param buffer    Buffer to free
 */
void DPS_RxBufferFree(DPS_RxBuffer* buffer);

/*
 * Clear receive buffer fields
 */
#define DPS_RxBufferClear(b) do { (b)->base = (b)->rxPos = (b)->eod = NULL; } while (0)

/*
 * Data available in a receive buffer
 */
#define DPS_RxBufferAvail(b)  ((uint32_t)((b)->eod - (b)->rxPos))

/**
 * For managing data to be transmitted
 */
typedef struct _DPS_TxBuffer {
    uint8_t* base;  /**< base address for buffer */
    uint8_t* eob;   /**< end of buffer */
    uint8_t* txPos; /**< current write location in buffer */
} DPS_TxBuffer;

/**
 * Initialize a transmit buffer
 *
 * @param buffer    Buffer to initialized
 * @param storage   The storage for the buffer. If the storage is NULL storage is allocated.
 * @param size      Current size of the buffer
 *
 * @return   DPS_OK or DP_ERR_RESOURCES if storage is needed and could not be allocated.
 */
DPS_Status DPS_TxBufferInit(DPS_TxBuffer* buffer, uint8_t* storage, size_t size);

/**
 * Free resources allocated for a buffer and nul out the buffer pointers.
 *
 * @param buffer    Buffer to free
 */
void DPS_TxBufferFree(DPS_TxBuffer* buffer);

/**
 * Add data to a transmit buffer
 *
 * @param buffer   Buffer to append to
 * @param storage  The data to append
 * @param len      Length of the data to append
 *
 * @return   DPS_OK or DP_ERR_RESOURCES if there not enough room in the buffer
 */
DPS_Status DPS_TxBufferAppend(DPS_TxBuffer* buffer, const uint8_t* data, size_t len);

/*
 * Clear transmit buffer fields
 */
#define DPS_TxBufferClear(b) do { (b)->base = (b)->txPos = (b)->eob = NULL; } while (0)

/*
 * Space left in a transmit buffer
 */
#define DPS_TxBufferSpace(b)  ((uint32_t)((b)->eob - (b)->txPos))

/*
 * Number of bytes that have been written to a transmit buffer
 */
#define DPS_TxBufferUsed(b)  ((uint32_t)((b)->txPos - (b)->base))

/**
 * Convert a transmit buffer into a receive buffer. Note that this
 * aliases the internal storage so care must be taken to avoid a
 * double free.
 *
 * @param txBuffer   A buffer containing data
 * @param rxBuffer   Receive buffer struct to be initialized
 */
void DPS_TxBufferToRx(DPS_TxBuffer* txBuffer, DPS_RxBuffer* rxBuffer);

/**
 * Convert a receive buffer into a transmit buffer. Note that this
 * aliases the internal storage so care must be taken to avoid a
 * double free.
 *
 * @param rxBuffer   A buffer containing data
 * @param txBuffer   Transmit buffer struct to be initialized
 */
void DPS_RxBufferToTx(DPS_RxBuffer* rxBuffer, DPS_TxBuffer* txBuffer);

/**
 * Print the current subscriptions
 */
void DPS_DumpSubscriptions(DPS_Node* node);

/**
 * Copy a DPS_KeyId
 *
 * @return dest on success or NULL on failure
 */
DPS_KeyId* DPS_CopyKeyId(DPS_KeyId* dest, const DPS_KeyId* src);

/**
 * Release memory used by the key ID.
 */
void DPS_ClearKeyId(DPS_KeyId* keyId);

/*
 * Compare two key IDs.
 *
 * @param a  The ID to compare against
 * @param b  The ID to compare with b
 *
 * @return Returns zero if the addresses are different non-zero if they are the same
 */
int DPS_SameKeyId(const DPS_KeyId* a, const DPS_KeyId* b);

/**
 * A key store request.
 */
struct _DPS_KeyStoreRequest {
    DPS_KeyStore* keyStore;
    void* data;
    DPS_Status (*setKeyAndIdentity)(DPS_KeyStoreRequest* request, const DPS_Key* key, const DPS_KeyId* keyId);
    DPS_Status (*setKey)(DPS_KeyStoreRequest* request, const DPS_Key* key);
    DPS_Status (*setCA)(DPS_KeyStoreRequest* request, const char* ca);
    DPS_Status (*setCert)(DPS_KeyStoreRequest* request, const char* cert, size_t certLen,
                          const char* key, size_t keyLen, const char* pwd, size_t pwdLen);
};

/**
 * A key store.
 */
struct _DPS_KeyStore {
    void* userData;
    DPS_KeyAndIdentityHandler keyAndIdentityHandler;
    DPS_KeyHandler keyHandler;
    DPS_EphemeralKeyHandler ephemeralKeyHandler;
    DPS_CAHandler caHandler;
};

/**
 * A permission store.
 */
struct _DPS_PermissionStore {
    void* userData;
    DPS_GetPermissionsHandler getHandler;
};

/**
 * Returns true if the message sender identified by either the network
 * layer ID or message signature is authorized with the provided
 * permissions.
 *
 * @param node The node containing the permissions to check
 * @param netId The network layer ID
 * @param buf The encrypted field of a DPS message
 * @param perm The permissions to check
 *
 * @return DPS_TRUE if authorized, DPS_FALSE otherwise
 */
int DPS_IsAuthorized(DPS_Node* node, const DPS_KeyId* netId, const DPS_RxBuffer* buf, int perm);

/**
 * Returns a non-secure random number
 */
uint32_t DPS_Rand();

#ifdef __cplusplus
}
#endif

#endif
