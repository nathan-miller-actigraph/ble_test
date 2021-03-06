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

#define TIME_WIDTH (25)

#define HCI_NUMBER 0

#define log_debug(...) write_log("DEBUG", __VA_ARGS__)
#define log_info(...) write_log("INFO", __VA_ARGS__)
#define log_error(...) write_log("ERROR", __VA_ARGS__)

static gboolean read_rssi(gpointer data);
static gboolean disconnect_device(gpointer data);

static GMainLoop *main_loop;
static GAttrib *attrib;

static void write_log(const char *level, const char *fmt, ...)
{
    char time_string[TIME_WIDTH];
    struct tm * timeinfo;
    time_t current_time = time(NULL);
    timeinfo = localtime (&current_time);

    strftime(time_string, TIME_WIDTH, "%c", timeinfo);

    printf("%-24s %-6s", time_string, level);

    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);

    va_end(args);

    printf("\n");
}

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

        attrib = g_attrib_new(io, 23, false);

        g_idle_add(read_rssi, NULL);
    }
}

void ag_connect(const char *mac,
        const char *dst_type,
        const char *sec_level,
        const char *opt_src)
{
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

        g_error_free(gerr);
        g_main_loop_quit(main_loop);
    }
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
    log_debug("RSSI = %d", (gint8)pdu[1]);

    g_idle_add(disconnect_device, NULL);
}

int ag_read_characteristic(uint16_t handle)
{
    log_debug("Reading char handle %d", handle);
    return gatt_read_char(attrib, handle, read_characteristic_callback, read_characteristic_callback);
}

void ag_hci_read_local_version(int hci_num)
{
    struct hci_version ver;
    int hci_device_handle = -1;
    int err;
    char *hciver = NULL;

    hci_device_handle = hci_open_dev(hci_num);
    if(hci_device_handle < 0)
    {
        log_error("Failed to open HCI device for hci%d (err = %d)", hci_num, hci_device_handle);
        g_main_loop_quit(main_loop);
        return;
    }

    err = hci_read_local_version(hci_device_handle, &ver, 1000);
    if (err < 0)
    {
        log_error("Failed to read local version for hci%d (err = %d)", hci_num, err);
        hci_close_dev(hci_device_handle);
        g_main_loop_quit(main_loop);
        return;
    }

    hciver = hci_vertostr(ver.hci_ver);

    if (hciver)
    {
        log_debug("HCI version for hci%d: %s", hci_num, hciver);
    }
    else
    {
        log_error("Failed to read HCI version");
        g_main_loop_quit(main_loop);
    }

    hci_close_dev(hci_device_handle);
}

static gboolean disconnect_device(gpointer data)
{
    ag_disconnect();
    g_main_loop_quit(main_loop);

    return FALSE;
}

static gboolean read_rssi(gpointer data)
{
    ag_read_characteristic(28);

    return FALSE;
}

static gboolean connect_device(gpointer data)
{
    char hci_name[5];
    snprintf(hci_name, 5, "hci%d", HCI_NUMBER);

    char *mac = (char *)data;
    log_debug("Connecting to %s", mac);

    ag_connect(mac, "random", "low", hci_name);

    return FALSE;
}

static gboolean read_local_version(gpointer data)
{
    ag_hci_read_local_version(HCI_NUMBER);

    g_idle_add(connect_device, data);

    return FALSE;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        log_error("No MAC provided");
        return -1;
    }

    log_info("START");

    char *mac = argv[1];

    log_debug("Setting up glib loop");
    main_loop = g_main_loop_new(NULL, FALSE);
    g_idle_add(read_local_version, mac);

    log_debug("Starting glib loop");
    g_main_loop_run(main_loop);

    log_debug("Glib loop stopped, unref'ing loop and exiting");
    g_main_loop_unref(main_loop);

    log_info("END");

    return 0;
}
