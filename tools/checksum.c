#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Informe o G(N|P)RMC como parâmetro!\n");
		return 1;
	}

	char gps_line[128];

	char n_bytes_from_argv = strlen(argv[1]);
	strncat(gps_line, argv[1], n_bytes_from_argv);

	char checksum = 0x00;

	// o checksum da linha é basicamente um xor entre todos
	// os caracteres entre $ e *
	for(int i = 0; i < n_bytes_from_argv; i++) {
		checksum ^= gps_line[i];
	}

	printf("checksum: *%02x\n", checksum);

	return 0;
}
