/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */

/**
 * Basic OPC UA Server with Driver Integration
 * -------------------------------------------
 *
 * This example demonstrates:
 *  - how to create and run a minimal OPC UA server using open62541
 *  - how to integrate a simple "Driver" subsystem
 *    that uses UA_Server_addRepeatedCallback() for periodic updates
 *
 * Drivers are small modules with their own lifecycle:
 *    create → init → start → (periodic updates) → stop → destroy
 *
 * The update function can perform any application‑specific logic.
 * This allows you to embed hardware access, file polling, or
 * background tasks directly into the server’s main loop.
 */

#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>
#include <driver.h>

/* -------------------------------------------------------------------------
 * Example Driver Update Function
 * -------------------------------------------------------------------------
 * This function is called periodically by the server.
 * The interval is configured when creating the driver.
 */
static void
exampleDriverUpdate(UA_Driver *driver, void *userData) {
    (void)driver;
    (void)userData;

    /* In a real driver, you would perform I/O, polling, etc. */
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "ExampleDriver: periodic update executed");
}

int main(void) {
    /* ---------------------------------------------------------------------
     * Create and configure the OPC UA server
     * --------------------------------------------------------------------- */
    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    /* ---------------------------------------------------------------------
     * Create a basic driver and attach it to the server
     * --------------------------------------------------------------------- */
    UA_Driver *exampleDriver = driver_create("ExampleDriver", server);

    driver_setUpdateCallback(exampleDriver, exampleDriverUpdate, NULL);
    driver_setUpdateInterval(exampleDriver, 1000); /* 1000ms = 1s */

    driver_init(exampleDriver);
    driver_start(exampleDriver);

    /* ---------------------------------------------------------------------
     * Run the server until Ctrl‑C
     * --------------------------------------------------------------------- */
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Server running. Press Ctrl‑C to exit.");

    UA_Server_runUntilInterrupt(server);

    /* ---------------------------------------------------------------------
     * Cleanup
     * --------------------------------------------------------------------- */
    driver_stop(exampleDriver);
    driver_destroy(exampleDriver);

    UA_Server_delete(server);
    return 0;
}