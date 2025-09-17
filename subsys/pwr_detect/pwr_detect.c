/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>
#include <app/subsys/pwr_detect.h>

#define PWR_DETECT_STACK_SIZE 2048
#define PWR_DETECT_PRIORITY 5
#define PWR_SAMPLE_PERIOD_MS 1000

K_THREAD_STACK_DEFINE(pwr_detect_stack_area, PWR_DETECT_STACK_SIZE);
struct k_thread pwr_detect_thread_data;

static void pwr_detect_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(pwr_detect_work, pwr_detect_work_handler);

static int pwr_detect_init(void);

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
LOG_MODULE_REGISTER(pwr_detect, CONFIG_PWR_DETECT_LOG_LEVEL);

K_MUTEX_DEFINE(pwr_mutex);

static int16_t buf;
static struct adc_sequence sequence = {
	.buffer = &buf,
	/*buffer size in bytes, not number of samples*/
	.buffer_size = sizeof(buf),
	//Optional
	//.calibrate = true,
};

static void pwr_detect_work_handler(struct k_work *work)
{
	int err;
	/* Read a sample from the ADC */
	k_mutex_lock(&pwr_mutex, K_FOREVER);
	err = adc_read(adc_channel.dev, &sequence);
	k_mutex_unlock(&pwr_mutex);
	if (err < 0) {
		LOG_ERR("Could not read (%d)", err);
	}
}

static void  pwr_detect_thread(void *p1, void *p2, void *p3)
{
	while (true)
	{
		k_sleep(K_MSEC(PWR_SAMPLE_PERIOD_MS));
		k_work_schedule(&pwr_detect_work, K_NO_WAIT);
	}
}

int raw2mv(int raw)
{
	int err;
	k_mutex_lock(&pwr_mutex, K_FOREVER);
	err = adc_raw_to_millivolts_dt(&adc_channel, &raw);
	k_mutex_unlock(&pwr_mutex);
	/* conversion to mV may not be supported. skip if not */
	if (err < 0) {
		LOG_WRN(" (value in mV not available)\n");
		return err;
	} else {
		LOG_DBG(" = %d mV", raw);
		return raw;
	}
}

int get_pwr_raw(void)
{
	int raw_value;
	k_mutex_lock(&pwr_mutex, K_FOREVER);
	raw_value = (int)buf;
	k_mutex_unlock(&pwr_mutex);
	return (int)buf;
}

int get_pwr_source(void)
{
	int mv = get_pwr_raw();
	if (mv == 0) {
		return PWR_SOURCE_BATTERY;
	} else if (mv < 8000) {
		return PWR_SOURCE_USB;
	} else if (mv >= 8000){
		return PWR_SOURCE_12V;
	} else {
		return PWR_SOURCE_UNKNOWN;
	}
}

static int pwr_detect_init(void)
{
	LOG_INF("Initializing power detect subsystem...");
	int err;

	/* Validate that the ADC peripheral (SAADC) is ready */
	if (!adc_is_ready_dt(&adc_channel)) {
		LOG_ERR("ADC controller device %s not ready", adc_channel.dev->name);
		return 0;
	}
	/* Setup the ADC channel */
	err = adc_channel_setup_dt(&adc_channel);
	if (err < 0) {
		LOG_ERR("Could not setup channel #%d (%d)", 0, err);
		return 0;
	}
	/* Initialize the ADC sequence */
	err = adc_sequence_init_dt(&adc_channel, &sequence);
	if (err < 0) {
		LOG_ERR("Could not initalize sequence");
		return 0;
	}

	k_thread_create(&pwr_detect_thread_data, pwr_detect_stack_area,
			K_THREAD_STACK_SIZEOF(pwr_detect_stack_area),
			pwr_detect_thread,
			NULL, NULL, NULL,
			PWR_DETECT_PRIORITY, 0, K_NO_WAIT);
	return 0;
}

SYS_INIT(pwr_detect_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
