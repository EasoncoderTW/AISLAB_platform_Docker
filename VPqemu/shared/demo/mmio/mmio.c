#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define MMIO_START 0x6000000 // Replace with actual physical address
#define MMIO_SIZE  0x6000000     // Size of the memory region

int main() {
    int fd;
    void *mapped_base;
    volatile uint64_t *mapped_dev_base;

    // Open /dev/mem
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Error opening /dev/mem");
        exit(EXIT_FAILURE);
    }

    // Map the physical address to virtual address space
    mapped_base = mmap(0, MMIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MMIO_START);
    if (mapped_base == (void *) -1) {
        perror("Error mapping memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Access the mapped memory
    mapped_dev_base = (volatile uint64_t *)mapped_base;

    // Read from the device
    uint64_t value = mapped_dev_base[10];
    printf("Read value: 0x%x\n", value);

    // Write to the device
    mapped_dev_base[0] = 0x12345678deadbeef;
    printf("Wrote value: 0x12345678deadbeef\n");

    // Unmap the memory
    if (munmap(mapped_base, MMIO_SIZE) == -1) {
        perror("Error unmapping memory");
    }

    // Close the file
    close(fd);

    return 0;
}