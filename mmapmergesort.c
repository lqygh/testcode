#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fs.h>

void merge (char *a, off_t n, off_t m) {
    off_t i, j, k;
    char *x = malloc(n * sizeof (char));
    for (i = 0, j = m, k = 0; k < n; k++) {
        x[k] = j == n      ? a[i++]
             : i == m      ? a[j++]
             : a[j] < a[i] ? a[j++]
             :               a[i++];
    }
    for (i = 0; i < n; i++) {
        a[i] = x[i];
    }
    free(x);
}
 
void merge_sort (char *a, off_t n) {
    if (n < 2)
        return;
    off_t m = n / 2;
    merge_sort(a, m);
    merge_sort(a + m, n - m);
    merge(a, n, m);
}

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
        char *p;
        int fd;

        if (argc < 2) {
                fprintf (stderr, "usage: %s <file>\n", argv[0]);
                return 1;
        }

        fd = open (argv[1], O_RDWR);
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
		
        p = mmap (0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
                perror ("mmap");
                return 1;
        }

        if (close (fd) == -1) {
                perror ("close");
                return 1;
        }

		if(is_sorted(p, sb.st_size)) {
			printf("%s is already sorted\n", argv[1]);
		} else {
			printf("%s is not sorted yet\n", argv[1]);
		}
		
		merge_sort(p, sb.st_size);

		if(is_sorted(p, sb.st_size)) {
			printf("%s has been sorted\n", argv[1]);
		} else {
			printf("%s has not been sorted\n", argv[1]);
		}
		
        if (munmap (p, sb.st_size) == -1) {
                perror ("munmap");
                return 1;
        }

        return 0;
}
