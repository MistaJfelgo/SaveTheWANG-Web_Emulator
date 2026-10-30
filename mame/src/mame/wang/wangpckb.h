// license:BSD-3-Clause
// copyright-holders:Curt Coder
// Modified for Save the Wang, 2026-10-30. See SOURCE_CHANGES.md at repository root.
/**********************************************************************

    Wang PC keyboard emulation

*********************************************************************/

#ifndef MAME_WANG_WANGPCKB_H
#define MAME_WANG_WANGPCKB_H

#pragma once

#include "cpu/mcs51/i8051.h"
#include "diserial.h"

#include "sound/sn76496.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpc_keyboard_device

class wangpc_keyboard_device :  public device_t
{
public:
	// construction/destruction
	wangpc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }

	void write_rxd(int state);

	void wangpc_keyboard_data(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<i8051_device> m_maincpu;
	required_ioport_array<16> m_y;
	devcb_write_line m_txd_handler;
	output_finder<6> m_leds;

	uint8_t m_keylatch;
	int m_rxd;

	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
	uint8_t kb_p3_r();
	void kb_p3_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_KEYBOARD, wangpc_keyboard_device)

// Optional high-level serial keyboard; preserves the original MCU-based device.
class wangpc_hle_keyboard_device : public device_t, public device_buffered_serial_interface<64U>
{
public:
	wangpc_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	auto txd_handler() { return m_txd_handler.bind(); }
	void write_rxd(int state) { rx_w(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void tra_callback() override;
	virtual void received_byte(u8 byte) override;

private:
	TIMER_CALLBACK_MEMBER(scan_keys);
	TIMER_CALLBACK_MEMBER(repeat_key);
	void reset_protocol();
	void send_key(u8 code);
	required_ioport_array<16> m_rows;
	devcb_write_line m_txd_handler;
	output_finder<6> m_leds;
	emu_timer *m_scan_timer = nullptr;
	emu_timer *m_repeat_timer = nullptr;
	u8 m_keys[16] = {};
	u8 m_key_up[5] = {};
	u8 m_pending = 0;
	u8 m_parameter = 0;
	u8 m_remaining = 0;
	u8 m_last_key = 0;
	u8 m_repeat = 0;
	u8 m_led_state = 0;
	u8 m_volume = 0;
	bool m_all_key_up = false;
};

DECLARE_DEVICE_TYPE(WANGPC_HLE_KEYBOARD, wangpc_hle_keyboard_device)

#endif // MAME_WANG_WANGPCKB_H
