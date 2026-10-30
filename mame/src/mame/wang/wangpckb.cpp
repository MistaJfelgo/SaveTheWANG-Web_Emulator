// license:BSD-3-Clause
// copyright-holders:Curt Coder
// Modified for Save the Wang, 2026-10-30. See SOURCE_CHANGES.md at repository root.
/**********************************************************************

    Wang PC keyboard emulation

*********************************************************************/

/*

PCB Layout
----------

Key Tronic A65-02454-002
PCB 201

|-----------------------------------------------------------------------|
|        LS273  PSG  555  LS04  4MHz  LS132  CN1            LS374   XR  |
|                                                    CPU                |
|     LD1           LD2           LD3           LD4           LD5       |
|        SW1              LS259              SW2          LS259         |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    Relevant IC's shown.

    CPU         - Intel P8051AH "20-8051-225"
    PSG         - Texas Instruments SN76496N
    XR          - Exar Semiconductor XR22-908-03B? (hard to read)
    CN1         - 1x6 pin PCB header
    SW1         - 8-way DIP switch
    SW2         - 8-way DIP switch

*/

/*

    TODO:

    - dip switches

*/

#include "emu.h"
#include "wangpckb.h"

#include "speaker.h"

#include "utf8.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define I8051_TAG       "z5"
#define SN76496_TAG     "z4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_KEYBOARD, wangpc_keyboard_device, "wangpckb", "Wang PC Keyboard")



//-------------------------------------------------
//  ROM( wangpc_keyboard )
//-------------------------------------------------

ROM_START( wangpc_keyboard )
	ROM_REGION( 0x1000, I8051_TAG, 0 )
	ROM_LOAD( "20-8051-225.z5", 0x0000, 0x1000, CRC(82d2999f) SHA1(2bb34a1de2d94b2885d9e8fcd4964296f6276c5c) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wangpc_keyboard_device::device_rom_region() const
{
	return ROM_NAME( wangpc_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_keyboard_data )
//-------------------------------------------------

void wangpc_keyboard_device::wangpc_keyboard_data(address_map &map)
{
	//map(0x0000, 0xfeff).nopr();
	map(0x47, 0x58).mirror(0xff00).nopr();
	map(0x00, 0x00).mirror(0xff00).w(SN76496_TAG, FUNC(sn76496_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void wangpc_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8051(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_DATA, &wangpc_keyboard_device::wangpc_keyboard_data);
	m_maincpu->port_in_cb<1>().set(FUNC(wangpc_keyboard_device::kb_p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(wangpc_keyboard_device::kb_p1_w));
	m_maincpu->port_out_cb<2>().set(FUNC(wangpc_keyboard_device::kb_p2_w));
	m_maincpu->port_in_cb<3>().set(FUNC(wangpc_keyboard_device::kb_p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(wangpc_keyboard_device::kb_p3_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76496(config, SN76496_TAG, 2000000).add_route(ALL_OUTPUTS, "mono", 1.00); // ???
}


//-------------------------------------------------
//  INPUT_PORTS( wangpc_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( wangpc_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) // 2b
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // 63
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) // 53
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) // 52
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 43
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) // 29
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x96") // 26
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 2e

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') // 64
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // 1f
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // 21
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("GL") // 54
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')// 45
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r') // 44
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // 35
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // 2d

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') // 65
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_CHAR('~') // 2f
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // 56
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(']') PORT_CHAR('[') // 55
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') // 46
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // 22
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 23
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR('\\') // 37

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') // 67
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') // 66
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // 58
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // 57
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // 48
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // 47
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // 38
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // 2a

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR(U'£') // 69
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') // 68
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // 5a
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // 59
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // 4a
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // 49
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // 3a
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // 39

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 6b
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // 6a
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // 5c
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // 5b
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // 4c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // 4b
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // 3c
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') // 3b

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') // 6d
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // 6c
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // 5e
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // 5d
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // 4e
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // 4d
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // 3e
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // 3d

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\xA4") PORT_CODE(KEYCODE_TILDE) // 1f
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // 6e
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x87\xA5") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') // 6f
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // 5f
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // 1e
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // 4f
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // 1b
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // 3f

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_F8) // 77
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SRCH") PORT_CODE(KEYCODE_F9) // 76
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ENTER") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) // 1c
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad . ;") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) // 32
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 33
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0 '") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) // 34
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 35
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // 28

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MERGE") PORT_CODE(KEYCODE_F6) // 79
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NOTE") PORT_CODE(KEYCODE_F7) // 78
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GOTO") // 11
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CANCEL") // 12
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	// 2ND is a second-level shift: 2ND+/ = backslash, 2ND+- = tilde
	// (the BIOS keyboard tables have no dedicated keys for these)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2ND") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2) // 25

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEC TAB") PORT_CODE(KEYCODE_F4) // 7b
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FORMAT") PORT_CODE(KEYCODE_F5) // 7a
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x95") // 71
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("(blank)") // 70
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') // 2c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 27

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAGE") PORT_CODE(KEYCODE_F2) // 7d
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CENTER") PORT_CODE(KEYCODE_F3) // 7c
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MOVE") PORT_CODE(KEYCODE_F12) // 73
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMMAND") // 72
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_ESC) // 36
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INDENT") PORT_CODE(KEYCODE_F1) // 7e
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPLC") PORT_CODE(KEYCODE_F10) // 75
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY") PORT_CODE(KEYCODE_F11) // 74
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) // 24
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // 1d
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"Keypad ×") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) // 14
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"Keypad ÷") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) // 18
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT") // 19
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // 20
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("ERASE") // 1a
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 1c

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad + *") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))// 60
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad - !") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) // 13
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8 >") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) // 15
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9 " UTF8_UP) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) // 16
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5 ]") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) // 40
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6 $") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) // 17
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3 #") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) // 10
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 1b

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) // 62
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PREV") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP)) // 61
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEXT") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) // 51
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7 <") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) // 50
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // 42
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4 [") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) // 41
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1 &") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) // 31
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2 @") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) // 30

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor wangpc_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wangpc_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_keyboard_device - constructor
//-------------------------------------------------

wangpc_keyboard_device::wangpc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_KEYBOARD, tag, owner, clock),
	m_maincpu(*this, I8051_TAG),
	m_y(*this, "Y%u", 0),
	m_txd_handler(*this),
	m_leds(*this, "led%u", 0U),
	m_keylatch(0),
	m_rxd(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_keyboard_device::device_start()
{
	save_item(NAME(m_keylatch));
	save_item(NAME(m_rxd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_keyboard_device::device_reset()
{
	m_txd_handler(1);
}


//-------------------------------------------------
//  write_rxd -
//-------------------------------------------------

void wangpc_keyboard_device::write_rxd(int state)
{
	m_rxd = state;
}


//-------------------------------------------------
//  kb_p1_r -
//-------------------------------------------------

uint8_t wangpc_keyboard_device::kb_p1_r()
{
	return m_y[m_keylatch]->read();
}


//-------------------------------------------------
//  kb_p1_w -
//-------------------------------------------------

void wangpc_keyboard_device::kb_p1_w(uint8_t data)
{
	/*

	    bit     description

	    P1.0    led 0
	    P1.1    led 1
	    P1.2    led 2
	    P1.3    led 3
	    P1.4    led 4
	    P1.5    led 5
	    P1.6
	    P1.7

	*/

	for (int i = 0; i < 6; i++)
	{
		m_leds[i] = BIT(~data, i);
	}

	//if (LOG) logerror("P1 %02x\n", data);
}


//-------------------------------------------------
//  kb_p2_w -
//-------------------------------------------------

void wangpc_keyboard_device::kb_p2_w(uint8_t data)
{
	/*

	    bit     description

	    P2.0    row 0
	    P2.1    row 1
	    P2.2    row 2
	    P2.3    row 3
	    P2.4
	    P2.5
	    P2.6    chip enable for something
	    P2.7

	*/

	m_keylatch = data & 0x0f;

	//if (LOG) logerror("P2 %02x\n", data);
}


//-------------------------------------------------
//  kb_p3_r -
//-------------------------------------------------

uint8_t wangpc_keyboard_device::kb_p3_r()
{
	return m_rxd;
}


//-------------------------------------------------
//  kb_p3_w -
//-------------------------------------------------

void wangpc_keyboard_device::kb_p3_w(uint8_t data)
{
	/*

	    bit     description

	    P3.0    RxD
	    P3.1    TxD
	    P3.2
	    P3.3    chip enable for something
	    P3.4
	    P3.5
	    P3.6
	    P3.7

	*/

	//if (LOG) logerror("P3 %02x\n", data);
	m_txd_handler(BIT(data, 1));
}


// High-level protocol reference: number42net/wang-keyboard-adapter (wire behavior only).
// Serial framing follows the motherboard's IM6402 setup: 62500 baud, 8N2.
DEFINE_DEVICE_TYPE(WANGPC_HLE_KEYBOARD, wangpc_hle_keyboard_device, "wangpckb_hle", "Wang PC Keyboard (high-level)")

wangpc_hle_keyboard_device::wangpc_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, WANGPC_HLE_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, m_rows(*this, "Y%u", 0)
	, m_txd_handler(*this)
	, m_leds(*this, "led%u", 0U)
{
}

ioport_constructor wangpc_hle_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wangpc_keyboard);
}

void wangpc_hle_keyboard_device::device_start()
{
	m_scan_timer = timer_alloc(FUNC(wangpc_hle_keyboard_device::scan_keys), this);
	m_repeat_timer = timer_alloc(FUNC(wangpc_hle_keyboard_device::repeat_key), this);
	save_item(NAME(m_keys));
	save_item(NAME(m_key_up));
	save_item(NAME(m_pending));
	save_item(NAME(m_parameter));
	save_item(NAME(m_remaining));
	save_item(NAME(m_last_key));
	save_item(NAME(m_repeat));
	save_item(NAME(m_led_state));
	save_item(NAME(m_volume));
	save_item(NAME(m_all_key_up));
}

void wangpc_hle_keyboard_device::reset_protocol()
{
	std::fill(std::begin(m_key_up), std::end(m_key_up), 0);
	m_pending = m_parameter = m_remaining = m_last_key = m_repeat = 0;
	m_led_state = m_volume = 0;
	m_all_key_up = false;
	m_repeat_timer->reset();
	for (int i = 0; i < 6; ++i)
		m_leds[i] = 0;
}

void wangpc_hle_keyboard_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(62500);
	set_tra_rate(62500);
	clear_fifo();
	receive_register_reset();
	transmit_register_reset();
	m_txd_handler(1);
	reset_protocol();
	std::fill(std::begin(m_keys), std::end(m_keys), 0xff);
	m_scan_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void wangpc_hle_keyboard_device::tra_callback()
{
	m_txd_handler(transmit_register_get_data_bit());
}

void wangpc_hle_keyboard_device::received_byte(u8 byte)
{
	if (m_remaining)
	{
		if (m_pending <= 5)
			m_key_up[m_parameter] = byte;
		else if (m_pending == 0x20 && !m_parameter)
			m_volume = byte;
		++m_parameter;
		--m_remaining;
		return;
	}
	logerror("HLE keyboard command %02X\n", byte);
	if (byte >= 1 && byte <= 5)
	{
		std::fill(std::begin(m_key_up), std::end(m_key_up), 0);
		m_all_key_up = false;
		m_pending = byte;
		m_remaining = byte;
		m_parameter = 0;
	}
	else if (byte >= 0x10 && byte <= 0x1b)
	{
		u8 const mask = 1U << ((byte - 0x10) / 2);
		m_led_state = BIT(byte, 0) ? (m_led_state & ~mask) : (m_led_state | mask);
	}
	else switch (byte)
	{
	case 0x00: break;
	case 0x06: m_all_key_up = true; break;
	case 0x07:
		std::fill(std::begin(m_key_up), std::end(m_key_up), 0);
		m_all_key_up = false;
		break;
	case 0x08: reset_protocol(); break;
	case 0x09: transmit_byte(0x01); transmit_byte(0x33); break;
	case 0x0a: case 0x0b: break; // ponytail: click/beep synthesis deferred; serial keyboard input works without sound.
	case 0x0c: transmit_byte(m_last_key); break;
	case 0x0f: transmit_byte(0x01); transmit_byte(m_volume); break;
	case 0x1c: m_led_state = 0x3f; break;
	case 0x1d: m_led_state = 0; break;
	case 0x1e: transmit_byte(0x01); transmit_byte(m_led_state); break;
	case 0x20: case 0x31: case 0x32:
		m_pending = byte;
		m_parameter = 0;
		m_remaining = byte == 0x31 ? 1 : 2;
		break; // sound-generator data is consumed without synthesizing sound
	default: logerror("HLE keyboard unsupported command %02X\n", byte); break;
	}
	for (int i = 0; i < 6; ++i)
		m_leds[i] = BIT(m_led_state, i);
}

void wangpc_hle_keyboard_device::send_key(u8 code)
{
	m_last_key = code;
	transmit_byte(code);
}

TIMER_CALLBACK_MEMBER(wangpc_hle_keyboard_device::repeat_key)
{
	if (m_repeat)
		send_key(m_repeat);
}

TIMER_CALLBACK_MEMBER(wangpc_hle_keyboard_device::scan_keys)
{
	static constexpr u8 codes[16][8] = {
		{ 0x2b, 0x00, 0x53, 0x52, 0x00, 0x29, 0x26, 0x2e },
		{ 0x64, 0x00, 0x00, 0x54, 0x45, 0x44, 0x35, 0x2d },
		{ 0x65, 0x2f, 0x56, 0x55, 0x46, 0x00, 0x00, 0x37 },
		{ 0x67, 0x66, 0x58, 0x57, 0x48, 0x47, 0x38, 0x2a },
		{ 0x69, 0x68, 0x5a, 0x59, 0x4a, 0x49, 0x3a, 0x39 },
		{ 0x6b, 0x6a, 0x5c, 0x5b, 0x4c, 0x4b, 0x3c, 0x3b },
		{ 0x6d, 0x6c, 0x5e, 0x5d, 0x4e, 0x4d, 0x3e, 0x3d },
		{ 0x1f, 0x6e, 0x6f, 0x5f, 0x1e, 0x4f, 0x1b, 0x3f },
		{ 0x77, 0x76, 0x1c, 0x32, 0x00, 0x34, 0x00, 0x28 },
		{ 0x79, 0x78, 0x11, 0x12, 0x00, 0x00, 0x00, 0x25 },
		{ 0x7b, 0x7a, 0x71, 0x70, 0x2c, 0x00, 0x00, 0x00 },
		{ 0x7d, 0x7c, 0x73, 0x72, 0x00, 0x00, 0x00, 0x00 },
		{ 0x36, 0x7e, 0x75, 0x74, 0x24, 0x00, 0x00, 0x00 },
		{ 0x14, 0x18, 0x19, 0x00, 0x1a, 0x00, 0x00, 0x00 },
		{ 0x60, 0x13, 0x15, 0x16, 0x40, 0x17, 0x10, 0x00 },
		{ 0x62, 0x61, 0x51, 0x50, 0x00, 0x41, 0x31, 0x30 },
	};
	for (int row = 0; row < 16; ++row)
	{
		u8 const keys = m_rows[row]->read();
		u8 const changed = keys ^ m_keys[row];
		m_keys[row] = keys;
		for (int col = 0; col < 8; ++col)
		{
			u8 const code = codes[row][col];
			if (!code || !BIT(changed, col))
				continue;
			if (!BIT(keys, col))
			{
				send_key(code);
				if (code != 0x1b && code != 0x35 && code != 0x24 && code != 0x25 && code != 0x1e)
				{
					m_repeat = code;
					m_repeat_timer->adjust(attotime::from_msec(500), 0, attotime::from_msec(33));
				}
			}
			else
			{
				if (m_repeat == code)
				{
					m_repeat = 0;
					m_repeat_timer->reset();
				}
				if (m_all_key_up || code == 0x1b || code == 0x35 || std::find(std::begin(m_key_up), std::end(m_key_up), code) != std::end(m_key_up))
					send_key(code | 0x80);
			}
		}
	}
}
