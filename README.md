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

For a detailed writeup on the features and inner workings of Machium, [read the blog post about the project.](https://psychobird.github.io/Machium/Machium.html)

## Machium Commands

- write [0xADDRESS] [0xDATA] - write [0xDATA] to memory [0xADDRESS]
- read
    - bytes [0xADDRESS] [size] - reads [size] amount of memory at [0xADDRESS]
    - value [0xADDRESS] [size] - reads [size <= 8] value at [0xADDRESS]
    - lines
        - char [0xADDRESS] [lines] - reads [lines] amount of lines of memory as ASCII at [0xADDRESS]
        - bytes [0xADDRESS] [lines] - reads [lines] amount of lines of memory as bytes at [0xADDRESS]
- register
    - read - read all register values
    - write [register] [0xDATA] - write [0xDATA] to [register]
- breakpoint
    - set [0xADDRESS] - set a breakpoint at memory [0xADDRESS]
    - remove - remove a breakpoint
- watchpoint
    - set [0xADDRESS] - set a watchpoint at memory [0xADDRESS]
    - remove - remove a watchpoint
- pause - pauses the debugger
- continue - resumes execution of task
- pid - get current pid of debugged process
    - [pid] - change current debug process to new process, [pid]

## Machium In Action

![]("https://psychobird.github.io/Machium/Images/image1.png")
![]("https://psychobird.github.io/Machium/Images/image2.png")
![]("https://psychobird.github.io/Machium/Images/image3.png")
