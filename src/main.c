/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
/* STEP 3 - Include the header file of the I2C API */
#include <zephyr/drivers/i2c.h>

/* STEP 4.1 - Include the header file of printk() */
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(logging_blog);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* STEP 7 - Define the I2C slave device address and the addresses of relevant registers */
#define I2C_NODE DT_NODELABEL(mysensor)
#define INA260 DT_NODELABEL(adafruitina260)
#define CTRLMEAS 0xF4
#define CALIB00	 0x88
#define ID	     0xD0


#define CHIP_ID 0x60

#define INA260_ID 0xff
#define INA260_MANU 0xfe
#define CONFIGURATION_REGISTER 0x00
#define CURRENT_REGISTER 0x01
#define BUS_VOLTAGE_REGISTER 0x02
#define POWER_REGISTER 0x03

/* STEP 5 - Get the label of the I2C controller connected to your sensor */

int read_register(const struct i2c_dt_spec *ina260, uint8_t register_address, uint8_t* result)
{
	int ret;
	uint8_t bytes[2];
	uint8_t register_array[] = {register_address};

	ret = i2c_write_read_dt(ina260, register_array, 1, &bytes, 2);
	if (ret != 0) {
		printk("Failed to read register %x \n", register_array[0]);
		return -1;
	}
	//printk("Bytes: %x%x \n", bytes[0], bytes[1]);

	memcpy(result, bytes, 2);

	return 0;
}

void read_manufacturer(const struct i2c_dt_spec *ina260)//const struct device * ina260) 
{
	int ret;
	uint16_t manufacturer_id = 0;
	uint8_t manufacturer_bytes[2];
	uint8_t manufacturer_register[] = {INA260_MANU};
	ret = i2c_write_read_dt(ina260, manufacturer_register, 1, &manufacturer_bytes, 2);
	if (ret != 0) {
		printk("Failed to read register %x \n", manufacturer_register[0]);
		return -1;
	}
	printk("Manufacturer: %x%x \n", manufacturer_bytes[0], manufacturer_bytes[1]);
	//printk("Manufacturer: %x \n", manufacturer_bytes[0]);

	//manufacturer_id = ((manufacturer_bytes[1] << 8) | manufacturer_bytes[0]);
}

void read_chip_id(const struct i2c_dt_spec *ina260)//const struct device * ina260) 
{
	int ret;
	uint16_t manufacturer_id = 0;
	uint8_t manufacturer_bytes[2];
	uint8_t manufacturer_register[] = {INA260_MANU};
	ret = i2c_write_read_dt(ina260, manufacturer_register, 1, &manufacturer_bytes, 2);
	if (ret != 0) {
		printk("Failed to read register %x \n", manufacturer_register[0]);
		return -1;
	}
	printk("Manufacturer: %x%x \n", manufacturer_bytes[0], manufacturer_bytes[1]);

}

void main(void)
{

	int ret;

	printk("Application starting...\n\r");

/* STEP 6 - Get the binding of the I2C driver  */
	//static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);
	static const struct i2c_dt_spec ina260 = I2C_DT_SPEC_GET(INA260);
	if (!device_is_ready(ina260.bus)) {
		printk("INA260 bus %s is not ready!\n\r",ina260.bus->name);
		return -1;
	}

	uint8_t config[3] = {CONFIGURATION_REGISTER, 0x69, 0x27};
	ret = i2c_write_dt(&ina260, config, sizeof(config));
	if(ret != 0){
		printk("Failed to write to I2C device address %x at reg. %x \n\r", ina260.addr, config[0]);
	}

	k_msleep(500);

	uint8_t manufacturer[2] = {0};
	read_register(&ina260, INA260_MANU, manufacturer);
	printk("Manufacturer: %x%x \n", manufacturer[0], manufacturer[1]);
	uint8_t chip_id[2] = {0};
	read_register(&ina260, INA260_ID, chip_id);
	printk("Chip id: %x%x \n", chip_id[0], chip_id[1]);
	uint8_t configs[2] = {0};
	read_register(&ina260, CONFIGURATION_REGISTER, configs);
	printk("Config: %x%x \n", configs[0], configs[1]);

	uint8_t current[2] = {0};
	uint8_t voltage[2] = {0};
	uint8_t power[2] = {0};
	int16_t current_converted;
	int16_t voltage_converted;
	uint16_t power_converted;
	
	read_register(&ina260, CURRENT_REGISTER, current);
	printk("Current: %x%x \n", current[0], current[1]);
	
	read_register(&ina260, BUS_VOLTAGE_REGISTER, voltage);
	printk("Voltage: %x%x \n", voltage[0], voltage[1]);
	
	read_register(&ina260, POWER_REGISTER, power);
	printk("Power: %x%x \n", power[0], power[1]);

	while(1) {
		read_register(&ina260, CURRENT_REGISTER, current);
		printk("Current: %x%x \n", current[0], current[1]);
		current_converted = (int16_t) ((current[0] << 8) + current[1]);
		printk("Current value: %f \n", 1.25 * current_converted);
		/*current_converted = (int16_t) ((current[0] << 8) + current[1]);
		printk("Current value: %f \n", current_converted*1.25);*/
		
		
		read_register(&ina260, BUS_VOLTAGE_REGISTER, voltage);
		printk("Voltage: %x%x \n", voltage[0], voltage[1]);
		voltage_converted = (int16_t) ((voltage[0] << 8) + voltage[1]);
		printk("Voltage value: %f \n", 1.25 * voltage_converted);
		
		read_register(&ina260, POWER_REGISTER, power);
		printk("Power: %x%x \n", power[0], power[1]);
		power_converted = ((power[0] << 8) + power[1]);
		printk("Power value: %d \n", 10 * power_converted);
		
	k_msleep(1000);
	}
/*
	// do it this way:
//uint8_t b; int16_t value;
ret = i2c_read(&ina260, value, 2, CURRENT_REGISTER);
i2c_read_blocking(p->i2cbus_port, ina260_addr, u.b, 2, false);
value=(b[0]<<8)|b[1]; fp=(float)value * LSB;
*/



}
