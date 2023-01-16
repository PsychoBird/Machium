#ifndef REGISTER_H
#define REGISTER_H

#include "Machium.h"

//handler function for all subsequent register functions
machium_command_t m_register(Machium* machium);

//read from registers
machium_command_t m_register_read(Machium* machium);

//write to registers
machium_command_t m_register_write(Machium* machium);


#endif /* REGISTER_H */
