#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fs.h>

int is_sorted (char *a, off_t n) {
	if(n < 2) {
		return 1;
	} else {
		off_t i;
		for(i = 1; i < n; i++) {
			if(a[i] < a[i-1]) return 0;
		}
	}
	
	return 1;
}

int main (int argc, char *argv[])
{
        struct stat sb;
        off_t len;
		off_t size = 0;
		off_t offset = 0;
        char *p;
        int fd;

        if (argc < 4) {
                fprintf (stderr, "usage: %s <file> <size> <offset>\n", argv[0]);
                return 1;
        }
		
        fd = open (argv[1], O_RDONLY);
        if (fd == -1) {
                perror ("open");
                return 1;
        }

        if (fstat (fd, &sb) == -1) {
                perror ("fstat");
                return 1;
        }
		
        if (!S_ISREG (sb.st_mode)) {
                fprintf (stderr, "%s is not a file\n", argv[1]);
                //return 1;
        }

		if (S_ISBLK (sb.st_mode)) {
				fprintf (stderr, "%s is a block device\n", argv[1]);
				ioctl (fd, BLKGETSIZE64, &(sb.st_size));
		}
		
		if (argv[2][0] != '-') {
			size = strtol (argv[2], NULL, 10);
			if (size < 0) offset = 0;
		} else {
			size = sb.st_size;
		}

		if (argv[3][0] != '-') {
			offset = strtol (argv[3], NULL, 10);
			if (offset < 0) offset = 0;
		} else {
			offset = 0;
		}
		
		p = mmap (0, size, PROT_READ, MAP_SHARED, fd, offset);
        if (p == MAP_FAILED) {
                perror ("mmap");
                return 1;
        }

        if (close (fd) == -1) {
                perror ("close");
                return 1;
        }

		if (is_sorted(p, size)) {
			printf ("%s is already sorted\n", argv[1]);
		} else {
			printf ("%s is not sorted yet\n", argv[1]);
		}
		
        char min = p[0], max = p[0];
        for (len = 0; len < size; len++) {
			if(p[len] < min) {
				min = p[len];
			}
			
			if(p[len] > max) {
				max = p[len];
			}
		}

		printf("min is %d, max is %d\n", min, max);

        if (munmap (p, size) == -1) {
                perror ("munmap");
                return 1;
        }

        return 0;
}
