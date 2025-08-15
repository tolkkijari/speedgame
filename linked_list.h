#include <avr/io.h>

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

void add_new(uint8_t value);
void delete_start();
void delete_all();
uint8_t get_start();
int get_size();
#endif