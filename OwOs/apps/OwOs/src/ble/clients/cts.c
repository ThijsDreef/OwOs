#include "ble/clients/cts.h"

const ble_uuid_t *ble_gatt_svc_cts = BLE_UUID16_DECLARE(0x1805);
const ble_uuid_t *ble_gatt_chr_cur_time = BLE_UUID16_DECLARE(0x2A2B);

const ble_uuid_t* get_cts_service_uuid() {
    return ble_gatt_svc_cts;
}

/// Data Format for Current Time Service. Based on https://github.com/sdalu/mynewt-nimble/blob/495ff291a15306787859a2fe8f2cc8765b546e02/nimble/host/services/cts/src/ble_svc_cts.c
struct ble_current_time {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint8_t  day_of_week;  //  From 1 (Monday) to 7 (Sunday)
    uint8_t  fraction256;
    uint8_t  adjust_reason;
} __attribute__((__packed__));


int ble_cts_read_current_time(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg) {
    const struct os_mbuf* om = attr->om;
        //  Get the Mbuf size
    uint16_t om_len = OS_MBUF_PKTLEN(om);

    //  Allocate storage for the BLE Current Time
    struct ble_current_time current_time;
    struct os_timeval time;
    struct os_timezone zone;
    struct clocktime ct;


    //  Copy the data from the Mbuf to the BLE Current Time
    ble_hs_mbuf_to_flat(  //  Flatten and copy the Mbuf...
        om,               //  From om...
		&current_time,    //  To current_time...
        om_len,           //  For om_len bytes
        NULL
    );

    os_gettimeofday(&time, &zone);

    ct.year = le16toh(current_time.year);
    ct.mon  = current_time.month;
    ct.day  = current_time.day;
    ct.hour = current_time.hours;
    ct.min  = current_time.minutes;
    ct.sec  = current_time.seconds;
    ct.usec = (current_time.fraction256 * 1000000) / 256;

    clocktime_to_timeval(&ct, &zone, &time);

    os_settimeofday(&time, NULL);
    return 0;
}

int ble_cts_characteristic_discover_callback(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg) {
    if (chr != NULL && ble_uuid_cmp(&chr->uuid.u, ble_gatt_chr_cur_time) == 0) {
        ble_gattc_read(conn_handle, chr->val_handle, ble_cts_read_current_time, NULL);
    }
    return 0;
}