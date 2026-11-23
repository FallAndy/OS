#include <sbi.h>
#include <console.h>

/* kbd_intr - try to feed input characters from keyboard */
void kbd_intr(void) {}

/* serial_intr - try to feed input characters from serial port */
void serial_intr(void) {}

/* cons_init - initializes the console devices */
void cons_init(void) {}

/* cons_putc - print a single character @c to console devices */
void cons_putc(int c) { sbi_console_putchar((unsigned char)c); }

/* *
 * cons_getc - return the next input character from console,
 * or 0 if none waiting.
 * */
int cons_getc(void) {
    int c = 0;
    c = sbi_console_getchar();
    // The legacy SBI console returns -1 when no input is available.
    // Keep the console contract expected by getchar(): 0 means "no data".
    return (c < 0) ? 0 : c;
}
