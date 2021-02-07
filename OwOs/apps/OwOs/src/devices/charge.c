#include "devices/charge.h"
#include "battery/battery.h"

uint8_t batteryPercentage;
int batteryMv;

static int battery_change_callback(struct battery_prop_listener *listener, const struct battery_property *prop) {
    if(prop->bp_type == BATTERY_PROP_VOLTAGE_NOW) {
        batteryPercentage = max(0, min(100, ((prop->bp_value.bpv_voltage) - 3500) / 6));
        batteryMv = prop->bp_value.bpv_voltage;
    }
    return 0;
}

static struct battery_prop_listener battery_listener = {
    .bpl_prop_read = NULL,
    .bpl_prop_changed = battery_change_callback,
};

void initCharge() {
    int rc;

    struct os_dev *battery;

    battery = os_dev_open("battery", OS_TIMEOUT_NEVER, NULL);
    assert(battery);

    struct battery_property * prop_voltage = battery_find_property(
            battery, BATTERY_PROP_VOLTAGE_NOW, BATTERY_PROPERTY_FLAGS_NONE, NULL);
    assert(prop_voltage);

    rc = battery_prop_change_subscribe(&battery_listener, prop_voltage);
    assert(rc == 0);

    rc = battery_set_poll_rate_ms(battery, 1000);
    assert(rc == 0);
}

uint8_t getBatteryPercentage() {
    return batteryPercentage;
}

int getBatteryVoltageMv() {
    return batteryMv;
}