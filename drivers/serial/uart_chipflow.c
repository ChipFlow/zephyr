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
	uint32_t config;
	uint32_t phy_config;
	uint32_t status;
	uint32_t data;
};

struct uart_chipflow_config {
	struct uart_chipflow_regs *rx;
	struct uart_chipflow_regs *tx;
	uint32_t clk_freq;
	uint32_t baudrate;
};

static void uart_chipflow_poll_out(const struct device *dev, unsigned char c)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *tx = config->tx;

	while (!(tx->status & 1)) {
	}

	tx->data = (uint32_t)(c);
}

static int uart_chipflow_poll_in(const struct device *dev, unsigned char *c)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *rx = config->rx;

	if (!(rx->status & 1)) {
		return -1;
	}

	*c = (unsigned char)(rx->data & 0xFF);

	return 0;
}

static int uart_chipflow_init(const struct device *dev)
{
	const struct uart_chipflow_config *const config = dev->config;
	volatile struct uart_chipflow_regs *rx = config->rx;
	volatile struct uart_chipflow_regs *tx = config->tx;

	// disable the UART
	rx->config = 0;
	tx->config = 0;

	// set the baudrate divisor
	rx->phy_config = (uint32_t)((config->clk_freq / config->baudrate - 1) & 0x00FFFFFF);
	tx->phy_config = (uint32_t)((config->clk_freq / config->baudrate - 1) & 0x00FFFFFF);

	// enable the UART
	rx->config = 1;
	tx->config = 1;

	return 0;
}

static const struct uart_driver_api uart_chipflow_driver_api = {
	.poll_in = uart_chipflow_poll_in,
	.poll_out = uart_chipflow_poll_out,
	.err_check = NULL
};

static const struct uart_chipflow_config uart_chipflow_config_0 = {
	.rx = (void *)(DT_INST_REG_ADDR(0) + 0x000U),
	.tx = (void *)(DT_INST_REG_ADDR(0) + 0x200U),
	.clk_freq = DT_INST_PROP(0, clock_frequency),
	.baudrate = DT_INST_PROP(0, current_speed)
};

DEVICE_DT_INST_DEFINE(0,
	uart_chipflow_init,
	NULL,
	NULL,
	&uart_chipflow_config_0,
	PRE_KERNEL_1,
	CONFIG_SERIAL_INIT_PRIORITY,
	(void *)&uart_chipflow_driver_api);
