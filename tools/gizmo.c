/**
 * GreenPois0n Gizmo - gizmo.c
 * Copyright (C) 2010 Chronic-Dev Team
 * Copyright (C) 2010 Joshua Hill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROP_MOV_SP_R7 "\xA7\xF1\x00\x0D"
#define ROP_SUB_SP_R7 "\xA7\xF1"
#define ROP_SUB_R7_RX "\x0D"
#define ROP_POP_R7_PC "\x80\xBD"
#define ROP_POP_R4_R7_PC "\x90\xBD"
#define ROP_POP_R4_TO_R7_PC "\xF0\xBD"
#define ROP_POP_R4_R5_R7_PC "\xB0\xBD"

int main(int argc, char* argv[]) {
	uint64_t i = 0;
	uint64_t found = 0;
	uint64_t offset = 0;
	unsigned char shift = 0;
	unsigned char data[0x10];
	unsigned int offsets[10][50000];
	unsigned int* location = NULL;

	if (argc != 2) {
		printf("Usage: ./gizmo <dyldcache>\n");
		return -1;
	}
	char* cache_path = argv[1];

	FILE* fd = fopen(cache_path, "rb");
	if (fd) {
		// Find the size
		fseek(fd, 0, SEEK_END);
		uint64_t cache_size = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		//printf("Opened file %s, %d bytes long\n", cache_path, cache_size);
		printf("// GreenPois0n Absinthe - offsets.h\n");
		printf("// Copyright (C) 2010 Chronic-Dev Team\n");
		printf("// WARNING: This file is autogenerated. Please don't edit by hand\n\n");
		printf("#ifndef OFFSETS_H\n");
		printf("#define OFFSETS_H\n\n");
		printf("typedef struct rop_offsets_t {\n");
		printf("\tunsigned int offset;\n");
		printf("\tunsigned short size;\n");
		printf("\tunsigned short shift;\n");
		printf("} rop_offsets_t;\n\n");
		printf("const rop_offsets_t offsets[] = {");
		for (i = 0; i < cache_size; i++) {
			// Ok now let's loop through the binary searching for ROP gadgets
			fseek(fd, i, SEEK_SET);
			memset(data, '\0', 0x10);
			int got = fread(data, 1, 10, fd);
			if (got == 10) {
				//if(memcmp(data, "\xA7\xF1\x18\x0D\xBD\xE8\x00\x0D\xF0\xBD", 10) == 0) {
				if ((memcmp(data, ROP_SUB_SP_R7, 2) == 0) && (memcmp(&data[3], ROP_SUB_R7_RX, 1) == 0)) {
					shift = data[2];
					unsigned int* index = &offsets[shift / 4];
					//printf("\nMOV SP, R7 at 0x%x\n", i);
					//printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
					//data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
					//if(shift != 0) {
					if ((memcmp(data + 4, ROP_POP_R7_PC, 2) == 0) || (memcmp(data + 4, ROP_POP_R4_R7_PC, 2) == 0) || (memcmp(data + 4, ROP_POP_R4_R5_R7_PC, 2) == 0)
					        || (memcmp(data + 4, ROP_POP_R4_TO_R7_PC, 2) == 0)) {
						//printf("POP {R7,PC} at 0x%x\n", i + 4);
						//printf("Found shift 0x%02x -> ", data[2]);
						//printf("0x%08x: ", i);
						//printf("%02x %02x %02x %02x %02x %02x\n",
						//data[0], data[1], data[2], data[3], data[4], data[5]);
						//printf("\n\t0x%08x,", i);
						found++;
						offset = i;
						if (*index < 49999) {
							location = &offsets[shift / 4][index[0]];
							//printf("Added offset 0x%08x to shift array index %d at location %p\n", i, shift / 4, location);
							int size = 0;
							if(memcmp(data + 4, ROP_POP_R7_PC, 2) == 0) {
								size = 8;
							} else
							if(memcmp(data + 4, ROP_POP_R4_R7_PC, 2) == 0) {
								size = 12;
							} else
							if(memcmp(data + 4, ROP_POP_R4_R5_R7_PC, 2) == 0) {
								size = 16;
							} else
							if(memcmp(data + 4, ROP_POP_R4_TO_R7_PC, 2) == 0) {
								size = 20;
							}
							printf("\n\t{ 0x%08x, 0x%04x, 0x%04x },", i, size, shift);
						} else {
							fprintf(stderr, "Already too many of these gadgets\n");
							break;
						}
						index[0] += 1;
						index[index[0]] = offset;
					}
					//}
				}

				//printf("\r(%d / %d) Found %d gadgets (0x%x)", i, cache_size, found, offset);

			} else {
				break;
			}
		}
		fclose(fd);
		printf("\n\t{ 0x0, 0x0, 0x0 },");
		printf("\n};\n\n");
		printf("#endif\n");
		//printf("\nFound %d gadgets in dyldcache\n");
		fprintf(stderr, "Found %d %d byte shift gadgets\n", offsets[0][0], 0);
		fprintf(stderr, "Found %d %d byte shift gadgets\n", offsets[1][0], 4);
		fprintf(stderr, "Found %d %d byte shift gadgets\n", offsets[2][0], 8);
		fprintf(stderr, "Found %d %d byte shift gadgets\n", offsets[3][0], 12);
		fprintf(stderr, "Found %d %d byte shift gadgets\n", offsets[4][0], 16);

	} else {
		printf("Unable to open dyldcache\n");
		return -1;
	}
	return 0;
}
