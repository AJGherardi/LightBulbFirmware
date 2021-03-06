#include <gpio.h>

#include "publisher.h"

struct device *led_device;
struct device *button_device[4];

K_WORK_DEFINE(button_work, publish);

static void button_pressed(struct device *dev,
			   struct gpio_callback *cb, u32_t pins)
{
	k_work_submit(&button_work);
}

void app_gpio_init(void)
{
	static struct gpio_callback button_cb[4];

	/* LED controler configuration & setting */

	led_device = device_get_binding(DT_NORDIC_NRF_PWM_PWM_0_LABEL);

	/* Buttons configuration & setting */

	k_work_init(&button_work, publish);

	button_device[0] = device_get_binding(SW0_GPIO_CONTROLLER);
	gpio_pin_configure(button_device[0], SW0_GPIO_PIN,
			   (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |
			    GPIO_PUD_PULL_UP |
			    GPIO_INT_DEBOUNCE | GPIO_INT_ACTIVE_LOW));
	gpio_init_callback(&button_cb[0], button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button_device[0], &button_cb[0]);
	gpio_pin_enable_callback(button_device[0], SW0_GPIO_PIN);

	button_device[1] = device_get_binding(SW1_GPIO_CONTROLLER);
	gpio_pin_configure(button_device[1], SW1_GPIO_PIN,
			   (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |
			    GPIO_PUD_PULL_UP |
			    GPIO_INT_DEBOUNCE | GPIO_INT_ACTIVE_LOW));
	gpio_init_callback(&button_cb[1], button_pressed, BIT(SW1_GPIO_PIN));
	gpio_add_callback(button_device[1], &button_cb[1]);
	gpio_pin_enable_callback(button_device[1], SW1_GPIO_PIN);

	button_device[2] = device_get_binding(SW2_GPIO_CONTROLLER);
	gpio_pin_configure(button_device[2], SW2_GPIO_PIN,
			   (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |
			    GPIO_PUD_PULL_UP |
			    GPIO_INT_DEBOUNCE | GPIO_INT_ACTIVE_LOW));
	gpio_init_callback(&button_cb[2], button_pressed, BIT(SW2_GPIO_PIN));
	gpio_add_callback(button_device[2], &button_cb[2]);
	gpio_pin_enable_callback(button_device[2], SW2_GPIO_PIN);

	button_device[3] = device_get_binding(SW3_GPIO_CONTROLLER);
	gpio_pin_configure(button_device[3], SW3_GPIO_PIN,
			   (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |
			    GPIO_PUD_PULL_UP |
			    GPIO_INT_DEBOUNCE | GPIO_INT_ACTIVE_LOW));
	gpio_init_callback(&button_cb[3], button_pressed, BIT(SW3_GPIO_PIN));
	gpio_add_callback(button_device[3], &button_cb[3]);
	gpio_pin_enable_callback(button_device[3], SW3_GPIO_PIN);
}
