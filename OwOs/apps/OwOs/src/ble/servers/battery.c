#include <nimble/ble.h>
#include "host/ble_hs.h"
#include "devices/charge.h"
#include "ble/servers/battery.h"

static const ble_uuid16_t batteryInformationServiceUuid = BLE_UUID16_INIT(0x180f);
static const ble_uuid16_t batteryLevelCharacteristicUuid = BLE_UUID16_INIT(0x2a19);

static int gattBatteryLevel(uint16_t connectionHandle, uint16_t attributeHandle, struct ble_gatt_access_ctxt* ctxt, void * arg) {
    if (ble_uuid_u16(ctxt->chr->uuid) == batteryLevelCharacteristicUuid.value) {
        uint8_t battery = getBatteryPercentage();
        os_mbuf_append(ctxt->om, &battery, 1);
        return 0;
    }

    return 0;
}

void initBatteryService() {
    // TODO find a way to collapse this has to be const initialized
    static const struct ble_gatt_chr_def batteryCharacteristics[]  = { {
            .uuid = &batteryLevelCharacteristicUuid.u,
            .access_cb = gattBatteryLevel,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0
        }
    };

    static const struct ble_gatt_svc_def batteryServerDefinition[] = {
        { 
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &batteryInformationServiceUuid.u,
            .characteristics = batteryCharacteristics
        }, {
            0
        },
    };

    int rc = 0;

    rc = ble_gatts_count_cfg(batteryServerDefinition);
    assert(rc == 0);

    ble_gatts_add_svcs(batteryServerDefinition);
    assert(rc == 0);
}