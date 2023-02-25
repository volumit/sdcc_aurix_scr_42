/* ARGS:
  A - A reg
  B - bit
  C - AB reg
  D - data
  N - none
  R - R0..R7
  r - C reg
  P - DPTR
  @ - @A+DPTR
  X - @A+PC
  I - @R0, @R1
  J - jump rel
  / - /C
  T - @DPTR
  # - #data
  d - data16
  1 - jump addr 11
  6 - jump addr 16
*/

/* MRELOC
  N - none                - IIIIIIII
  I - register indirect   - IIIIIIIr
  i - reg. indirect, data - IIIIIIIr dddddddd
  R - register            - IIIIIrrr
  r - register,data       - IIIIIrrr dddddddd
  7 - relative jump 8 bit - IIIIIIII aaaaaaaa
  1 - jump 11 bit         - aaaIIIII aaaaaaaa
  6 - data/jump 16 bit    - IIIIIIII aaaaaaaa aaaaaaaa
  D - data 8 bit          - IIIIIIII dddddddd
  d - data,data 8 bit     - IIIIIIII dddddddd dddddddd
  a - addr,data 8 bit     - IIIIIIII aaaaaaaa dddddddd
  B - bitdata 8 bit       - IIIIIIII bbbbbbbb
  J - bit, r. jump 8 bit  - IIIIIIII bbbbbbbb aaaaaaaa
  X - reg. indirect, r. jump 8 bit - IIIIIIIr dddddddd aaaaaaaa
  Y - register, rel. jump 8 bit    - IIIIIrrr dddddddd aaaaaaaa
  Z - data 8 bit, r. jump 8 bit    - IIIIIIII dddddddd aaaaaaaa
  W - register, rel. jump 8 bit    - IIIIIrrr aaaaaaaa
*/
#define I51_MCS51 0x00000001
#define I51_SCR   0x00000002


I51_INS ("acall", "1NN", 0x82, "aaa10001", '1', 0x11, 0x1F, I51_MCS51 | I51_SCR)
I51_INS ("add",   "ARN", 0x01, "00101rrr", 'R', 0x28, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("add",   "AIN", 0x01, "0010011r", 'I', 0x26, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("add",   "ADN", 0x02, "00100101", 'D', 0x25, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("add",   "A#N", 0x82, "00100100", 'D', 0x24, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("addc",  "ARN", 0x01, "00111rrr", 'R', 0x38, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("addc",  "AIN", 0x01, "0011011r", 'I', 0x36, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("addc",  "ADN", 0x02, "00110101", 'D', 0x35, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("addc",  "A#N", 0x82, "00110100", 'D', 0x34, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("ajmp",  "1NN", 0x82, "aaa00001", '1', 0x01, 0x1F, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "ARN", 0x01, "01011rrr", 'R', 0x58, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "AIN", 0x01, "0101011i", 'I', 0x56, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "ADN", 0x02, "01010101", 'D', 0x55, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "A#N", 0x02, "01010100", 'D', 0x54, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "rBN", 0x02, "10000010", 'B', 0x82, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "r/N", 0x02, "10110000", 'B', 0xB0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "DAN", 0x02, "01010010", 'D', 0x52, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("anl",   "D#N", 0x83, "01010011", 'a', 0x53, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("cjne",  "A#J", 0x03, "10110100", 'Z', 0xB4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("cjne",  "R#J", 0x03, "10111rrr", 'Y', 0xB8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("cjne",  "I#J", 0x03, "1011011r", 'X', 0xB6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("cjne",  "ADJ", 0x83, "10110101", 'Z', 0xB5, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("clr",   "ANN", 0x01, "11100100", 'N', 0xE4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("clr",   "rNN", 0x01, "11000011", 'N', 0xC3, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("clr",   "BNN", 0x82, "11000010", 'B', 0xC2, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("cpl",   "ANN", 0x01, "11110100", 'N', 0xF4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("cpl",   "rNN", 0x01, "10110011", 'N', 0xB3, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("cpl",   "BNN", 0x82, "10110010", 'B', 0xB2, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("da",    "ANN", 0x81, "11010100", 'N', 0xD4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("dec",   "ANN", 0x01, "00010100", 'N', 0x14, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("dec",   "RNN", 0x01, "00011rrr", 'R', 0x18, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("dec",   "INN", 0x01, "0001011r", 'I', 0x16, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("dec",   "DNN", 0x82, "00010101", 'D', 0x15, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("div",   "CNN", 0x81, "10000100", 'N', 0x84, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("djnz",  "RJN", 0x02, "11011rrr", 'W', 0xD8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("djnz",  "DJN", 0x83, "11010101", 'Z', 0xD5, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("inc",   "ANN", 0x01, "00000100", 'N', 0x04, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("inc",   "RNN", 0x01, "00001rrr", 'R', 0x08, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("inc",   "INN", 0x01, "0000011r", 'I', 0x06, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("inc",   "PNN", 0x01, "10100011", 'N', 0xA3, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("inc",   "DNN", 0x82, "00000101", 'D', 0x05, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jb",    "BJN", 0x83, "00100000", 'J', 0x20, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jbc",   "BJN", 0x83, "00010000", 'J', 0x10, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jc",    "JNN", 0x82, "01000000", '7', 0x40, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jmp",   "@NN", 0x81, "01110011", 'N', 0x73, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jnb",   "BJN", 0x83, "00110000", 'J', 0x30, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jnc",   "JNN", 0x82, "01010000", '7', 0x50, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jnz",   "JNN", 0x82, "01110000", '7', 0x70, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("jz",    "JNN", 0x82, "01100000", '7', 0x60, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("lcall", "6NN", 0x83, "00010010", '6', 0x12, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("ljmp",  "6NN", 0x83, "00000010", '6', 0x02, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "ARN", 0x01, "11101rrr", 'R', 0xE8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "AIN", 0x01, "1110011r", 'I', 0xE6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "ADN", 0x02, "11100101", 'D', 0xE5, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "A#N", 0x02, "01110100", 'D', 0x74, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "RAN", 0x01, "11111rrr", 'R', 0xF8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "RDN", 0x02, "10101rrr", 'r', 0xA8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "R#N", 0x02, "01111rrr", 'r', 0x78, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "IAN", 0x01, "1111011r", 'I', 0xF6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "IDN", 0x02, "1010011r", 'r', 0xA6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "I#N", 0x02, "0111011r", 'r', 0x76, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "rBN", 0x02, "10100010", 'B', 0xA2, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "P#N", 0x03, "10010000", '6', 0x90, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "BrN", 0x02, "10010010", 'B', 0x92, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "DAN", 0x02, "11110101", 'D', 0xF5, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "DRN", 0x02, "10001rrr", 'r', 0x88, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "DIN", 0x02, "1000011r", 'r', 0x86, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "DDN", 0x03, "10000101", 'd', 0x85, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mov",   "D#N", 0x83, "01110101", 'a', 0x75, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("movc",  "A@N", 0x01, "10010011", 'N', 0x93, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("movc",  "&AN", 0x01, "10100101", 'N', 0xA5, 0xFF, I51_SCR)
I51_INS ("movc",  "AXN", 0x81, "10000011", 'N', 0x83, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("movx",  "AIN", 0x01, "1110001r", 'I', 0xE2, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("movx",  "ATN", 0x01, "11100000", 'N', 0xE0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("movx",  "IAN", 0x01, "1111001r", 'I', 0xF2, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("movx",  "TAN", 0x81, "11110000", 'N', 0xF0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("mul",   "CNN", 0x81, "10100100", 'N', 0xA4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("nop",   "NNN", 0x81, "00000000", 'N', 0x00, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "ARN", 0x01, "01001rrr", 'R', 0x48, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "AIN", 0x01, "0100011i", 'I', 0x46, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "ADN", 0x02, "01000101", 'D', 0x45, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "A#N", 0x02, "01000100", 'D', 0x44, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "rBN", 0x02, "01110010", 'B', 0x72, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "r/N", 0x02, "10100000", 'B', 0xA0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "DAN", 0x02, "01000010", 'D', 0x42, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("orl",   "D#N", 0x83, "01000011", 'a', 0x43, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("pop",   "DNN", 0x82, "11010000", 'D', 0xD0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("push",  "DNN", 0x82, "11000000", 'D', 0xC0, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("ret",   "NNN", 0x81, "00100010", 'N', 0x22, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("reti",  "NNN", 0x81, "00110010", 'N', 0x32, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("rl",    "ANN", 0x81, "00100011", 'N', 0x23, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("rlc",   "ANN", 0x81, "00110011", 'N', 0x33, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("rr",    "ANN", 0x81, "00000011", 'N', 0x03, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("rrc",   "ANN", 0x81, "00010011", 'N', 0x13, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("setb",  "rNN", 0x01, "11010011", 'N', 0xD3, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("setb",  "BNN", 0x82, "11010010", 'B', 0xD2, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("sjmp",  "JNN", 0x82, "10000000", '7', 0x80, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("subb",  "ARN", 0x01, "10011rrr", 'R', 0x98, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("subb",  "AIN", 0x01, "1001011r", 'I', 0x96, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("subb",  "ADN", 0x02, "10010101", 'D', 0x95, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("subb",  "A#N", 0x82, "10010100", 'D', 0x94, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("swap",  "ANN", 0x81, "11000100", 'N', 0xC4, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("xch",   "ARN", 0x01, "11001rrr", 'R', 0xC8, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("xch",   "AIN", 0x01, "1100011r", 'I', 0xC6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("xch",   "ADN", 0x82, "11000101", 'D', 0xC5, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("xchd",  "AIN", 0x81, "1101011r", 'I', 0xD6, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "ARN", 0x01, "01101rrr", 'R', 0x68, 0xF8, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "AIN", 0x01, "0110011i", 'I', 0x66, 0xFE, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "ADN", 0x02, "01100101", 'D', 0x65, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "A#N", 0x02, "01100100", 'D', 0x64, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "DAN", 0x02, "01100010", 'D', 0x62, 0xFF, I51_MCS51 | I51_SCR)
I51_INS ("xrl",   "D#N", 0x83, "01100011", 'a', 0x63, 0xFF, I51_MCS51 | I51_SCR)

