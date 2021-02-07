#include <nimble/ble.h>
#include <host/ble_hs.h>
#include "ble/servers/deviceInfo.h"

#define MANUFACTURER_ID 0x2a29
#define MODEL_NUMBER_ID 0x2a24
#define SERIAL_NUMBER_ID 0x2a25
#define FW_REVISION_ID 0x2a26
#define HW_REVISION_ID 0x2a27
#define PNP_ID 0x2a50

static const char* manufacturer = "Dreef";
static const char* modelNumber = "1";
static const char* serialNumber = "9.8.7.6.5.4";
static const char* firmwareRevision = "0.0.1";
static const char* hardwareRevision = "1.0.0";

static const ble_uuid16_t deviceInfoId = BLE_UUID16_INIT(0x180a);
static const ble_uuid16_t manufacturerNameId = BLE_UUID16_INIT(MANUFACTURER_ID);
static const ble_uuid16_t modelnumberId = BLE_UUID16_INIT(MODEL_NUMBER_ID);
static const ble_uuid16_t serialNumberId = BLE_UUID16_INIT(SERIAL_NUMBER_ID);
static const ble_uuid16_t fwRevisionId = BLE_UUID16_INIT(FW_REVISION_ID);
static const ble_uuid16_t hwRevisionId = BLE_UUID16_INIT(HW_REVISION_ID);
static const ble_uuid16_t pnpId = BLE_UUID16_INIT(PNP_ID);
// hardware related information
static const uint8_t pnpBuff[7] = { 0x02, 0xc4, 0x10, 0x01, 0x00, 0x01, 0x00 };

static int deviceInfoRequest(uint16_t connectionHandle, uint16_t attributeHandle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    uint16_t size;
    const void* str;
    uint16_t value = ble_uuid_u16(ctxt->chr->uuid);
    switch (value) {
        case MANUFACTURER_ID:
            str = manufacturer;
            size = strlen(str);
            break;
        case MODEL_NUMBER_ID:
            str = modelNumber;
            size = strlen(str);
            break;
        case SERIAL_NUMBER_ID:
            str = serialNumber;
            size = strlen(str);
            break;
        case FW_REVISION_ID:
            str = firmwareRevision;
            size = strlen(str);
            break;
        case HW_REVISION_ID:
            str = hardwareRevision;
            size = strlen(str);
            break;
        case PNP_ID:
            str = pnpBuff;
            size = 7;
            break;
        default:
            return 0;
            break;
    }

    os_mbuf_append(ctxt->om, str, size);
    return 0;
}

void initDeviceInfoService() {
    // TODO find a way to collapse this has to be const initialized
    static const struct ble_gatt_chr_def deviceInfoCharacteristics[]  = { {
            .uuid = &manufacturerNameId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            .uuid = &modelnumberId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            .uuid = &serialNumberId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            .uuid = &fwRevisionId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            .uuid = &hwRevisionId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            .uuid = &pnpId.u,
            .access_cb = deviceInfoRequest,
            .flags = BLE_GATT_CHR_F_READ
        }, {
            0
        }
    };

    static const struct ble_gatt_svc_def deviceInfoServerDefinition[] = {
        { 
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = &deviceInfoId.u,
            .characteristics = deviceInfoCharacteristics
        }, {
            0
        },
    };

    int rc = 0;

    rc = ble_gatts_count_cfg(deviceInfoServerDefinition);
    assert(rc == 0);

    ble_gatts_add_svcs(deviceInfoServerDefinition);
    assert(rc == 0);
}