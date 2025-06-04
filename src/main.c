// #include <inttypes.h>
// #include <stddef.h>
// #include <stdint.h>
// #include <math.h>

// #include <zephyr/device.h>
// #include <zephyr/devicetree.h>
// #include <zephyr/drivers/adc.h>
// #include <zephyr/kernel.h>
// #include <zephyr/sys/printk.h>
// #include <zephyr/sys/util.h>

// #if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
//     !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
// #error "No suitable devicetree overlay specified"
// #endif

// #define DT_SPEC_AND_COMMA(node_id, prop, idx) \
//     ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

// /* Data of ADC io-channels specified in devicetree. */
// static const struct adc_dt_spec adc_channels[] = {
//     DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
//                          DT_SPEC_AND_COMMA)
// };

// // Constants for thermistor calculation
// #define R1 10000.0f // 10kΩ series resistor
// #define C1 1.009249522e-03f
// #define C2 2.378405444e-04f
// #define C3 2.019202697e-07f

// int main(void)
// {
//     int err;
//     uint16_t buf;
//     float R2, logR2, T, Tc, Tf;

//     struct adc_sequence sequence = {
//         .buffer = &buf,
//         .buffer_size = sizeof(buf),
//     };

//     size_t channel_index = 1;
//     if (!device_is_ready(adc_channels[channel_index].dev)) {
//         printk("ADC controller device %s not ready\n", adc_channels[channel_index].dev->name);
//         return 0;
//     }

//     err = adc_channel_setup_dt(&adc_channels[channel_index]);
//     if (err < 0) {
//         printk("Could not setup channel #%zu (%d)\n", channel_index, err);
//         return 0;
//     }

//     while (1) {
//         int32_t val_mv;

//         (void)adc_sequence_init_dt(&adc_channels[channel_index], &sequence);
//         err = adc_read(adc_channels[channel_index].dev, &sequence);
//         if (err < 0) {
//             printk("Could not read (%d)\n", err);
//             continue;
//         }

//         if (adc_channels[channel_index].channel_cfg.differential) {
//             val_mv = (int32_t)((int16_t)buf);
//         } else {
//             val_mv = (int32_t)buf;
//         }

//         err = adc_raw_to_millivolts_dt(&adc_channels[channel_index], &val_mv);
//         if (err < 0) {
//             printk("ADC reading failed to convert to millivolts\n");
//             continue;
//         }

//         // --- Thermistor Logic ---
//         // Vout = val_mv in millivolts
//         // Vcc assumed 3300mV
//         float Vout = (float)val_mv;
//         float Vcc = 3300.0f;

//         // Calculate thermistor resistance
//         R2 = R1 * (Vcc / Vout - 1.0f);
//         logR2 = logf(R2);

//         // Steinhart-Hart Equation
//         T = 1.0f / (C1 + C2 * logR2 + C3 * logR2 * logR2 * logR2);
//         Tc = T - 273.15f;
//         // Tf = (Tc * 9.0f) / 5.0f + 32.0f;

//         // Print results
//         // printk("ADC raw: %u, Voltage: %d mV\n", buf, val_mv);
//         // printk("Temperature: %.2f °C; %.2f °F\n", Tc, Tf);
//         printk("Temperature: %.2f °C\n", Tc);

//         k_sleep(K_MSEC(1000));
//     }

//     return 0;
// }


#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                         DT_SPEC_AND_COMMA)
};

// Constants for thermistor calculation
#define R1 10000.0f // 10kΩ series resistor
#define C1 1.009249522e-03f
#define C2 2.378405444e-04f
#define C3 2.019202697e-07f

int main(void)
{
    int err;
    uint16_t buf;
    float R2, logR2, T, Tc;

    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };

    size_t channel_index = 1;
    if (!device_is_ready(adc_channels[channel_index].dev)) {
        printk("ADC controller device %s not ready\n", adc_channels[channel_index].dev->name);
        return 0;
    }

    err = adc_channel_setup_dt(&adc_channels[channel_index]);
    if (err < 0) {
        printk("Could not setup channel #%zu (%d)\n", channel_index, err);
        return 0;
    }

    while (1) {
        int32_t val_mv;

        (void)adc_sequence_init_dt(&adc_channels[channel_index], &sequence);
        err = adc_read(adc_channels[channel_index].dev, &sequence);
        if (err < 0) {
            printk("Could not read (%d)\n", err);
            continue;
        }

        if (adc_channels[channel_index].channel_cfg.differential) {
            val_mv = (int32_t)((int16_t)buf);
        } else {
            val_mv = (int32_t)buf;
        }

        err = adc_raw_to_millivolts_dt(&adc_channels[channel_index], &val_mv);
        if (err < 0) {
            printk("ADC reading failed to convert to millivolts\n");
            continue;
        }

        // --- Thermistor Logic ---
        // Vout = val_mv in millivolts
        // Vcc assumed 3300mV
        float Vout = (float)val_mv;
        float Vcc = 3300.0f;

        // Calculate thermistor resistance
        R2 = R1 * (Vcc / Vout - 1.0f);
        logR2 = logf(R2);

        // Steinhart-Hart Equation
        T = 1.0f / (C1 + C2 * logR2 + C3 * logR2 * logR2 * logR2);
        Tc = T - 273.15f;

        // Print results
        printk("Temperature: %.2f °C\n", Tc);

        k_sleep(K_MSEC(1000));
    }

    return 0;
}
