/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void keyboard_handler();
void clock_handler();
void system_call_handler();
void custom_page_fault_handler();

#endif  /* __ENTRY_H__ */
