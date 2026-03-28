#include <driver.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_UPDATE_INTERVAL_MS 1000

/* Internal Wrapper for UA Server Repeated Callbacks */
static void driver_repeatedCallback(UA_Server *server, void *user_data) {
    UA_Driver *driver = (UA_Driver *)user_data;

    if (driver->running && driver->updateCallback) {
        driver->updateCallback(driver, driver->userData);
    }
}

/* Manage drivers */
UA_Driver *driver_create(const char *name, UA_Server *server) {
    UA_Driver *driver = (UA_Driver *)malloc(sizeof(UA_Driver));
    if (!driver)
        return NULL;

    driver->name = name;
    driver->server = server;
    driver->updateIntervalMs = DEFAULT_UPDATE_INTERVAL_MS;

    return driver;
}

void driver_destroy(UA_Driver *driver) {
    if (!driver)
        return;

    driver_stop(driver);
    free(driver);
}

/* Driver lifecycle */
UA_StatusCode driver_init(UA_Driver *driver) {
    if (!driver)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    driver->initialized = UA_TRUE;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode driver_start(UA_Driver *driver) {
    if (!driver || !driver->initialized)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    if (driver->running)
        return UA_STATUSCODE_GOOD;

    UA_StatusCode ret = UA_Server_addRepeatedCallback(
        driver->server, 
        driver_repeatedCallback, 
        driver, 
        driver->updateIntervalMs,
        &driver->callbackId
    );

    if (ret != UA_STATUSCODE_GOOD)
        return UA_STATUSCODE_BADINTERNALERROR;

    driver->running = UA_TRUE;
    return UA_STATUSCODE_GOOD;
}

void driver_stop(UA_Driver *driver) {
    if (!driver || !driver->running)
        return;

    UA_Server_removeRepeatedCallback(driver->server, driver->callbackId);
    driver->running = UA_FALSE;
}

/* Update management */
UA_StatusCode driver_setUpdateCallback(UA_Driver *driver, DriverUpdateCallback callback, void *user_data) {
    if (!driver)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    if (driver->running) {
        driver_stop(driver);
        driver->updateCallback = callback;
        driver->userData = user_data;
        driver_start(driver);
    } else {
        driver->updateCallback = callback;
        driver->userData = user_data;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
driver_setUpdateInterval(UA_Driver *driver, UA_UInt64 intervalMs) {
    if (!driver)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    driver->updateIntervalMs = intervalMs;
    if (driver->running) {
        return UA_Server_changeRepeatedCallbackInterval(
            driver->server, 
            driver->callbackId, 
            intervalMs
        );
    }
    
    return UA_STATUSCODE_GOOD;
}
