#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "ble/ble.h"
#include "ble/clients/cts.h"

const char* device_name = "pinetime OWOS";

int ble_gap_callback(struct ble_gap_event *event, void* arg);


void ble_set_addr() { 

    int rc;
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
}

void ble_advertise() {
    if(ble_gap_adv_active()) return;
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    uint8_t own_addr_type;
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);

    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    static const ble_uuid16_t hidServiceId = BLE_UUID16_INIT(0x1812);


    fields.uuids16 = &hidServiceId;
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    fields.name = (uint8_t*)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    fields.appearance = 962;
    fields.appearance_is_present = 1;

    
    rc = ble_gap_adv_set_fields(&fields);
    assert(rc == 0);

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_callback, NULL);
    assert(rc == 0);

}

void ble_on_sync(void) {
    /* Generate a non-resolvable private address. */
    ble_set_addr();

    /* Advertise indefinitely. */
    ble_advertise();
}

int ble_gatt_discovery_callback(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_svc *service, void *arg) {
    if (service == NULL) return 0;
    if (ble_uuid_cmp(&service->uuid.u, get_cts_service_uuid()) == 0) {
        ble_gattc_disc_all_chrs(conn_handle, service->start_handle, service->end_handle, ble_cts_characteristic_discover_callback, NULL);
    }
    return 0;
}

int ble_gap_callback(struct ble_gap_event *event, void* arg) {
    
    // struct ble_gap_conn_desc desc;
    // int rc;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed. */
            if (event->connect.status != 0) {
                ble_advertise();
            } else {
                ble_gap_security_initiate(event->connect.conn_handle);
                // assert(rc == 0);
                ble_gattc_disc_all_svcs(event->connect.conn_handle, ble_gatt_discovery_callback, NULL);
            }
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            /* Connection terminated; resume advertising. */
            ble_advertise();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            return 0;
        case BLE_GAP_EVENT_ENC_CHANGE:
            return 0;
        case (BLE_GAP_EVENT_REPEAT_PAIRING):
            /* Delete the old bond. */ {
                struct ble_gap_conn_desc desc;
                ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
                ble_store_util_delete_peer(&desc.peer_id_addr);
                return BLE_GAP_REPEAT_PAIRING_RETRY;
            }
            return 0;
        case BLE_GAP_EVENT_SUBSCRIBE:
            return 0;
        default:
            break;
    }

    return 0;
}

void initBle() {
    ble_hs_cfg.sync_cb = ble_on_sync;
}