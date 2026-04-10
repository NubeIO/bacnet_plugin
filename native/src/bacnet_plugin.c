#include "bacnet_plugin.h"
#include <setjmp.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

/* Global jump buffer to intercept exit() calls */
static jmp_buf g_exit_jmp;
static bool g_jmp_active = false;

/*
 * Custom exit handler to prevent the native library from terminating the entire
 * Flutter process. Redefined via CMake: -Dexit=bacnet_plugin_exit_handler
 */
#ifdef _WIN32
__declspec(dllexport)
#endif
void bacnet_plugin_exit_handler(int code)
{
    char buf[256];
    sprintf(buf, "BACnet Native Exit Intercepted: code %d\n", code);
#ifdef _WIN32
    OutputDebugStringA(buf);
#else
    fprintf(stderr, "%s", buf);
#endif

    if (g_jmp_active) {
        longjmp(g_exit_jmp, 1);
    }

    /* Fallback if jump is not active (should not happen in wrapped calls) */
#ifdef _WIN32
    TerminateThread(GetCurrentThread(), code);
#else
    pthread_exit(NULL);
#endif
}

/* Helper macro for safe wrapper functions.
 * On Windows, uses SEH (__try/__except) for crash protection.
 * On Linux, uses plain setjmp/longjmp (no SEH available). */
#ifdef _WIN32
#define SAFE_WRAP_BEGIN  __try { g_jmp_active = true; if (setjmp(g_exit_jmp) == 0) {
#define SAFE_WRAP_EXIT(msg, fail_val) } else { OutputDebugStringA(msg " Intercepted exit()\n"); fail_val; } } __except(EXCEPTION_EXECUTE_HANDLER) { OutputDebugStringA(msg " Caught Access Violation/Crash!\n"); fail_val; } g_jmp_active = false;
#else
#define SAFE_WRAP_BEGIN  g_jmp_active = true; if (setjmp(g_exit_jmp) == 0) {
#define SAFE_WRAP_EXIT(msg, fail_val) } else { fprintf(stderr, "%s Intercepted exit()\n", msg); fail_val; } g_jmp_active = false;
#endif

/* Wrapper to simplify calling Send_Write_Property_Multiple_Request */
uint8_t bacnet_plugin_send_write_property_multiple(
    uint32_t device_id,
    BACNET_WRITE_ACCESS_DATA *write_access_data)
{
    uint8_t result = 0;
    SAFE_WRAP_BEGIN
        uint8_t pdu[MAX_APDU] = {0};
        result = Send_Write_Property_Multiple_Request(pdu, sizeof(pdu), device_id, write_access_data);
    SAFE_WRAP_EXIT("BACnet WPM:", result = 0)
    return result;
}

uint8_t bacnet_plugin_send_read_range_request(
    uint32_t device_id,
    BACNET_READ_RANGE_DATA *read_range_data)
{
    uint8_t result = 0;
    SAFE_WRAP_BEGIN
        result = Send_ReadRange_Request(device_id, read_range_data);
    SAFE_WRAP_EXIT("BACnet ReadRange:", result = 0)
    return result;
}

bool bacnet_plugin_safe_bip_init(char *ifname)
{
    bool result = false;
    SAFE_WRAP_BEGIN
        result = bip_init(ifname);
    SAFE_WRAP_EXIT("BACnet safe_bip_init:", result = false)
    return result;
}

bool bacnet_plugin_safe_datalink_init(char *ifname)
{
    bool result = false;
    SAFE_WRAP_BEGIN
        result = datalink_init(ifname);
    SAFE_WRAP_EXIT("BACnet safe_datalink_init:", result = false)
    return result;
}

int bacnet_plugin_safe_bip_receive(
    BACNET_ADDRESS *src,
    uint8_t *npdu,
    uint16_t max_npdu,
    unsigned timeout)
{
    int result = 0;
    SAFE_WRAP_BEGIN
        result = bip_receive(src, npdu, max_npdu, timeout);
    SAFE_WRAP_EXIT("BACnet safe_bip_receive:", result = -1)
    return result;
}

void bacnet_plugin_safe_npdu_handler(
    BACNET_ADDRESS *src,
    uint8_t *npdu,
    uint16_t pdu_len)
{
    SAFE_WRAP_BEGIN
        npdu_handler(src, npdu, pdu_len);
    SAFE_WRAP_EXIT("BACnet safe_npdu_handler:", (void)0)
}
