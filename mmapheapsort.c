#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fs.h>

void swap (char *a, char *b) {
	char swaptmp = *a;
	*a = *b;
	*b = swaptmp;
}

off_t left (off_t i) {
	return 2 * i;
}

off_t right (off_t i) {
	return 2 * i + 1;
}

void bubbledown (off_t i, char *heap, off_t n) {
	if (left(i) > n) {
		return;
	} else if (right(i) > n) {
		if (heap[i] < heap[left(i)]) {
			swap(heap+i, heap+left(i));
		}
	} else {
		if (heap[left(i)] > heap[right(i)] && heap[i] < heap[left(i)]) {
			swap(heap+i, heap+left(i));
			bubbledown(left(i), heap, n);
		} else if (heap[i] < heap[right(i)]) {
			swap(heap+i, heap+right(i));
			bubbledown(right(i), heap, n);
		}
	}
}

void heapify (char *a, off_t n) {
	off_t i;
	for (i = n/2; i > 0; i--) {
		bubbledown(i, a, n);
	}
}

void heapsort (char *a, off_t n) {
	a -= 1;
	heapify(a, n);
	off_t j;
	for (j = n; j > 1; j--) {
		swap(a+1, a+j);
		bubbledown(1, a, j-1);
	}
}

int is_sorted (char *a, off_t n) {
	if (n < 2) {
		return 1;
	} else {
		off_t i;
		for (i = 1; i < n; i++) {
			if (a[i] < a[i-1]) return 0;
		}
	}
	
	return 1;
}

int main (int argc, char *argv[])
{
		struct stat sb;
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
			printf("%s is already sorted, sorting again...\n", argv[1]);
		} else {
			printf("%s is not sorted yet\n", argv[1]);
		}
		
		heapsort(p, sb.st_size);

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
