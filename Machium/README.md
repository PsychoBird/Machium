# Machium
 A debugger for Apple Silicon

## Machium Features

Like a "standard debugger", Machium has the ability to:
- Read / Write Registers
- Read / Write Memory
- Pause Tasks
- Set Breakpoints / Watchpoints

Machium is much lighter than lldb, gdb, and other debuggers that run on iDevices.

Having a "lighter" debugger is useful for simple tasks where the extra baggage of a heavier debugger can cause the target application to crash upon detection or the delay between the debugger starting and becoming usable. 
