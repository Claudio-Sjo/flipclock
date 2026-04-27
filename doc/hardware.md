# Hardware

## Components

| Component          | Description                                      |
|--------------------|--------------------------------------------------|
| Raspberry Pi Pico  | RP2040 microcontroller                           |
| Pico Display 2     | Pimoroni 320×240 LCD with 4 buttons (A, B, X, Y) |
| DS3231             | High-accuracy I2C real-time clock (address 0x68) |
| 24LC65             | I2C EEPROM (address 0x50, used for testing)      |
| Si5351             | Programmable clock generator (address 0x60, driver included but unused) |

## Pin Assignments

| GPIO | Function     | Notes                        |
|------|-------------|------------------------------|
| 26   | I2C1 SDA    | DS3231 + EEPROM data line    |
| 27   | I2C1 SCL    | DS3231 + EEPROM clock line   |

The Pico Display 2 uses SPI for the LCD and dedicated GPIOs for the A/B/X/Y buttons (managed by the Pimoroni library).

## I2C Bus

- **Bus**: I2C1
- **Speed**: 100 kHz
- **Devices**:
  - DS3231 RTC at 0x68
  - 24LC65 EEPROM at 0x50

The I2C communication uses the `MemI2C` library which provides blocking read/write with multi-byte address support, directly accessing the RP2040's I2C hardware registers.

## Display

- **Resolution**: 320 × 240 pixels
- **Framebuffer**: 16-bit RGB565, stored in `buffer[320*240]`
- **Screen layout**:
  - Top 2/3 (y 0–159): clock digits and background animation
  - Bottom 1/3 (y 160–239): date/status text and debug info
