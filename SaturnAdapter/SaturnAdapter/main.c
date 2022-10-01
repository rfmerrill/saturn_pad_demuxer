/*
 * SaturnAdapter.c
 *
 * Created: 9/30/2022 8:21:47 PM
 * Author : rfm
 */

#include <avr/io.h>

// CKSEL fuse 0100 for internal 8MHz osc
// program RSTDISBL fuse

// Port A
// 0 - X output
// 1 - Y output
// 2 - C output

// Port B
// 0 - right dpad out
// 1 - up dpad out
// 2 - saturn D2 in
// 3 - saturn D3 in
// 4 - saturn S1 out
// 5 - saturn S0 out
// 6 - saturn D0 in
// 7 - saturn D1 in

// Port D
// 0 - B out
// 1 - Z out
// 2 - A out
// 3 - left dpad out
// 4 - MODE out
// 5 - START out
// 6 - down dpad out

#define D0_BIT _BV(6)
#define D1_BIT _BV(7)
#define D2_BIT _BV(2)
#define D3_BIT _BV(3)

static inline void waitForTimer(void)
{
    // Wait for compare interrupt and then clear it
    while (0 == (TIFR & _BV(OCF0A)))
    {
        // pass
    }

    TIFR = _BV(OCF0A);
}

int main(void)
{
    // Turn prescaler to /1
    CLKPR = _BV(CLKPCE);  // unlock
    CLKPR = 0x00;         // set to divide by 1.

    OCR0A = 63;           // 1 MHz / 63 = 15.87kHz
    TCCR0A = _BV(WGM01);  // CTC mode
    TCCR0B = _BV(CS01);   // Clock / 8

    DDRA = 0x07;  // PA0-PA2 output
    DDRB = 0x33;  // PB0-1,4-5 output
    DDRD = 0x7F;  // PD0-6 output

    uint8_t reads[3];
    waitForTimer();
    while (1)
    {
        // 00 - ZYXR
        PORTB &= ~(_BV(PB4) | _BV(PB5));
        waitForTimer();
        reads[0] = PINB;

        // 01 - UDLR
        PORTB |= _BV(PB4);
        waitForTimer();
        reads[1] = PINB;

        // For d-pad, process up/down and left/right
        // atomically so we don't get caught in accidental left+right or
        // up+down as that may crash some games?

        // Left/right
        switch ((reads[1] >> 2) & 0x3)
        {
            case 0:
            {
                // both down
                PORTB &= ~_BV(0);
                PORTD &= ~_BV(3);
            }
            break;
            case 1:
            {
                // right is down, left is up
                PORTD |= _BV(3);
                PORTB &= ~_BV(0);
            }
            break;
            case 2:
            {
                // left is down, right is up
                PORTB |= _BV(0);
                PORTD &= ~_BV(3);
            }
            break;
            default:
            {
                // both are up
                PORTD |= _BV(3);
                PORTB |= _BV(0);
            }
            break;
        }

        // Up/down
        switch ((reads[1] >> 6) & 0x3)
        {
            case 0:
            {
                // both down
                PORTB &= ~_BV(1);
                PORTD &= ~_BV(6);
            }
            break;
            case 1:
            {
                // down is down, up is up
                PORTB |= _BV(1);
                PORTD &= ~_BV(6);
            }
            break;
            case 2:
            {
                // up is down, down is up
                PORTD |= _BV(6);
                PORTB &= ~_BV(1);
            }
            break;
            default:
            {
                // both are up
                PORTD |= _BV(6);
                PORTB |= _BV(1);
            }
            break;
        }

        // 10 - BCASt
        PORTB &= ~_BV(PB4);
        PORTB |= _BV(PB5);
        waitForTimer();
        reads[2] = PINB;

        // Port A
        // 0 - X output
        // 1 - Y output
        // 2 - C output

        if (reads[0] & D2_BIT)
            PORTA |= _BV(0);
        else
            PORTA &= ~_BV(0);

        if (reads[0] & D1_BIT)
            PORTA |= _BV(1);
        else
            PORTA &= ~_BV(1);

        if (reads[2] & D1_BIT)
            PORTA |= _BV(2);
        else
            PORTA &= ~_BV(2);

        // Port D
        // 0 - B out
        // 1 - Z out
        // 2 - A out
        // 4 - MODE out
        // 5 - START out
        if (reads[2] & D0_BIT)
            PORTD |= _BV(0);
        else
            PORTD &= ~_BV(0);

        if (reads[0] & D0_BIT)
            PORTD |= _BV(1);
        else
            PORTD &= ~_BV(1);

        if (reads[2] & D2_BIT)
            PORTD |= _BV(2);
        else
            PORTD &= ~_BV(2);

        if (reads[0] & D3_BIT)
            PORTD |= _BV(4);
        else
            PORTD &= ~_BV(4);

        if (reads[2] & D3_BIT)
            PORTD |= _BV(5);
        else
            PORTD &= ~_BV(5);

        waitForTimer();
    }
    return 0;
}
