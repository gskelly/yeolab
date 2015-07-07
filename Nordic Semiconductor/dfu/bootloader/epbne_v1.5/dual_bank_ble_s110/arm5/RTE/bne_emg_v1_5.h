/* Copyright (c) 2015 Graham Kelly
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#ifndef BNE_EMG_V1_5_H
#define BNE_EMG_V1_5_H

// Board has no LEDs or buttons
#define LEDS_NUMBER    0
#define BUTTONS_NUMBER 0

// SPI Slave definitions for SPI pseudo-master implementation (TODO)
/*
#define SPIS_MISO_PIN  18    // SPI MISO signal. 
#define SPIS_CSN_PIN   21    // SPI CSN signal. 
#define SPIS_MOSI_PIN  20    // SPI MOSI signal. 
#define SPIS_SCK_PIN   19    // SPI SCK signal.
*/

#define SPIM0_SCK_PIN      13     /**< SPI clock GPIO pin number - D13 on Arduino */
#define SPIM0_MOSI_PIN      14     /**< SPI Master Out Slave In GPIO pin number - D11 on Arduino */
#define SPIM0_MISO_PIN      12     /**< SPI Master In Slave Out GPIO pin number - D12 on Arduino */
#define SPIM0_SS_PIN        15     /**< SPI Slave Select GPIO pin number - D1 on Arduino */

#endif // BNE_EMG_V1_5_H
