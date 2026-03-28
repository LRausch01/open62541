/* 
 * Driver Subsystem for open62541
 * ------------------------------
 *
 * This header defines a lightweight driver abstraction that integrates
 * directly into the open62541 server event loop. A driver represents a
 * self‑contained module with its own lifecycle:
 *
 *      create → init → start → periodic updates → stop → destroy
 *
 * Drivers register a user‑defined update callback which is executed at a
 * configurable interval using UA_Server_addRepeatedCallback(). This allows
 * application‑specific background tasks—such as hardware polling, file
 * monitoring, communication with external systems, or cyclic computations—
 * to run automatically inside the OPC UA server loop without requiring
 * additional threads or timers.
 *
 * Each driver instance stores:
 *   - a name for identification
 *   - a pointer to the UA_Server
 *   - an update interval in milliseconds
 *   - a callback ID managed by open62541
 *   - a user‑defined update function and user data
 *   - lifecycle state flags (initialized, running)
 *
 * This subsystem provides a clean and extensible foundation for building
 * modular, reusable components that integrate seamlessly with open62541.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include <open62541/server.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct UA_Driver UA_Driver;

typedef void (*DriverUpdateCallback)(UA_Driver *driver, void *user_data);

struct UA_Driver {
    const char *name;

    UA_Server *server;

    UA_UInt64 updateIntervalMs;

    UA_UInt64 callbackId;

    DriverUpdateCallback updateCallback;
    void *userData;

    UA_Boolean initialized;
    UA_Boolean running;
};

UA_Driver *driver_create(const char *name, UA_Server *server);
void driver_destroy(UA_Driver *driver);

UA_StatusCode driver_init(UA_Driver *driver);
UA_StatusCode driver_start(UA_Driver *driver);
void driver_stop(UA_Driver *driver);

UA_StatusCode driver_setUpdateCallback(UA_Driver *driver, DriverUpdateCallback callback, void *user_data);

UA_StatusCode driver_setUpdateInterval(UA_Driver *driver, UA_UInt64 intervalMs);

#endif // DRIVER_H
