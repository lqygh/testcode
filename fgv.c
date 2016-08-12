#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

//generate sequence 0-255 255-0
void genseq(uint8_t* seq) {
	uint16_t i = 0;
	uint8_t val = 0;
	for(i = 0; i < 256; i++) {
		seq[i] = val++;
	}
	
	val = 255;
	for(i = 256; i < 511; i++) {
		seq[i] = val--;
	}
}

//verify sequence 0-255 255-0
int verseq(uint8_t* seq, size_t size) {
	if(size <= 0) {
		return 1;
	}
	
	if(size > 512) {
		return verseq(seq, 512) && verseq(seq+512, size-512);
	}
	
	uint16_t i = 0;
	uint8_t val = 0;
	for(i = 0; i < 256; i++) {
		if(i > size-1) {
			return 1;
		}
		
		if(seq[i] != val) {
			return 0;
		}
		
		val += 1;
	}
	
	val = 255;
	for(i = 256; i < 511; i++) {
		if(i > size-1) {
			return 1;
		}
		
		if(seq[i] != val) {
			return 0;
		}

		val -= 1;
	}
	
	return 1;
}

int main(int argc, char* argv[]) {
	if(argc < 3) {
		printf("Usage for file generation: %s g <filename>\n", argv[0]);
		printf("Usage for file verification: %s v <filename>\n", argv[0]);
		return 1;
	}
	
	if(argv[1][0] == 'g') {
		uint64_t filesize = 0;
		
		//check if the file already exists
		FILE* fp = fopen(argv[2], "r");
		if(fp != NULL) {
			printf("file %s already exists, please delete it first\n", argv[2]);
			fclose(fp);
			return 1;
		}
		
		//generate sequence array
		uint8_t seq[512];
		genseq(seq);
		
		//write to file
		fp = fopen(argv[2], "w");
		if(fp == NULL) {
			perror("fopen()");
			return 1;
		}
		
		size_t retval = fwrite(&filesize, sizeof(uint64_t), 1, fp);
		if(retval != 1) {
			printf("at least 8 bytes of free space is required for file generation\n");
			fclose(fp);
			return 1;
		}
		
		//keep writing until no space left
		while(1) {
			retval = fwrite(seq, sizeof(uint8_t), sizeof(seq), fp);
			if(retval < sizeof(seq)) {
				break;
			}
		}
		filesize = ftell(fp);
		printf("%" PRIu64 " bytes written\n", filesize);
		
		//write size info to beginning of file
		rewind(fp);
		retval = fwrite(&filesize, sizeof(uint64_t), 1, fp);
		if(retval != 1) {
			printf("failed to write size info to file\n");
			fclose(fp);
			return 1;
		}
		
		//close file
		printf("closing file\n");
		fclose(fp);
		
		return 0;
	} else if(argv[1][0] == 'v') {
		uint64_t filesize = 0;
		uint64_t readcounter = 0;
		
		//open file
		FILE* fp = fopen(argv[2], "r");
		if(fp == NULL) {
			perror("fopen()");
			return 1;
		}
		
		size_t retval = fread(&filesize, sizeof(uint64_t), 1, fp);
		if(retval != 1) {
			printf("failed to read size info from file\n");
			fclose(fp);
			return 1;
		}
		readcounter += sizeof(uint64_t);
		
		uint8_t seq[512] = {0};
		int result = 0;
		while(1) {
			retval = fread(seq, sizeof(uint8_t), sizeof(seq), fp);
			readcounter += retval;
			
			//when verification fails
			if(verseq(seq, retval) != 1) {
				result = 0;
				break;
			}
			
			
			//when reached end of file
			if(retval < sizeof(seq)) {
				//verify file size
				if(readcounter == filesize) {
					result = 1;
				} else {
					printf("file size info should be %" PRIu64 ", but it is actually %" PRIu64 "\n", readcounter, filesize);
					result = 0;
				}
				break;
			}
		}
		
		//close file
		printf("%" PRIu64 " bytes read\n", readcounter);
		printf("closing file\n");
		fclose(fp);
		
		if(result == 1) {
			printf("file verification successful\n");
			return 0;
		} else {
			printf("file verification failed\n");
			return 2;
		}
	} else {
		printf("Unknown argument %s\n", argv[1]);
		return 1;
	}
}