#include <stdio.h>
#include <stdint.h>

const char* ESC = "\x1b[";

void EraseToEOL(void) {
  printf("%sK", ESC);
}

void EraseLine(uint8_t param_1) {
  printf("%s%dG", ESC, param_1 & 0xff);
  EraseToEOL();
}

void MoveCursor(uint8_t param_1, uint8_t param_2) {
  printf("%s%d;%dH", ESC, param_1 & 0xff, param_2 & 0xff);
}

void ClearScreen(void) {
  printf("%s%d%c", ESC, 2, 'J');
  MoveCursor(1,1);
}

void CursorRight(uint8_t param_1) {
  printf("%s%dC", ESC, param_1 & 0xff);
}

void CursorLeft(uint8_t param_1) {
  printf("%s%dD", ESC, param_1 & 0xff);
}

int main() {
    ClearScreen();
    printf("Hello from C code!\n");
    MoveCursor(5, 10);
    printf("Cursor moved to (5,10)\n");
    CursorRight(5);
    printf("Cursor moved right by 5\n");
    CursorLeft(2);
    printf("Cursor moved left by 2\n");
    MoveCursor(8, 1);
    printf("This line will be erased from column 10 onwards.");
    MoveCursor(8, 10);
    EraseToEOL();
    MoveCursor(9, 1);
    printf("This line will be partially erased from column 5 onwards.");
    EraseLine(5);
    MoveCursor(12, 1);
    printf("Done. Press Enter to clear screen and exit.");
    getchar();
    ClearScreen();
    return 0;
}