#include <ctype.h>
#include <errno.h>
#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "bluez/lib/bluetooth.h"
#include "bluez/lib/hci.h"
#include "bluez/lib/hci_lib.h"
#include "bluez/src/shared/util.h"
#include "bluez/lib/uuid.h"
#include "bluez/btio/btio.h"
#include "bluez/attrib/att.h"
#include "bluez/attrib/gattrib.h"
#include "bluez/src/log.h"
#include "bluez/lib/sdp.h"
#include "bluez/lib/sdp_lib.h"
#include "bluez/attrib/gatt.h"
#include "bluez/attrib/gattrib.h"
#include "bluez/attrib/gatttool.h"
#include "bluez/btio/btio.h"

#define log_debug(...) write_log("DEBUG", __VA_ARGS__)
#define log_error(...) write_log("ERROR", __VA_ARGS__)

static GMainLoop *main_loop;
//static GAttrib *attrib;

static void write_log(const char *level, const char *fmt, ...)
{
    printf("%-6s", level);

    va_list args;
    va_start(args, fmt);
    
    vprintf(fmt, args);

    va_end(args);

    printf("\n");
}

/*
static void gatt_connect_callback(GIOChannel *io, GError *err, gpointer user_data)
{
    if (err)
    {
        log_error("%s", err->message);
        g_main_loop_quit(main_loop);
    }
    else
    {
        log_debug("Gatt connect succeeded");
        
        attrib = g_attrib_new(io, ATT_DEFAULT_LE_MTU, false);
    }
}

int ag_connect(const char *mac,
        const char *dst_type,
        const char *sec_level,
        const char *opt_src,
        connect_callback_t connect_callback,
        disconnect_callback_t disconnect_callback)
{
    // Since our devices follow the 48-bit universal standard, this is public.
    const int OPT_MTU = 0;
    const int OPT_PSM = 0;
    GError * gerr = NULL;

    log_debug("mac = %s", mac);
    log_debug("dst_type = %s", dst_type);
    log_debug("sec_level = %s", sec_level);
    log_debug("opt_src = %s", opt_src);

    gatt_connect(opt_src, mac, dst_type, sec_level, OPT_PSM,
            OPT_MTU, gatt_connect_callback, &gerr);

    if (gerr)
    {
        log_error("Failed to connect: '%s' (%d)", gerr->message, gerr->code);
        int tmp_err_code = gerr->code;
        g_error_free(gerr);
        g_main_loop_quit(main_loop);
        return tmp_err_code;
    }

    return 0;
}

int ag_disconnect(void)
{
    if (attrib)
    {
        log_debug("Disconnecting from device");

        g_attrib_unref(attrib);
        attrib = NULL;
    }

    return 0;
}

static void read_characteristic_callback(guint8 status, const guint8 *pdu, guint16 len, gpointer user_data)
{
    log_debug("Read response received with status %d, %d bytes of data", status, len);
}

int ag_read_characteristic(uint16_t handle, read_characteristic_callback_t callback)
{
    log_debug("Reading char handle %d", handle);
    return gatt_read_char(attrib, handle, read_characteristic_callback, callback);
}

int ag_hci_read_local_version(int hci_num, struct ag_hci_version  *ag_ver)
{
    struct hci_version ver;
    int hci_device_handle = -1;
    int err;
    char *hciver;

    hci_device_handle = hci_open_dev(hci_num);
    if(hci_device_handle < 0)
    {
        log_error("Failed to open HCI device for hci%d (err = %d)", hci_num, hci_device_handle);
        g_main_loop_quit(main_loop);
        return AG_READ_LOCAL_VERSION_FAILED;
    }

    err = hci_read_local_version(hci_device_handle, &ver, 1000);
    if (err < 0)
    {
        log_error("Failed to read local version for hci%d (err = %d)", hci_num, err);
        g_main_loop_quit(main_loop);
        return AG_READ_LOCAL_VERSION_FAILED;
    }

    hciver = hci_vertostr(ver.hci_ver);

    ag_ver->bt_major_ver = ver.hci_ver;
    ag_ver->bt_rev_ver = ver.hci_rev;
    ag_ver->bt_ver_str = hciver ? hciver : "n/a";

    hci_close_dev(hci_device_handle);

    return 0;
}

static gboolean disconnect_device(gpointer data)
{
    return FALSE;
}

static gboolean read_rssi(gpointer data)
{
    return FALSE;
}
*/

static gboolean connect_device(gpointer data)
{
    log_debug("Done");
    g_main_loop_quit(main_loop);

    return FALSE;
}

int main(void)
{
    log_debug("Setting up glib loop");
    main_loop = g_main_loop_new(NULL, FALSE);
    g_idle_add(connect_device, NULL);

    log_debug("Starting glib loop");
    g_main_loop_run(main_loop);

    log_debug("Glib loop stopped, unref'ing loop and exiting");
    g_main_loop_unref(main_loop);

    return 0;
}