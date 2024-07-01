#include "os.h"
#include <stdlib.h>





void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    /*There are 5 levels to multi level pagetable as I calculated*/
    uint64_t address_part[5];
    address_part[0] = (vpn >> 36 ) & 0x1FF; // mask for bits 36 - 45 
    address_part[1] = (vpn >> 27) & 0x1FF; // mask for bits 27 - 36 
    address_part[2] = (vpn >> 18 ) & 0x1FF; // mask for bits 18 - 27  
    address_part[3] = (vpn >> 9 ) & 0x1FF; // mask for bits 9 - 18
    address_part[4] = vpn &  0x1FF; // mask for bits 0 - 9 
    
    
    uint64_t current_table_physical;
    current_table_physical = pt << 12;
    uint64_t *current_table;

    for(int i = 0; i< 4; i ++){
        // we check if the address is valid 
        current_table = phys_to_virt(current_table_physical);
        if(current_table == NULL || (1 & current_table[address_part[i]]) == 0 ){
            // if ppn is NO_MAPPING then we return as there is no mapping to be destroyed
            if (ppn == NO_MAPPING){
                return;
            }
            // if ppn is not NO_MAPPING then we allocate a page frame and set the valid bit to 1
            current_table[address_part[i]] = (alloc_page_frame() << 12) + 1;
        }
        current_table_physical = current_table[address_part[i]] - 1;
    }
    current_table = phys_to_virt(current_table_physical);
    if (ppn == NO_MAPPING){
        current_table[address_part[4]] = current_table[address_part[4]] & 0;
    }
    else{
        current_table[address_part[4]] = (ppn << 12) + 1;
    }

    return;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    uint64_t address_part[5];
    address_part[0] = (vpn >> 36 ) & 0x1FF; // mask for bits 36 - 45 
    address_part[1] = (vpn >> 27) & 0x1FF; // mask for bits 27 - 36 
    address_part[2] = (vpn >> 18 ) & 0x1FF; // mask for bits 18 - 27  
    address_part[3] = (vpn >> 9 ) & 0x1FF; // mask for bits 9 - 18
    address_part[4] = vpn &  0x1FF; // mask for bits 0 - 9 
    
    
    uint64_t current_table_physical;
    current_table_physical = pt << 12 ;
    uint64_t *current_table;

    for(int i = 0; i< 4; i ++){
        // we check if the address is valid 
        current_table = phys_to_virt(current_table_physical);
        if(current_table == NULL || (1 & current_table[address_part[i]]) == 0){
            return NO_MAPPING;
            }
        current_table_physical = current_table[address_part[i]] - 1;
    }
    current_table = phys_to_virt(current_table_physical);
    if(current_table == NULL || (1 & current_table[address_part[4]])  == 0){
        return NO_MAPPING;
    }
    if (current_table[address_part[4]] ==  0){
        return NO_MAPPING;
    }

    return current_table[address_part[4]] >> 12;


}