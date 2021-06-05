/***************************************************************************
 *   Copyright (C) 2020 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN                             *
 *                         Frederik Saraci IU2NRO                          *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

/*
 * The graphical user interface (GUI) works by splitting the screen in
 * horizontal rows, with row height depending on vertical resolution.
 *
 * The general screen layout is composed by an upper status bar at the
 * top of the screen and a lower status bar at the bottom.
 * The central portion of the screen is filled by two big text/number rows
 * And a small row.
 *
 * Below is shown the row height for two common display densities.
 *
 *        160x128 display (MD380)            Recommended font size
 *      ┌─────────────────────────┐
 *      │  top_status_bar (16px)  │  8 pt (11 px) font with 2 px vertical padding
 *      │      top_pad (4px)      │  4 px padding
 *      │      Line 1 (20px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │      Line 2 (20px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │                         │
 *      │      Line 3 (40px)      │  16 pt (xx px) font with 6 px vertical padding
 *      │ RSSI+squelch bar (20px) │  20 px
 *      │      bottom_pad (4px)   │  4 px padding
 *      └─────────────────────────┘
 *
 *         128x64 display (GD-77)
 *      ┌─────────────────────────┐
 *      │  top_status_bar (11 px) │  6 pt (9 px) font with 1 px vertical padding
 *      │      top_pad (1px)      │  1 px padding
 *      │      Line 1 (10px)      │  6 pt (9 px) font without vertical padding
 *      │      Line 2 (10px)      │  6 pt (9 px) font with 2 px vertical padding
 *      │      Line 3 (18px)      │  12 pt (xx px) font with 0 px vertical padding
 *      │ RSSI+squelch bar (11px) │  11 px
 *      │      bottom_pad (1px)   │  1 px padding
 *      └─────────────────────────┘
 *
 *         128x48 display (RD-5R)
 *      ┌─────────────────────────┐
 *      │  top_status_bar (11 px) │  6 pt (9 px) font with 1 px vertical padding
 *      ├─────────────────────────┤  1 px line
 *      │      Line 2 (10px)      │  8 pt (11 px) font with 4 px vertical padding
 *      │      Line 3 (18px)      │  8 pt (11 px) font with 4 px vertical padding
 *      └─────────────────────────┘
 */

#include <stdio.h>
#include <stdint.h>
#include <ui.h>
#include <rtx.h>
#include <interfaces/platform.h>
#include <interfaces/nvmem.h>
#ifdef HAS_GPS
#include <interfaces/gps.h>
#endif
#include <string.h>
#include <battery.h>
#include <input.h>
#include <hwconfig.h>

/* UI main screen functions, their implementation is in "ui_main.c" */
extern void _ui_drawMainBackground();
extern void _ui_drawMainTop();
extern void _ui_drawVFOMiddle();
extern void _ui_drawMEMMiddle();
extern void _ui_drawVFOBottom();
extern void _ui_drawMEMBottom();
extern void _ui_drawMainVFO();
extern void _ui_drawMainVFOInput(ui_state_t* ui_state);
extern void _ui_drawMainMEM();
/* UI menu functions, their implementation is in "ui_menu.c" */
extern void _ui_drawMenuTop(ui_state_t* ui_state);
extern void _ui_drawMenuZone(ui_state_t* ui_state);
extern void _ui_drawMenuChannel(ui_state_t* ui_state);
extern void _ui_drawMenuContacts(ui_state_t* ui_state);
#ifdef HAS_GPS
extern void _ui_drawMenuGPS();
extern void _ui_drawSettingsGPS(ui_state_t* ui_state);
#endif
extern void _ui_drawMenuSettings(ui_state_t* ui_state);
extern void _ui_drawMenuInfo(ui_state_t* ui_state);
extern void _ui_drawMenuAbout();
#ifdef HAS_RTC
extern void _ui_drawSettingsTimeDate();
extern void _ui_drawSettingsTimeDateSet(ui_state_t* ui_state);
#endif
extern void _ui_drawSettingsDisplay(ui_state_t* ui_state);
extern bool _ui_drawMacroMenu();

const char *menu_items[] =
{
    "Zone",
    "Channels",
    "Contacts",
#ifdef HAS_GPS
    "GPS",
#endif
    "Settings",
    "Info",
    "About"
};

const char *settings_items[] =
{
    "Display",
#ifdef HAS_RTC
    "Time & Date",
#endif
#ifdef HAS_GPS
    "GPS"
#endif
};

const char *display_items[] =
{
    "Brightness",
#ifdef SCREEN_CONTRAST
    "Contrast"
#endif
};

#ifdef HAS_GPS
const char *settings_gps_items[] =
{
    "GPS Enabled",
    "GPS Set Time",
    "UTC Timezone"
};
#endif

const char *info_items[] =
{
    "",
    "Bat. Voltage",
    "Bat. Charge",
    "RSSI",
    "Model",
    "Band",
    "VHF",
    "UHF",
    "LCD Type"
};

const char *authors[] =
{
    "Niccolo' IU2KIN",
    "Silvano IU2KWO",
    "Federico IU2NUO",
    "Fred IU2NRO",
};

// Calculate number of menu entries
const uint8_t menu_num = sizeof(menu_items)/sizeof(menu_items[0]);
const uint8_t settings_num = sizeof(settings_items)/sizeof(settings_items[0]);
const uint8_t display_num = sizeof(display_items)/sizeof(display_items[0]);
#ifdef HAS_GPS
const uint8_t settings_gps_num = sizeof(settings_gps_items)/sizeof(settings_gps_items[0]);
#endif
const uint8_t info_num = sizeof(info_items)/sizeof(info_items[0]);
const uint8_t author_num = sizeof(authors)/sizeof(authors[0]);

const color_t color_black = {0, 0, 0, 255};
const color_t color_grey = {60, 60, 60, 255};
const color_t color_white = {255, 255, 255, 255};
const color_t yellow_fab413 = {250, 180, 19, 255};

layout_t layout;
state_t last_state;
ui_state_t ui_state;
bool macro_menu = false;
bool layout_ready = false;
bool redraw_needed = true;

layout_t _ui_calculateLayout()
{
    // Horizontal line height
    const uint16_t hline_h = 1;
    // Compensate for fonts printing below the start position
    const uint16_t text_v_offset = 1;

    // Calculate UI layout depending on vertical resolution
    // Tytera MD380, MD-UV380
    #if SCREEN_HEIGHT > 127

    // Height and padding shown in diagram at beginning of file
    const uint16_t top_h = 16;
    const uint16_t top_pad = 4;
    const uint16_t line1_h = 20;
    const uint16_t line2_h = 20;
    const uint16_t line3_h = 40;
    const uint16_t menu_h = 16;
    const uint16_t bottom_h = 20;
    const uint16_t bottom_pad = top_pad;
    const uint16_t status_v_pad = 2;
    const uint16_t small_line_v_pad = 2;
    const uint16_t big_line_v_pad = 6;
    const uint16_t horizontal_pad = 4;

    // Top bar font: 8 pt
    const fontSize_t top_font = FONT_SIZE_8PT;
    // Text line font: 8 pt
    const fontSize_t line1_font = FONT_SIZE_8PT;
    const fontSize_t line2_font = FONT_SIZE_8PT;
    // Frequency line font: 16 pt
    const fontSize_t line3_font = FONT_SIZE_16PT;
    // Bottom bar font: 8 pt
    const fontSize_t bottom_font = FONT_SIZE_8PT;
    // TimeDate/Frequency input font
    const fontSize_t input_font = FONT_SIZE_12PT;
    // Menu font
    const fontSize_t menu_font = FONT_SIZE_8PT;

    // Radioddity GD-77
    #elif SCREEN_HEIGHT > 63

    // Height and padding shown in diagram at beginning of file
    const uint16_t top_h = 11;
    const uint16_t top_pad = 1;
    const uint16_t line1_h = 10;
    const uint16_t line2_h = 10;
    const uint16_t line3_h = 16;
    const uint16_t menu_h = 10;
    const uint16_t bottom_h = 8;
    const uint16_t bottom_pad = 0;
    const uint16_t status_v_pad = 1;
    const uint16_t small_line_v_pad = 1;
    const uint16_t big_line_v_pad = 0;
    const uint16_t horizontal_pad = 4;

    // Top bar font: 6 pt
    const fontSize_t top_font = FONT_SIZE_6PT;
    // Middle line fonts: 5, 8, 8 pt
    const fontSize_t line1_font = FONT_SIZE_6PT;
    const fontSize_t line2_font = FONT_SIZE_6PT;
    const fontSize_t line3_font = FONT_SIZE_10PT;
    // Bottom bar font: 6 pt
    const fontSize_t bottom_font = FONT_SIZE_6PT;
    // TimeDate/Frequency input font
    const fontSize_t input_font = FONT_SIZE_8PT;
    // Menu font
    const fontSize_t menu_font = FONT_SIZE_6PT;

    // Radioddity RD-5R
    #elif SCREEN_HEIGHT > 47

    // Height and padding shown in diagram at beginning of file
    const uint16_t top_h = 11;
    const uint16_t top_pad = 1;
    const uint16_t line1_h = 0;
    const uint16_t line2_h = 10;
    const uint16_t line3_h = 18;
    const uint16_t menu_h = 10;
    const uint16_t bottom_h = 0;
    const uint16_t bottom_pad = 0;
    const uint16_t status_v_pad = 1;
    const uint16_t small_line_v_pad = 1;
    const uint16_t big_line_v_pad = 0;
    const uint16_t horizontal_pad = 4;

    // Top bar font: 8 pt
    const fontSize_t top_font = FONT_SIZE_6PT;
    // Middle line fonts: 16, 16
    const fontSize_t line2_font = FONT_SIZE_6PT;
    const fontSize_t line3_font = FONT_SIZE_12PT;
    // TimeDate/Frequency input font
    const fontSize_t input_font = FONT_SIZE_8PT;
    // Menu font
    const fontSize_t menu_font = FONT_SIZE_6PT;
    // Not present on this resolution
    const fontSize_t line1_font = 0;
    const fontSize_t bottom_font = 0;

    #else
    #error Unsupported vertical resolution!
    #endif

    // Calculate printing positions
    point_t top_pos    = {horizontal_pad, top_h - status_v_pad - text_v_offset};
    point_t line1_pos  = {horizontal_pad, top_h + top_pad + line1_h - small_line_v_pad - text_v_offset};
    point_t line2_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h - small_line_v_pad - text_v_offset};
    point_t line3_pos  = {horizontal_pad, top_h + top_pad + line1_h + line2_h + line3_h - big_line_v_pad - text_v_offset};
    point_t bottom_pos = {horizontal_pad, SCREEN_HEIGHT - bottom_pad - status_v_pad - text_v_offset};

    layout_t new_layout =
    {
        hline_h,
        top_h,
        line1_h,
        line2_h,
        line3_h,
        menu_h,
        bottom_h,
        status_v_pad,
        horizontal_pad,
        text_v_offset,
        top_pos,
        line1_pos,
        line2_pos,
        line3_pos,
        bottom_pos,
        top_font,
        line1_font,
        line2_font,
        line3_font,
        bottom_font,
        input_font,
        menu_font
    };
    return new_layout;
}


void ui_init()
{
    redraw_needed = true;
    layout = _ui_calculateLayout();
    layout_ready = true;
    // Initialize struct ui_state to all zeroes
    // This syntax is called compound literal
    // https://stackoverflow.com/questions/6891720/initialize-reset-struct-to-zero-null
    ui_state = (const struct ui_state_t){ 0 };
}

void ui_drawSplashScreen(bool centered)
{
    gfx_clearScreen();
    point_t splash_origin = {0,0};
    #ifdef OLD_SPLASH
    if(centered)
        splash_origin.y = SCREEN_HEIGHT / 2 + 6;
    else
        splash_origin.y = SCREEN_HEIGHT / 4;
    gfx_print(splash_origin, FONT_SIZE_12PT, TEXT_ALIGN_CENTER, yellow_fab413, "OpenRTX");
    #else
    if(centered)
        splash_origin.y = SCREEN_HEIGHT / 2 - 6;
    else
        splash_origin.y = SCREEN_HEIGHT / 5;
    gfx_print(splash_origin, FONT_SIZE_12PT, TEXT_ALIGN_CENTER, yellow_fab413, "O P N\nR T X");
    #endif
}

void _ui_drawLowBatteryScreen()
{
    gfx_clearScreen();
    uint16_t bat_width = SCREEN_WIDTH / 2;
    uint16_t bat_height = SCREEN_HEIGHT / 3;
    point_t bat_pos = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 8};
    gfx_drawBattery(bat_pos, bat_width, bat_height, 0.1f);
    point_t text_pos_1 = {0, SCREEN_HEIGHT * 2 / 3};
    point_t text_pos_2 = {0, SCREEN_HEIGHT * 2 / 3 + 16};

    gfx_print(text_pos_1,
              FONT_SIZE_6PT,
              TEXT_ALIGN_CENTER,
              color_white,
              "For emergency use");
    gfx_print(text_pos_2,
              FONT_SIZE_6PT,
              TEXT_ALIGN_CENTER,
              color_white,
              "press any button.");
}

freq_t _ui_freq_add_digit(freq_t freq, uint8_t pos, uint8_t number)
{
    freq_t coefficient = 10;
    for(uint8_t i=0; i < FREQ_DIGITS - pos; i++)
    {
        coefficient *= 10;
    }
    return freq += number * coefficient;
}

#ifdef HAS_RTC
void _ui_timedate_add_digit(curTime_t *timedate, uint8_t pos, uint8_t number)
{
    switch(pos)
    {
        // Set date
        case 1:
            timedate->date += number * 10;
            break;
        case 2:
            timedate->date += number;
            break;
        // Set month
        case 3:
            timedate->month += number * 10;
            break;
        case 4:
            timedate->month += number;
            break;
        // Set year
        case 5:
            timedate->year += number * 10;
            break;
        case 6:
            timedate->year += number;
            break;
        // Set hour
        case 7:
            timedate->hour += number * 10;
            break;
        case 8:
            timedate->hour += number;
            break;
        // Set minute
        case 9:
            timedate->minute += number * 10;
            break;
        case 10:
            timedate->minute += number;
            break;
    }
}
#endif

bool _ui_freq_check_limits(freq_t freq)
{
    bool valid = false;
    const hwInfo_t* hwinfo = platform_getHwInfo();
    if(hwinfo->vhf_band)
    {
        // hwInfo_t frequencies are in MHz
        if(freq >= (hwinfo->vhf_minFreq * 1000000) && 
           freq <= (hwinfo->vhf_maxFreq * 1000000))
        valid = true; 
    }
    if(hwinfo->uhf_band)
    {
        // hwInfo_t frequencies are in MHz
        if(freq >= (hwinfo->uhf_minFreq * 1000000) && 
           freq <= (hwinfo->uhf_maxFreq * 1000000))
        valid = true; 
    }
    return valid;
}

bool _ui_channel_valid(channel_t* channel)
{
return _ui_freq_check_limits(channel->rx_frequency) &&
       _ui_freq_check_limits(channel->tx_frequency);
}

bool _ui_drawDarkOverlay() {
    color_t alpha_grey = {0, 0, 0, 255};
    point_t origin = {0, 0};
    gfx_drawRect(origin, SCREEN_WIDTH, SCREEN_HEIGHT, alpha_grey, true);
    return true;
}

int _ui_fsm_loadChannel(uint16_t zone_index, bool *sync_rtx) {
    uint16_t channel_index = zone_index;
    channel_t channel;
    // If a zone is active, get index from current zone
    if(state.zone_enabled)
    {
        // Calculate zone size
        const uint8_t zone_size = sizeof(state.zone.member)/sizeof(state.zone.member[0]);
        if((zone_index <= 0) || (zone_index > zone_size))
            return -1;
        else
            // Channel index is 1-based while zone array access is 0-based
            channel_index = state.zone.member[zone_index - 1];
    }
    int result = nvm_readChannelData(&channel, channel_index);
    // Read successful and channel is valid
    if(result != -1 && _ui_channel_valid(&channel))
    {
        // Set new channel index
        state.channel_index = zone_index;
        // Copy channel read to state
        state.channel = channel;
        *sync_rtx = true;
    }
    return result;
}

void _ui_fsm_confirmVFOInput(bool *sync_rtx) {
    // Switch to TX input
    if(ui_state.input_set == SET_RX)
    {
        ui_state.input_set = SET_TX;
        // Reset input position
        ui_state.input_position = 0;
    }
    else if(ui_state.input_set == SET_TX)
    {
        // Save new frequency setting
        // If TX frequency was not set, TX = RX
        if(ui_state.new_tx_frequency == 0)
        {
            ui_state.new_tx_frequency = ui_state.new_rx_frequency;
        }
        // Apply new frequencies if they are valid
        if(_ui_freq_check_limits(ui_state.new_rx_frequency) &&
           _ui_freq_check_limits(ui_state.new_tx_frequency))
        {
            state.channel.rx_frequency = ui_state.new_rx_frequency;
            state.channel.tx_frequency = ui_state.new_tx_frequency;
            *sync_rtx = true;
        }
        state.ui_screen = MAIN_VFO;
    }
}

void _ui_fsm_insertVFONumber(kbd_msg_t msg, bool *sync_rtx) {
    // Advance input position
    ui_state.input_position += 1;
    // Save pressed number to calculate frequency and show in GUI
    ui_state.input_number = input_getPressedNumber(msg);
    if(ui_state.input_set == SET_RX)
    {
        if(ui_state.input_position == 1)
            ui_state.new_rx_frequency = 0;
        // Calculate portion of the new RX frequency
        ui_state.new_rx_frequency = _ui_freq_add_digit(ui_state.new_rx_frequency,
                                ui_state.input_position, ui_state.input_number);
        if(ui_state.input_position >= FREQ_DIGITS)
        {
            // Switch to TX input
            ui_state.input_set = SET_TX;
            // Reset input position
            ui_state.input_position = 0;
            // Reset TX frequency
            ui_state.new_tx_frequency = 0;
        }
    }
    else if(ui_state.input_set == SET_TX)
    {
        if(ui_state.input_position == 1)
            ui_state.new_tx_frequency = 0;
        // Calculate portion of the new TX frequency
        ui_state.new_tx_frequency = _ui_freq_add_digit(ui_state.new_tx_frequency,
                                ui_state.input_position, ui_state.input_number);
        if(ui_state.input_position >= FREQ_DIGITS)
        {
            // Save both inserted frequencies
            if(_ui_freq_check_limits(ui_state.new_rx_frequency) &&
               _ui_freq_check_limits(ui_state.new_tx_frequency))
            {
                state.channel.rx_frequency = ui_state.new_rx_frequency;
                state.channel.tx_frequency = ui_state.new_tx_frequency;
                *sync_rtx = true;
            }
            state.ui_screen = MAIN_VFO;
        }
    }
}

void _ui_changeBrightness(int variation)
{
    if(variation >= 0)
        state.settings.brightness =
        (255 - state.settings.brightness < variation) ? 255 : state.settings.brightness + variation;
    else
        state.settings.brightness =
        (state.settings.brightness < -variation) ? 0 : state.settings.brightness + variation;
    platform_setBacklightLevel(state.settings.brightness);
}

void _ui_changeContrast(int variation)
{
    if(variation >= 0)
        state.settings.contrast =
        (255 - state.settings.contrast < variation) ? 255 : state.settings.contrast + variation;
    else
        state.settings.contrast =
        (state.settings.contrast < -variation) ? 0 : state.settings.contrast + variation;
    display_setContrast(state.settings.contrast);
}

void _ui_fsm_menuMacro(kbd_msg_t msg, bool *sync_rtx) {
    ui_state.input_number = input_getPressedNumber(msg);
    // CTCSS Encode/Decode Selection
    bool tone_tx_enable = state.channel.fm.txToneEn;
    bool tone_rx_enable = state.channel.fm.rxToneEn;
    uint8_t tone_flags = tone_tx_enable << 1 | tone_rx_enable;
    switch(ui_state.input_number)
    {
        case 1:
            state.channel.fm.txTone++;
            state.channel.fm.txTone %= MAX_TONE_INDEX;
            state.channel.fm.rxTone = state.channel.fm.txTone;
            *sync_rtx = true;
            break;
        case 2:
            tone_flags++;
            tone_flags %= 4;
            tone_tx_enable = tone_flags >> 1;
            tone_rx_enable = tone_flags & 1;
            state.channel.fm.txToneEn = tone_tx_enable;
            state.channel.fm.rxToneEn = tone_rx_enable;
            *sync_rtx = true;
            break;
        case 3:
            if (state.channel.power == 1.0f)
                state.channel.power = 5.0f;
            else
                state.channel.power = 1.0f;
            *sync_rtx = true;
            break;
        case 4:
            state.channel.bandwidth++;
            state.channel.bandwidth %= 3;
            *sync_rtx = true;
            break;
        case 5:
            if(state.channel.mode == FM)
                state.channel.mode = DMR;
            else if(state.channel.mode == DMR)
                state.channel.mode = FM;
            *sync_rtx = true;
            break;
        case 7:
            _ui_changeBrightness(+25);
            break;
        case 8:
            _ui_changeBrightness(-25);
            break;
    }

#ifdef HAS_ABSOLUTE_KNOB // If the radio has an absolute position knob
    if(msg.keys & KEY_LEFT || msg.keys & KEY_RIGHT) {
        state.sqlLevel = platform_getChSelector() - 1;
        *sync_rtx = true;
    }
#else // Use left and right buttons or relative position knob
    // NOTE: Use up and down for UV380 which has not yet a functional knob
    if(msg.keys & KEY_LEFT || msg.keys & KEY_DOWN) {
        state.sqlLevel = (state.sqlLevel == 0) ? 0 : state.sqlLevel - 1;
        *sync_rtx = true;
    }
    else if(msg.keys & KEY_RIGHT || msg.keys & KEY_UP) {
        state.sqlLevel = (state.sqlLevel == 15) ? 15 : state.sqlLevel + 1;
        *sync_rtx = true;
    }
#endif
}

void _ui_menuUp(uint8_t menu_entries)
{
    if(ui_state.menu_selected > 0)
        ui_state.menu_selected -= 1;
    else
        ui_state.menu_selected = menu_entries - 1;
}

void _ui_menuDown(uint8_t menu_entries)
{
    if(ui_state.menu_selected < menu_entries - 1)
        ui_state.menu_selected += 1;
    else
        ui_state.menu_selected = 0;
}

void _ui_menuBack(uint8_t prev_state)
{
    if(ui_state.edit_mode)
    {
        ui_state.edit_mode = false;
    }
    else
    {
        // Return to previous menu
        state.ui_screen = prev_state;
        // Reset menu selection
        ui_state.menu_selected = 0;
    }
}

void ui_saveState()
{
    last_state = state;
}

void ui_updateFSM(event_t event, bool *sync_rtx)
{
    // User wants to power off the radio, so shutdown.
    if(!platform_pwrButtonStatus())
    {
        state_terminate();
        platform_terminate();
        return;
    }

    // Check if battery has enough charge to operate.
    // Check is skipped if there is an ongoing transmission, since the voltage
    // drop caused by the RF PA power absorption causes spurious triggers of
    // the low battery alert.
    bool txOngoing = platform_getPttStatus();
    if ((!state.emergency) && (!txOngoing) && (state.charge <= 0))
    {
        state.ui_screen = LOW_BAT;
        if(event.type == EVENT_KBD && event.payload)
        {
            state.ui_screen = MAIN_VFO;
            state.emergency = true;
        }
        return;
    }

    // Process pressed keys
    if(event.type == EVENT_KBD)
    {
        kbd_msg_t msg;
        msg.value = event.payload;
        // If MONI is pressed, activate MACRO functions
        if(msg.keys & KEY_MONI)
        {
            macro_menu = true;
            _ui_fsm_menuMacro(msg, sync_rtx);
            return;
        }
        else
        {
            macro_menu = false;
        }
        switch(state.ui_screen)
        {
            // VFO screen
            case MAIN_VFO:
                if(msg.keys & KEY_UP)
                {
                    // Increment TX and RX frequency of 12.5KHz
                    if(_ui_freq_check_limits(state.channel.rx_frequency + 12500) &&
                       _ui_freq_check_limits(state.channel.tx_frequency + 12500))
                    {
                        state.channel.rx_frequency += 12500;
                        state.channel.tx_frequency += 12500;
                        *sync_rtx = true;
                    }
                }
                else if(msg.keys & KEY_DOWN)
                {
                    // Decrement TX and RX frequency of 12.5KHz
                    if(_ui_freq_check_limits(state.channel.rx_frequency - 12500) &&
                       _ui_freq_check_limits(state.channel.tx_frequency - 12500))
                    {
                        state.channel.rx_frequency -= 12500;
                        state.channel.tx_frequency -= 12500;
                        *sync_rtx = true;
                    }
                }
                else if(msg.keys & KEY_ENTER)
                {
                    // Save current main state
                    ui_state.last_main_state = state.ui_screen;
                    // Open Menu
                    state.ui_screen = MENU_TOP;
                }
                else if(msg.keys & KEY_ESC)
                {
                    // Save VFO channel
                    state.vfo_channel = state.channel;
                    int result = _ui_fsm_loadChannel(state.channel_index, sync_rtx);
                    // Read successful and channel is valid
                    if(result != -1)
                    {
                        // Switch to MEM screen
                        state.ui_screen = MAIN_MEM;
                    }
                }
                else if(input_isNumberPressed(msg))
                {
                    // Open Frequency input screen
                    state.ui_screen = MAIN_VFO_INPUT;
                    // Reset input position and selection
                    ui_state.input_position = 1;
                    ui_state.input_set = SET_RX;
                    ui_state.new_rx_frequency = 0;
                    ui_state.new_tx_frequency = 0;
                    // Save pressed number to calculare frequency and show in GUI
                    ui_state.input_number = input_getPressedNumber(msg);
                    // Calculate portion of the new frequency
                    ui_state.new_rx_frequency = _ui_freq_add_digit(ui_state.new_rx_frequency,
                                            ui_state.input_position, ui_state.input_number);
                }
                break;
            // VFO frequency input screen
            case MAIN_VFO_INPUT:
                if(msg.keys & KEY_ENTER)
                {
                    _ui_fsm_confirmVFOInput(sync_rtx);
                }
                else if(msg.keys & KEY_ESC)
                {
                    // Cancel frequency input, return to VFO mode
                    state.ui_screen = MAIN_VFO;
                }
                else if(msg.keys & KEY_UP || msg.keys & KEY_DOWN)
                {
                    if(ui_state.input_set == SET_RX)
                        ui_state.input_set = SET_TX;
                    else if(ui_state.input_set == SET_TX)
                        ui_state.input_set = SET_RX;
                    // Reset input position
                    ui_state.input_position = 0;
                }
                else if(input_isNumberPressed(msg))
                {
                    _ui_fsm_insertVFONumber(msg, sync_rtx);
                }
                break;
            // MEM screen
            case MAIN_MEM:
                if(msg.keys & KEY_ENTER)
                {
                    // Save current main state
                    ui_state.last_main_state = state.ui_screen;
                    // Open Menu
                    state.ui_screen = MENU_TOP;
                }
                else if(msg.keys & KEY_ESC)
                {
                    // Restore VFO channel
                    state.channel = state.vfo_channel;
                    // Update RTX configuration
                    *sync_rtx = true;
                    // Switch to VFO screen
                    state.ui_screen = MAIN_VFO;
                }
                else if(msg.keys & KEY_UP)
                {
                    _ui_fsm_loadChannel(state.channel_index + 1, sync_rtx);
                }
                else if(msg.keys & KEY_DOWN)
                {
                    _ui_fsm_loadChannel(state.channel_index - 1, sync_rtx);
                }
                break;
            // Top menu screen
            case MENU_TOP:
                if(msg.keys & KEY_UP)
                    _ui_menuUp(menu_num);
                else if(msg.keys & KEY_DOWN)
                    _ui_menuDown(menu_num);
                else if(msg.keys & KEY_ENTER)
                {
                    switch(ui_state.menu_selected)
                    {
                        case M_ZONE:
                            state.ui_screen = MENU_ZONE;
                            break;
                        case M_CHANNEL:
                            state.ui_screen = MENU_CHANNEL;
                            break;
                        case M_CONTACTS:
                            state.ui_screen = MENU_CONTACTS;
                            break;
#ifdef HAS_GPS
                        case M_GPS:
                            state.ui_screen = MENU_GPS;
                            break;
#endif
                        case M_SETTINGS:
                            state.ui_screen = MENU_SETTINGS;
                            break;
                        case M_INFO:
                            state.ui_screen = MENU_INFO;
                            break;
                        case M_ABOUT:
                            state.ui_screen = MENU_ABOUT;
                            break;
                    }
                    // Reset menu selection
                    ui_state.menu_selected = 0;
                }
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(ui_state.last_main_state);
                break;
            // Zone menu screen
            case MENU_ZONE:
            // Channel menu screen
            case MENU_CHANNEL:
            // Contacts menu screen
            case MENU_CONTACTS:
                if(msg.keys & KEY_UP)
                    // Using 1 as parameter disables menu wrap around
                    _ui_menuUp(1);
                else if(msg.keys & KEY_DOWN)
                {
                    if(state.ui_screen == MENU_ZONE)
                    {
                        zone_t zone;
                        // menu_selected is 0-based while channels are 1-based
                        // menu_selected == 0 corresponds to "All Channels" zone
                        if(nvm_readZoneData(&zone, ui_state.menu_selected + 1) != -1)
                            ui_state.menu_selected += 1;
                    }
                    else if(state.ui_screen == MENU_CHANNEL)
                    {
                        channel_t channel;
                        // menu_selected is 0-based while channels are 1-based
                        if(nvm_readChannelData(&channel, ui_state.menu_selected + 2) != -1)
                            ui_state.menu_selected += 1;
                    }
                    else if(state.ui_screen == MENU_CONTACTS)
                    {
                        contact_t contact;
                        // menu_selected is 0-based while channels are 1-based
                        if(nvm_readContactData(&contact, ui_state.menu_selected + 2) != -1)
                            ui_state.menu_selected += 1;
                    }
                }
                else if(msg.keys & KEY_ENTER)
                {
                    if(state.ui_screen == MENU_ZONE)
                    {
                        zone_t newzone;
                        int result = 0;
                        // If "All channels" is selected, load default zone
                        if(ui_state.menu_selected == 0)
                            state.zone_enabled = false;
                        else
                        {
                            state.zone_enabled = true;
                            result = nvm_readZoneData(&newzone, ui_state.menu_selected);
                        }
                        if(result != -1)
                        {
                            state.zone = newzone;
                            // If we were in VFO mode, save VFO channel
                            if(ui_state.last_main_state == MAIN_VFO)
                                state.vfo_channel = state.channel;
                            // Load zone first channel
                            _ui_fsm_loadChannel(1, sync_rtx);
                            // Switch to MEM screen
                            state.ui_screen = MAIN_MEM;
                        }
                    }
                    if(state.ui_screen == MENU_CHANNEL)
                    {
                        // If we were in VFO mode, save VFO channel
                        if(ui_state.last_main_state == MAIN_VFO)
                            state.vfo_channel = state.channel;
                        _ui_fsm_loadChannel(ui_state.menu_selected + 1, sync_rtx);
                        // Switch to MEM screen
                        state.ui_screen = MAIN_MEM;
                    }
                }
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_TOP);
                break;
#ifdef HAS_GPS
            // GPS menu screen
            case MENU_GPS:
                if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_TOP);
                break;
#endif
            // Settings menu screen
            case MENU_SETTINGS:
                if(msg.keys & KEY_UP)
                    _ui_menuUp(settings_num);
                else if(msg.keys & KEY_DOWN)
                    _ui_menuDown(settings_num);
                else if(msg.keys & KEY_ENTER)
                {

                    switch(ui_state.menu_selected)
                    {
                        case S_DISPLAY:
                            state.ui_screen = SETTINGS_DISPLAY;
                            break;
#ifdef HAS_RTC
                        case S_TIMEDATE:
                            state.ui_screen = SETTINGS_TIMEDATE;
                            break;
#endif
#ifdef HAS_GPS
                        case S_GPS:
                            state.ui_screen = SETTINGS_GPS;
                            break;
#endif
                        default:
                            state.ui_screen = MENU_SETTINGS;
                    }
                    // Reset menu selection
                    ui_state.menu_selected = 0;
                }
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_TOP);
                break;
            // Info menu screen
            case MENU_INFO:
                if(msg.keys & KEY_UP)
                    _ui_menuUp(info_num);
                else if(msg.keys & KEY_DOWN)
                    _ui_menuDown(info_num);
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_TOP);
                break;
            // About screen
            case MENU_ABOUT:
                if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_TOP);
                break;
#ifdef HAS_RTC
            // Time&Date settings screen
            case SETTINGS_TIMEDATE:
                if(msg.keys & KEY_ENTER)
                {
                    // Switch to set Time&Date mode
                    state.ui_screen = SETTINGS_TIMEDATE_SET;
                    // Reset input position and selection
                    ui_state.input_position = 0;
                    memset(&ui_state.new_timedate, 0, sizeof(curTime_t));
                }
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_SETTINGS);
                break;
            // Time&Date settings screen, edit mode
            case SETTINGS_TIMEDATE_SET:
                if(msg.keys & KEY_ENTER)
                {
                    // Save time only if all digits have been inserted
                    if(ui_state.input_position < TIMEDATE_DIGITS)
                        break;
                    // Return to Time&Date menu, saving values
                    // NOTE: The user inserted a local time, we must save an UTC time
                    curTime_t utc_time = state_getUTCTime(ui_state.new_timedate);
                    rtc_setTime(utc_time);
                    state.time = utc_time;
                    state.ui_screen = SETTINGS_TIMEDATE;
                }
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(SETTINGS_TIMEDATE);
                else if(input_isNumberPressed(msg))
                {
                    // Discard excess digits
                    if(ui_state.input_position > TIMEDATE_DIGITS)
                        break;
                    ui_state.input_position += 1;
                    ui_state.input_number = input_getPressedNumber(msg);
                    _ui_timedate_add_digit(&ui_state.new_timedate, ui_state.input_position,
                                            ui_state.input_number);
                }
                break;
#endif
            case SETTINGS_DISPLAY:
                if(msg.keys & KEY_LEFT || (msg.keys & KEY_DOWN && ui_state.edit_mode))
                {
                    switch(ui_state.menu_selected)
                    {
                        case D_BRIGHTNESS:
                            _ui_changeBrightness(-25);
                            break;
#ifdef SCREEN_CONTRAST
                        case D_CONTRAST:
                            _ui_changeContrast(-4);
                            break;
#endif
                        default:
                            state.ui_screen = SETTINGS_DISPLAY;
                    }
                }
                else if(msg.keys & KEY_RIGHT || (msg.keys & KEY_UP && ui_state.edit_mode))
                {
                    switch(ui_state.menu_selected)
                    {
                        case D_BRIGHTNESS:
                            _ui_changeBrightness(+25);
                            break;
#ifdef SCREEN_CONTRAST
                        case D_CONTRAST:
                            _ui_changeContrast(+4);
                            break;
#endif
                        default:
                            state.ui_screen = SETTINGS_DISPLAY;
                    }
                }
                else if(msg.keys & KEY_UP)
                    _ui_menuUp(display_num);
                else if(msg.keys & KEY_DOWN)
                    _ui_menuDown(display_num);
                else if(msg.keys & KEY_ENTER)
                    ui_state.edit_mode = !ui_state.edit_mode;
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_SETTINGS);
                break;
#ifdef HAS_GPS
            case SETTINGS_GPS:
                if(msg.keys & KEY_LEFT || msg.keys & KEY_RIGHT ||
                   ((msg.keys & KEY_UP || msg.keys & KEY_DOWN) && ui_state.edit_mode))
                {
                    switch(ui_state.menu_selected)
                    {
                        case G_ENABLED:
                            // Disable or Enable GPS to stop or start GPS thread
                            if(state.settings.gps_enabled)
                            {
                                state.settings.gps_enabled = 0;
                                gps_disable();
                            }
                            else
                            {
                                state.settings.gps_enabled = 1;
                                gps_enable();
                            }
                            break;
                        case G_SET_TIME:
                            state.settings.gps_set_time = !state.settings.gps_set_time;
                            break;
                        case G_TIMEZONE:
                            if(msg.keys & KEY_LEFT || msg.keys & KEY_UP)
                                state.settings.utc_timezone -= 1;
                            else if(msg.keys & KEY_RIGHT || msg.keys & KEY_DOWN)
                                state.settings.utc_timezone += 1;
                            break;
                        default:
                            state.ui_screen = SETTINGS_GPS;
                    }
                }
                else if(msg.keys & KEY_UP)
                    _ui_menuUp(settings_gps_num);
                else if(msg.keys & KEY_DOWN)
                    _ui_menuDown(settings_gps_num);
                else if(msg.keys & KEY_ENTER)
                    ui_state.edit_mode = !ui_state.edit_mode;
                else if(msg.keys & KEY_ESC)
                    _ui_menuBack(MENU_SETTINGS);
                break;
#endif
        }
    }
}

void ui_updateGUI()
{
    if(!layout_ready)
    {
        layout = _ui_calculateLayout();
        layout_ready = true;
    }
    // Draw current GUI page
    switch(last_state.ui_screen)
    {
        // VFO main screen
        case MAIN_VFO:
            _ui_drawMainVFO();
            break;
        // VFO frequency input screen
        case MAIN_VFO_INPUT:
            _ui_drawMainVFOInput(&ui_state);
            break;
        // MEM main screen
        case MAIN_MEM:
            _ui_drawMainMEM();
            break;
        // Top menu screen
        case MENU_TOP:
            _ui_drawMenuTop(&ui_state);
            break;
        // Zone menu screen
        case MENU_ZONE:
            _ui_drawMenuZone(&ui_state);
            break;
        // Channel menu screen
        case MENU_CHANNEL:
            _ui_drawMenuChannel(&ui_state);
            break;
        // Contacts menu screen
        case MENU_CONTACTS:
            _ui_drawMenuContacts(&ui_state);
            break;
#ifdef HAS_GPS
        // GPS menu screen
        case MENU_GPS:
            _ui_drawMenuGPS();
            break;
#endif
        // Settings menu screen
        case MENU_SETTINGS:
            _ui_drawMenuSettings(&ui_state);
            break;
        // Info menu screen
        case MENU_INFO:
            _ui_drawMenuInfo(&ui_state);
            break;
        // About menu screen
        case MENU_ABOUT:
            _ui_drawMenuAbout();
            break;
#ifdef HAS_RTC
        // Time&Date settings screen
        case SETTINGS_TIMEDATE:
            _ui_drawSettingsTimeDate();
            break;
        // Time&Date settings screen, edit mode
        case SETTINGS_TIMEDATE_SET:
            _ui_drawSettingsTimeDateSet(&ui_state);
            break;
#endif
        // Display settings screen
        case SETTINGS_DISPLAY:
            _ui_drawSettingsDisplay(&ui_state);
            break;
#ifdef HAS_GPS
        // GPS settings screen
        case SETTINGS_GPS:
            _ui_drawSettingsGPS(&ui_state);
            break;
#endif
        // Low battery screen
        case LOW_BAT:
            _ui_drawLowBatteryScreen();
            break;
    }
    // If MACRO menu is active draw it
    if(macro_menu) {
        _ui_drawDarkOverlay();
        _ui_drawMacroMenu(&last_state);
    }
}

void ui_terminate()
{
}
