#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

uint64_t get_physical_address(void *virtual_address)
{
    const uint64_t page_size = getpagesize();
    const uint64_t page_length = 8;
    const uint64_t page_shift = 12;
    uint64_t page_offset, page_number;
    int pagemap;

    pagemap = open("/proc/self/pagemap", O_RDONLY);
    if(pagemap < 0)
    {
        return 0;
    }

    page_offset = (((uint64_t)virtual_address) / page_size * page_length);
    if(lseek(pagemap, page_offset, SEEK_SET) != page_offset)
    {
        close(pagemap);
        return 0;
    }

    page_number = 0;
    if(read(pagemap, &page_number, sizeof(page_number)) != sizeof(page_number))
    {
        close(pagemap);
        return 0;
    }
    page_number &= 0x7FFFFFFFFFFFFFULL;

    close(pagemap);

    return ((page_number << page_shift) + (((uint64_t)virtual_address) % page_size));
}

int main() {
    int arr[10] = {0,1,2,3,4,5,6,7,8,9};

    // Example of finding the physical address of arr
    for(int* i = arr;i <= &arr[9];i++){
        uint64_t physical_addr = get_physical_address(i);
        if (physical_addr) {
            printf("Virtual address: %p , data = %d\n", i, *((int*)(i)));
            printf("Physical address: 0x%" PRIx64 " \n", physical_addr);
        } else {
            printf("Failed to get the physical address.\n");
        }
    }



    int a;
    printf("Waiting...");
    scanf("%d", &a);

    for(int i = 0;i<10;i++){
        printf("%d, ",arr[i]);
    }

    printf("\n\n");

    return 0;
}
