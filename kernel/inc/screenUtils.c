//include the code from the respective header file
#include "screenUtils.h"

//Initialize the variables created in screen.h
bool writing = 0, progexit = 0, layout = 0, ctrl = 0, typingCmd = 0;
uint8 startCmdY = 0, startCmdX = 0;
bool newCmd = 0;

//Setup variables needed for this file
int cursorX = 0, cursorY = 0;
const uint8 sw = 80,sh = 26,sd = 2;

void clearLine(uint8 from, uint8 to)
{
    string vidmem=(string)0xb8000;
    for(uint16 i = sw * from * sd; i < (sw * to * sd); i++)
    {
        vidmem[i] = 0x0;
    }
}

void updateCursor()
{
    unsigned temp;

    temp = cursorY * sw + cursorX;                                              // Position = (y * width) +  x

    outportb(0x3D4, 14);                                                        // CRT Control Register to Select Cursor Location
    outportb(0x3D5, temp >> 8);                                                 // ASM to send the high byte across the bus
    outportb(0x3D4, 15);                                                        // Another CRT Control Register to Select Send Low byte
    outportb(0x3D5, temp);                                                      // Use ASM outportb function again to send the Low byte of the cursor location
}

void clearScreen()
{
    clearLine(0, sh - 1);
    cursorX = 0;
    cursorY = 0;
    updateCursor();
}

void scrollUp(uint8 lineNumber)
{
    string vidmem = (string) 0xb8000;
    clearLine(0, lineNumber - 1);
    for (uint16 i = 0; i<sw * (sh - 1) * 2; i++)
    {
        vidmem[i] = vidmem[i+sw*2*lineNumber];
    }
    clearLine(sh-1-lineNumber,sh-1);
    if((cursorY - lineNumber) < 0 ) 
    {
        cursorY = 0;
        cursorX = 0;
    } 
    else 
    {
        cursorY -= lineNumber;
    }
    updateCursor();
}


void newLineCheck()
{
    if(cursorY >= sh - 1)
    {
        scrollUp(1);
    }
}

void printch(char c, int b)
{
    string vidmem = (string) 0xb8000;     
    switch(c)
    {
    case (0x08): // Backspace
        if(cursorX > 0) 
        {
            cursorX--;									
            vidmem[(cursorY * sw + cursorX)*sd] = 0x00;
        }
        break;
    case ('\t'): {
        int modX = cursorX % 4; // Tabs are 4 spaces wide
        modX = modX == 0 ? 4 : modX;
        while(modX--) {
            printch(' ', b);
        }
        break;
    }
    case ('\r'):
        cursorX = 0;
        break;
    case ('\n'):
        cursorX = 0;
        cursorY++;
        break;
    default:
        vidmem [((cursorY * sw + cursorX))*sd] = c;
        vidmem [((cursorY * sw + cursorX))*sd+1] = b;
        cursorX++;
        break;
    }
    if(cursorX >= sw)
    {
        cursorX = 0;
        cursorY++;
    }
    updateCursor();
    newLineCheck();
}

void print(string ch, int bh)
{
    uint8 length = strlength(ch);
    for(uint16 i = 0; i < length; i++)
    {
        printch(ch[i], bh);
    }
}

void printint(uint32 n, int bh) {
    if (n == 0)
    {
        printch('0', bh);
        return;
    }

    int32 acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc % 10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
    print(c2, bh);
}

void printhex(uint32 n, int bh)
{
    int32 tmp;
    print("0x", bh);
    char noZeroes = 1;
    int i;
    for (i = 28; i > 0; i -= 4)
    {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0)
        {
            continue;
        }
    
        if (tmp >= 0xA)
        {
            noZeroes = 0;
            printch(tmp-0xA+'a', bh);
        }
        else
        {
            noZeroes = 0;
            printch(tmp+'0', bh);
        }
    }
  
    tmp = n & 0xF;
    if (tmp >= 0xA)
    {
        printch(tmp-0xA+'a', bh);
    }
    else
    {
        printch(tmp+'0', bh);
    }
}

void moveCursorX(int x) {
    cursorX += x;
    while(cursorX < 0) {
        cursorX += sw;
        cursorY -= 1;
    }
    while(cursorX >= sw) {
        cursorX -= sw;
        cursorY += 1;
    }
    updateCursor();
    newLineCheck();
}

void moveCursorY(int y) {
    cursorY += y;
    updateCursor();
    newLineCheck();
}
