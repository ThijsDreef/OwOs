#include "nimble/ble.h"
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "datetime/datetime.h"

int ble_cts_characteristic_discover_callback(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr, void *arg);
const ble_uuid_t* get_cts_service_uuid();