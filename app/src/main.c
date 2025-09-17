#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define BUTTON_NODE DT_ALIAS(sw0)
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

#define DEBOUNCE_DELAY_MS 500
static int64_t last_interrupt_time = 0;
void pin_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int64_t current_time = k_uptime_get();
	if ((current_time - last_interrupt_time) < DEBOUNCE_DELAY_MS) {
		// Debounce: Ignore interrupts that occur within the debounce interval
		return;
	}
	last_interrupt_time = current_time;
	// Handle the button press event here
	printf("Button Interupt\n");
	gpio_pin_toggle_dt(&led0);
	return; 
}

int main(void)
{
	int ret;
	//int val;
	static struct gpio_callback pin_cb_data;
//	bool led_state = true;


	if (!gpio_is_ready_dt(&led0)) {
		printf("led0 not ready\n");
		return 0;
	}
	
	if (!gpio_is_ready_dt(&led1)) {
		printf("led1 not ready\n");
		return 0;
	}
	
	if (!gpio_is_ready_dt(&led2)) {
		printf("led2 not ready\n");
		return 0;
	}

	
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("led0 failed to configure\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("led1 failed to configure\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("led2 failed to configure\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP | GPIO_ACTIVE_LOW);
	if (ret < 0) {
		printf("button failed to configure\n");
		return 0;
	}


	gpio_pin_interrupt_configure_dt(&button,GPIO_INT_EDGE_TO_ACTIVE);
	//ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);	
	gpio_init_callback(&pin_cb_data, pin_isr, BIT(button.pin));
	gpio_add_callback(button.port, &pin_cb_data);
	gpio_pin_set_dt(&led0, 1);

	while (1) {
		
		//ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0) {
			return 0;
		}

		k_msleep(SLEEP_TIME_MS);
		//ret = gpio_pin_set_dt(&led0, 0);
		//ret = gpio_pin_set_dt(&led1, 0);
		//ret = gpio_pin_set_dt(&led2, 0);
		if (ret < 0) {
			return 0;
		}
		ret = gpio_pin_set_dt(&led1, 1);
		//ret = gpio_pin_toggle_dt(&led1);
		if (ret < 0) {
			return 0;
		}

		k_msleep(SLEEP_TIME_MS);
		ret = gpio_pin_set_dt(&led1, 0);
		ret = gpio_pin_set_dt(&led2, 1);
		//ret = gpio_pin_toggle_dt(&led1);
		if (ret < 0) {
			return 0;
		}
		k_msleep(SLEEP_TIME_MS);
		ret = gpio_pin_set_dt(&led2, 0);
	//	ret = gpio_pin_toggle_dt(&led2);
		if (ret < 0) {
			return 0;
		}

		//val = gpio_pin_get_dt(&button);
		//printf("Waiting for button interrupt...\n");
		//break;
		//printf("Button state: %d\n", val);
	}
	return 0;
}
