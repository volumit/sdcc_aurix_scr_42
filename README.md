# sdcc_aurix_scr_42
sdcc+binutils for SCR Aurix (mcs51)
Standby Controller SCR in AURIX Microcontroller Family TC3xx.

It is a tiny 8051/MCS51 which allows to do some tasks, with very low current consumption.

It contains an own subsystem some Timers, AD comperators, CAN , SPI, UARTS, 8Kbyte XRAM....

Target was here to support SCR C-Code generation, based on SDCC.

It is an experimental patch of SDCC , quick hack of mcs51 which allows fast progress.

It is based on SDCC mcs51 4.2.2 #13448.
With an updated release e.g. 4.3. (several activities are ongoing for bugfixes, feature enhancements, ...) I have in mind to make a more
cleaner release as an own architecture in SDCC, not as patch for mcs51.

SDCC is only used as compiler frontend to generate assembly.

Assembler, Link/Locate, .... is done by a modified version of binutils, which is now supporting SCR (MCS51).

Major hickup with the SCR is that register B is not bit addressable.

Register B 0xF0 in standard MCS51 architectures, 0xDA in Infineon SCR (Reg B is not bit addressable anymore).
With a lot of consequences, libs, compiler mid,back-end, ....

I recommend here Infineon to be a little bit more careful under the aspect of HW/SW/Tool- Codesign.
If you change RTL, please check consequences.

The simulator was adapted to reflect the architectural changes of SCR.

Testsuite results for mcs51-large, mcs51-large-stack-auto --nooverlay.

There is no blocking point to go for small and medium memory models, but maybe testsuite has to be adapted.

Summary for 'mcs51-large': 0 failures, 18471 tests, 3268 test cases, 7127687 bytes, 5297232060 ticks

Summary for 'mcs51-large-stack-auto': 0 failures, 18454 tests, 3267 test cases, 6607445 bytes, 3739580400 ticks

Quality status is reasonable, tried some examples on real target and it is working like expected.

../binutils is containg the binutils which supports mcs51,i51

../bin_mingw ready 2 use precompiled version for mingw

../bin_linux ready 2 use precompiled version linux
