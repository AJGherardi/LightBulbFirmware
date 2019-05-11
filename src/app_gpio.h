#ifndef _APP_GPIO_H
#define _APP_GPIO_H

/* GPIO */
extern struct device *button_device[4];
extern struct device *led_device;

void app_gpio_init(void);

#endif
