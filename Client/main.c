#include <stdio.h>

#include "client.h"

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Not the right amount of input arguments.\nNeed to give three.\nExiting...\n"); // first is path, other three are inputs
		return ERROR_CODE;
	}

	return SUCCESS_CODE;
}