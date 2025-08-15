#include <stdlib.h>
#include <avr/io.h>
#include "linked_list.h"

typedef struct list {
	uint8_t value;
	struct list* next;
} list_t;

list_t* start = NULL;
list_t* end = NULL;
int size = 0;

list_t* create_new(uint8_t value) {
	list_t* item = (list_t*)malloc(sizeof(list_t));

	if(item == NULL) {
		exit(EXIT_FAILURE);
	}

	item->value = value;
	item->next = NULL;
	return item;
}

void add_new(uint8_t value) {
	list_t* tmp = create_new(value);

	if(start == NULL) {
		start = tmp;
		end = tmp;
	}
	else {
		end->next = tmp;
		end = tmp;
	}
	size++;
}

void delete_start() {
	if(start != NULL) {
		list_t* next = start->next;
		free(start);
		if(next != NULL) {
			start = next;
		}
		else {
			start = NULL;
			end = NULL;
		}
		size--;
	}
}

void delete_all() {
	while(start != NULL) {
		delete_start();
	}
}

uint8_t get_start() {
	if(start != NULL) {
		return start->value;
	}
	return -1;
}

int get_size() {
	return size;
}