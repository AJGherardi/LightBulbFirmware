#include <gpio.h>
#include <pwm.h>

#include "app_gpio.h"
#include "ble_mesh.h"
#include "device_composition.h"
#include "no_transition_work_handler.h"
#include "publisher.h"
#include "state_binding.h"
#include "storage.h"
#include "transition.h"

#if defined(CONFIG_MCUMGR)
#include <mgmt/smp_bt.h>
#include "smp_svr.h"
#endif

#define PERIOD (USEC_PER_SEC / 50U)


static bool reset;

static void light_default_var_init(void)
{
	gen_def_trans_time_srv_user_data.tt = 0x00;

	gen_power_onoff_srv_user_data.onpowerup = STATE_DEFAULT;

	light_lightness_srv_user_data.light_range_min = LIGHTNESS_MIN;
	light_lightness_srv_user_data.light_range_max = LIGHTNESS_MAX;
	light_lightness_srv_user_data.last = LIGHTNESS_MAX;
	light_lightness_srv_user_data.def = LIGHTNESS_MAX;

	light_ctl_srv_user_data.temp_range_min = TEMP_MIN;
	light_ctl_srv_user_data.temp_range_max = TEMP_MAX;
	light_ctl_srv_user_data.lightness_def = LIGHTNESS_MAX;
	light_ctl_srv_user_data.temp_def = TEMP_MIN;

	light_ctl_srv_user_data.lightness_temp_last =
		(u32_t) ((LIGHTNESS_MAX << 16) | TEMP_MIN);
}

static void light_default_status_init(void)
{
	u16_t lightness;

	lightness = (u16_t) (light_ctl_srv_user_data.lightness_temp_last >> 16);

	if (lightness) {
		gen_onoff_srv_root_user_data.onoff = STATE_ON;
	} else {
		gen_onoff_srv_root_user_data.onoff = STATE_OFF;
	}

	/* Retrieve Default Lightness & Temperature Values */

	if (light_ctl_srv_user_data.lightness_temp_def) {
		light_ctl_srv_user_data.lightness_def = (u16_t)
			(light_ctl_srv_user_data.lightness_temp_def >> 16);

		light_ctl_srv_user_data.temp_def = (u16_t)
			(light_ctl_srv_user_data.lightness_temp_def);
	}

	light_lightness_srv_user_data.def =
		light_ctl_srv_user_data.lightness_def;

	light_ctl_srv_user_data.temp = light_ctl_srv_user_data.temp_def;

	/* Retrieve Range of Lightness & Temperature */

	if (light_lightness_srv_user_data.lightness_range) {
		light_lightness_srv_user_data.light_range_max = (u16_t)
			(light_lightness_srv_user_data.lightness_range >> 16);

		light_lightness_srv_user_data.light_range_min = (u16_t)
			(light_lightness_srv_user_data.lightness_range);
	}

	if (light_ctl_srv_user_data.temperature_range) {
		light_ctl_srv_user_data.temp_range_max = (u16_t)
			(light_ctl_srv_user_data.temperature_range >> 16);

		light_ctl_srv_user_data.temp_range_min = (u16_t)
			(light_ctl_srv_user_data.temperature_range);
	}

	switch (gen_power_onoff_srv_user_data.onpowerup) {
	case STATE_OFF:
		gen_onoff_srv_root_user_data.onoff = STATE_OFF;
		state_binding(ONOFF, ONOFF_TEMP);
		break;
	case STATE_DEFAULT:
		gen_onoff_srv_root_user_data.onoff = STATE_ON;
		state_binding(ONOFF, ONOFF_TEMP);
		break;
	case STATE_RESTORE:
		light_lightness_srv_user_data.last = (u16_t)
			(light_ctl_srv_user_data.lightness_temp_last >> 16);

		light_ctl_srv_user_data.temp =
			(u16_t) (light_ctl_srv_user_data.lightness_temp_last);

		state_binding(ONPOWERUP, ONOFF_TEMP);
		break;
	}

	default_tt = gen_def_trans_time_srv_user_data.tt;

	target_lightness = lightness;
	target_temperature = temperature;
}

void update_led_gpio(void)
{
	u8_t power, color;

	power = 100 * ((float) lightness / 65535);
	color = 100 * ((float) (temperature + 32768) / 65535);

	printk("power-> %d, color-> %d\n", power, color);

	pwm_pin_set_usec(led_device,DT_NORDIC_NRF_PWM_PWM_0_CH0_PIN,PERIOD,(power * 10));
}

void update_light_state(void)
{
	update_led_gpio();

	if (*ptr_counter == 0 || reset == false) {
		reset = true;
		k_work_submit(&no_transition_work);
	}
}

static void short_time_multireset_bt_mesh_unprovisioning(void)
{
	if (reset_counter >= 4U) {
		reset_counter = 0U;
		printk("BT Mesh reset\n");
		bt_mesh_reset();
	} else {
		printk("Reset Counter -> %d\n", reset_counter);
		reset_counter++;
	}

	save_on_flash(RESET_COUNTER);
}

static void reset_counter_timer_handler(struct k_timer *dummy)
{
	reset_counter = 0U;
	save_on_flash(RESET_COUNTER);
	printk("Reset Counter set to Zero\n");
}

K_TIMER_DEFINE(reset_counter_timer, reset_counter_timer_handler, NULL);

void main(void)
{
	int err;

	light_default_var_init();

	app_gpio_init();

#if defined(CONFIG_MCUMGR)
	smp_svr_init();
#endif

	printk("Initializing...\n");

	ps_settings_init();

	bt_set_id_addr(BT_ADDR_LE_ANY);
	bt_set_name("lindazain");
	

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	light_default_status_init();

	update_light_state();

	randomize_publishers_TID();

	short_time_multireset_bt_mesh_unprovisioning();
	k_timer_start(&reset_counter_timer, K_MSEC(7000), 0);

#if defined(CONFIG_MCUMGR)
	/* Initialize the Bluetooth mcumgr transport. */
	smp_bt_register();

	k_timer_start(&smp_svr_timer, 0, K_MSEC(1000));
#endif
}
