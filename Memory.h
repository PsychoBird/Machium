#ifndef MEMORY_H
#define MEMORY_H

#include "Machium.h"

//get and change pid
machium_command_t m_pid(Machium* machium);

//read memory (vm_read_overwrite wrapper)
machium_command_t m_read(Machium* machium);
machium_command_t m_read_bytes(Machium* machium); //read bytes from memory
machium_command_t m_read_lines(Machium* machium); //read lines from memory
machium_command_t m_read_value(Machium* machium); //read value from memory

//write to memory (vm_write wrapper)
machium_command_t m_write(Machium* machium);


#endif /* MEMORY_H */
