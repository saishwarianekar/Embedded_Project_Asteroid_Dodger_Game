#include <lpc213x.h>
#include <stdio.h>
#include <stdlib.h> // For rand() function

#define def_pos 8
#define MATRIX_SIZE 16 // Assuming a 16x2 LCD (modify if different)

void delay(unsigned int t)
{
    unsigned int i, j;
    for(j = 0; j < t; j++) {
        for(i = 0; i < 500; i++);
    }
}

void cmd(unsigned char a)
{
    IOCLR0 = (2047 << 13);
    IOSET0 = (a << 16);
    IOCLR0 = 1 << 13;
    IOCLR0 = 1 << 14;
    IOSET0 = 1 << 15;
    delay(1);
    IOCLR0 = 1 << 15;
}

void lcd_data(unsigned char b)
{
    IOCLR0 = (2047 << 13);
    IOSET0 = (b << 16);
    IOSET0 = 1 << 13;
    IOCLR0 = 1 << 14;
    IOSET0 = 1 << 15;
    delay(1);
    IOCLR0 = 1 << 15;
}

void lcd_init()
{
    cmd(0x38);
    cmd(0x0e);
    cmd(0x01);
    cmd(0x06);
    cmd(0x0c);
    cmd(0x80);
}

void display(char *s)
{
    while (*s) {
        lcd_data(*s++);
    }
}

unsigned int i;

int main() {
    int b_pos, asteroid_pos[3] = {0, 0, 0}, asteroid_row[3] = {0, 0, 0};
    int score, level, num_asteroids;
    char score_display[16];
    int a; // Loop variable declared here

    IODIR0 = (2047 << 13);
    lcd_init();

    while (1) {
        // Initialize game variables
        b_pos = 5;
        asteroid_pos[0] = rand() % MATRIX_SIZE;
        asteroid_row[0] = 0;
        score = 0;
        level = 1;
        num_asteroids = 1;

        while (1) {
            if (IOPIN1 & (1 << 28)) {
                if ((IOPIN1 & (1 << 29)) && b_pos < 31) {
                    b_pos += 1;
                    while (IOPIN1 & (1 << 29));
                }
                if ((IOPIN1 & (1 << 30)) && b_pos > 0) {
                    b_pos -= 1;
                    while (IOPIN1 & (1 << 30));
                }

                cmd(0x01);  // Clear display

                // Display the player's position on the fourth row
                cmd(0xD0);  // Assume the fourth row is mapped here
                for (i = 0; i < b_pos; i++) {
                    display(" ");
                }
                display("A");

                // Display asteroids in their respective rows
                for (a = 0; a < num_asteroids; a++) {  // Loop using external variable
                    if (asteroid_row[a] == 0) {
                        cmd(0x80);  // First row
                    } else if (asteroid_row[a] == 1) {
                        cmd(0xC0);  // Second row
                    } else if (asteroid_row[a] == 2) {
                        cmd(0x90);  // Third row
                    } else {
                        cmd(0xD0);  // Fourth row
                    }

                    for (i = 0; i < asteroid_pos[a]; i++) {
                        display(" ");
                    }
                    display("o");
                }

                delay(300); // Delay for asteroid speed

                // Move asteroids down or reset them
                for (a = 0; a < num_asteroids; a++) {  // Loop using external variable
                    if (asteroid_row[a] == 3) {
                        asteroid_row[a] = 0;
                        asteroid_pos[a] = rand() % MATRIX_SIZE;
                        score++;
                    } else {
                        asteroid_row[a]++;
                    }

                    // Collision detection on the fourth row
                    if (asteroid_row[a] == 3 && b_pos == asteroid_pos[a]) {
                        cmd(0x01);  // Clear display
                        cmd(0x80);  // Move to first row
                        display("GAME OVER     ");
                        cmd(0xC0);  // Move to second row
                        sprintf(score_display, "SCORE: %d     ", score);
                        display(score_display);
                        delay(2000);

                        // Wait for user to press a button to restart
                        while (!(IOPIN1 & (1 << 28)));
                        while (IOPIN1 & (1 << 28));
                        break;
                    }
                }

                // Level progression
                if (score >= 50 && level < 3) {
                    level = 3;
                    num_asteroids = 3;
                    cmd(0x01);  // Clear display
                    cmd(0x80);
                    display("LEVEL 3       ");
                    delay(2000);
                } else if (score >= 25 && level < 2) {
                    level = 2;
                    num_asteroids = 2;
                    cmd(0x01);  // Clear display
                    cmd(0x80);
                    display("LEVEL 2       ");
                    delay(2000);
                }
            } else {
                // Display start message
                cmd(0x80);
                display("                ");
                display("                ");
                cmd(0xC0);
                display("START  THE  GAME");
                display("                ");
            }
        }
    }
}
