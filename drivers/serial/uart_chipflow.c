/*
 * Copyright (c) 2024 ChipFlow
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT chipflow_uart

#include <soc.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

struct uart_chipflow_regs {
	uint32_t tx_data;
	uint32_t rx_data;
	uint32_t tx_ready;
	uint32_t rx_avail;
	uint32_t divisor;
};

struct uart_chipflow_config {
	struct uart_chipflow_regs *regs;
	uint32_t clk_freq;
	uint32_t baudrate;
};

struct uart_chipflow_data {
};

static void uart_chipflow_poll_out(const struct device *dev, unsigned char c)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *uart = config->regs;

	while (!uart->tx_ready) {
	}

	uart->tx_data = (uint32_t)(c);
}

static int uart_chipflow_poll_in(const struct device *dev, unsigned char *c)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *uart = config->regs;

	if (!uart->rx_avail) {
		return -1;
	}

	*c = (unsigned char)(uart->rx_data & 0xFF);

	return 0;
}

static int uart_chipflow_init(const struct device *dev)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *uart = config->regs;

	uart->divisor = config->clk_freq / config->baudrate - 1;

	return 0;
}

static const struct uart_driver_api uart_chipflow_driver_api = {
	.poll_in = uart_chipflow_poll_in,
	.poll_out = uart_chipflow_poll_out,
	.err_check = NULL
};

static struct uart_chipflow_data uart_chipflow_data_0;

static const struct uart_chipflow_config uart_chipflow_config_0 = {
	.regs = (void *)DT_INST_REG_ADDR(0),
	.clk_freq = DT_INST_PROP(0, clock_frequency),
	.baudrate = DT_INST_PROP(0, current_speed)
};

DEVICE_DT_INST_DEFINE(0,
	uart_chipflow_init,
	NULL,
	&uart_chipflow_data_0,
	&uart_chipflow_config_0,
	PRE_KERNEL_1,
	CONFIG_SERIAL_INIT_PRIORITY,
	(void *)&uart_chipflow_driver_api);
