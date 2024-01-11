/*************************************************************************
	> File Name: check-sm-table.c
	> Author:
	> Mail:
	> Created Time: 2024年01月10日 星期三 11时34分43秒
        > Attention input guest_addr_val sampe: 0x7fff888001230000
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

#define INDEX_END  0xFF8
#define RTE_MAP_SIZE  0xFFF
#define CTE_MAP_SIZE  0xFFF
#define PASIDDIRTE_MAP_SIZE  0xFFF
#define PASIDTE_MAP_SIZE  0xFFF
#define HUGEPAGE_BIT(x)  ((x & 0x80) >> 7)

#define REG_MAP_SIZE 0xFFF
#define RTE_MAP_SIZE 0xFFF
#define CTE_MAP_SIZE 0xFFF
#define PASIDTE_MAP_SIZE 0xFFF

#define SL_1st_MAP_SIZE 0xFFF
#define SL_2nd_MAP_SIZE 0xFFF
#define SL_3rd_MAP_SIZE 0xFFF
#define SL_4th_MAP_SIZE 0xFFF

#define FL_1st_MAP_SIZE 0xFFF
#define FL_2nd_MAP_SIZE 0xFFF
#define FL_3rd_MAP_SIZE 0xFFF
#define FL_4th_MAP_SIZE 0xFFF

#define PML4_PAGE_OFFSET(x)  (x & 0xFFF)
#define PML4_1st_OFFSET(x)  ((x & 0xFF8000000000) >> 39) << 3
#define PML4_2nd_OFFSET(x)  ((x & 0x7FC0000000) >> 30) << 3
#define PML4_3rd_OFFSET(x)  ((x & 0x3FE00000) >> 21) << 3
#define PML4_4th_OFFSET(x)  ((x & 0x1FF000) >> 12) << 3

#define INDEX_OFFSET(val)  (val / 4)
#define PASIDDIR_INDEX(val)  (val * 2)
#define PASID_INDEX(val)  (val * 16)

#define PRESENT_BIT(x)  (x & 0x1)

int walk_page_structure_entry(unsigned long long int SLPTPTR_val,
		              unsigned long long int input_va_val, int fd)
{
	int bit0_11 = PML4_PAGE_OFFSET(input_va_val);   // page offset
	int bit12_20 = PML4_4th_OFFSET(input_va_val);  // forth offset
	int bit21_29 = PML4_3rd_OFFSET(input_va_val);  // third offset
	int bit30_39 = PML4_2nd_OFFSET(input_va_val);  // second offset
	int bit39_47 = PML4_1st_OFFSET(input_va_val);  // first offset
	unsigned long long int SLPTPTR_1level;
        unsigned long long int SLPTPTR_2level;
        unsigned long long int SLPTPTR_3level;
        unsigned long long int SLPTPTR_4level;

	unsigned int *slp_addr_va = NULL;
        unsigned int *slp_addr_2nd_va = NULL;
        unsigned int *slp_addr_3rd_va = NULL;
        unsigned int *slp_addr_4th_va = NULL;
	int *start;

	slp_addr_va = (unsigned int *)(0x7AA66000);

	printf("input va: %#llx\n", input_va_val);
	printf("page offset bit0_11: %#x\n", bit0_11);
	printf("4th offset bit12_20: %#x\n", bit12_20);
	printf("3rd offset bit21_29: %#x\n", bit21_29);
	printf("2nd offset bit30_39: %#x\n", bit30_39);
	printf("1st offset bit39_47: %#x\n", bit39_47);

	start = (int *)mmap(slp_addr_va, SL_1st_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_val);
	if (start < 0)
        {
		printf("slp_addr_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("===start walk second pml4 table structure===\n");
	printf("1st offset: %#x, pointer val: 0x%08x%08x\n", bit39_47,
			slp_addr_va[1 + INDEX_OFFSET(bit39_47)], slp_addr_va[0 + INDEX_OFFSET(bit39_47)]);

        SLPTPTR_1level = (unsigned long long int)slp_addr_va[1 + INDEX_OFFSET(bit39_47)] << 32
                        | slp_addr_va[0 + INDEX_OFFSET(bit39_47)];

        if (munmap(slp_addr_va, SL_1st_MAP_SIZE) == -1)
                printf("slp_addr_va munmap error\n");

        if (PRESENT_BIT(SLPTPTR_1level) == 0)
                goto entry_not_present;

        SLPTPTR_1level >>= 12;
        SLPTPTR_1level <<= 12;
        printf("SLPTPTR_1level=0x%llx\n", SLPTPTR_1level);

        if (SLPTPTR_1level == 0)
                goto addr_0_err;

        slp_addr_2nd_va = (unsigned int *)(0x8AA66000);

        start = (int *)mmap(slp_addr_2nd_va, SL_2nd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_1level);
	if (start < 0)
        {
		printf("slp_addr_2nd_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("2nd offset: %#x, pointer val: 0x%08x%08x\n", bit30_39,
              slp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)], slp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)]);

        SLPTPTR_2level = (unsigned long long int)slp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)] << 32
                        | slp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)];

        if (munmap(slp_addr_2nd_va, SL_2nd_MAP_SIZE) == -1)
                printf("slp_addr_2nd_va munmap error\n");

        if (HUGEPAGE_BIT(SLPTPTR_2level) == 1)
                goto GB_HUGEPAGE;
        if (PRESENT_BIT(SLPTPTR_2level) == 0)
                goto entry_not_present;

        SLPTPTR_2level >>= 12;
        SLPTPTR_2level <<= 12;
        printf("SLPTPTR_2level=0x%llx\n", SLPTPTR_2level);

        if (SLPTPTR_2level == 0)
                goto addr_0_err;

        slp_addr_3rd_va = (unsigned int *)(0x9AA66000);
        start = (int *)mmap(slp_addr_3rd_va, SL_3rd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_2level);
        if (start < 0)
        {
                printf("slp_addr_3rd_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("3rd offset: %#x, pointer val: 0x%08x%08x\n", bit21_29,
              slp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)], slp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)]);
        SLPTPTR_3level = (unsigned long long int)slp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)] << 32
                        | slp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)];

        if (munmap(slp_addr_3rd_va, SL_3rd_MAP_SIZE) == -1)
                printf("slp_addr_3rd_va munmap error\n");

        if (HUGEPAGE_BIT(SLPTPTR_3level) == 1)
                goto MB_HUGEPAGE;
        if (PRESENT_BIT(SLPTPTR_3level) == 0)
                goto entry_not_present;

        SLPTPTR_3level >>= 12;
        SLPTPTR_3level <<= 12;
        printf("SLPTPTR_3level=0x%llx\n", SLPTPTR_3level);

        if (SLPTPTR_3level == 0)
                goto addr_0_err;

        slp_addr_4th_va = (unsigned int *)(0xaAA66000);
        start = (int *)mmap(slp_addr_4th_va, SL_4th_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_3level);
        if (start < 0)
        {
                printf("slp_addr_4th_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("4th offset: %#x, pointer val: 0x%08x%08x\n", bit12_20,
              slp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)], slp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)]);
        SLPTPTR_4level = (unsigned long long int)slp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)] << 32
                        | slp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)];

        if (munmap(slp_addr_4th_va, SL_4th_MAP_SIZE) == -1)
                printf("slp_addr_4th_va munmap errpr\n");

        if (PRESENT_BIT(SLPTPTR_4level) == 0)
                goto entry_not_present;

        SLPTPTR_4level >>= 12;
        SLPTPTR_4level <<=12;
        printf("SLPTPTR_4level=0x%llx\n", SLPTPTR_4level);

        if (SLPTPTR_4level == 0)
                goto addr_0_err;


        return 0;

addr_0_err:
        printf("this level addr pointer is 0!!\n");
        return -2;

entry_not_present:
        printf("this entry not present\n");
        return -3;

GB_HUGEPAGE:
        printf("Used 1GB Huge Page\n");
        return 0;

MB_HUGEPAGE:
        printf("Used 2MB Huge Page\n");
        return 0;
}

int walk_first_page_structure_entry(unsigned long long int FLPTPTR_val,
		              unsigned long long int input_va_val, int fd)
{
	int bit0_11 = PML4_PAGE_OFFSET(input_va_val);   // page offset
	int bit12_20 = PML4_4th_OFFSET(input_va_val);  // forth offset
	int bit21_29 = PML4_3rd_OFFSET(input_va_val);  // third offset
	int bit30_39 = PML4_2nd_OFFSET(input_va_val);  // second offset
	int bit39_47 = PML4_1st_OFFSET(input_va_val);  // first offset
	unsigned long long int FLPTPTR_1level;
        unsigned long long int FLPTPTR_2level;
        unsigned long long int FLPTPTR_3level;
        unsigned long long int FLPTPTR_4level;

	unsigned int *flp_addr_va = NULL;
        unsigned int *flp_addr_2nd_va = NULL;
        unsigned int *flp_addr_3rd_va = NULL;
        unsigned int *flp_addr_4th_va = NULL;
	int *start;

	flp_addr_va = (unsigned int *)(0x7AA66000);

	printf("input va: %#llx\n", input_va_val);
	printf("page offset bit0_11: %#x\n", bit0_11);
	printf("4th offset bit12_20: %#x\n", bit12_20);
	printf("3rd offset bit21_29: %#x\n", bit21_29);
	printf("2nd offset bit30_39: %#x\n", bit30_39);
	printf("1st offset bit39_47: %#x\n", bit39_47);

	start = (int *)mmap(flp_addr_va, FL_1st_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_val);
	if (start < 0)
        {
		printf("flp_addr_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("===start walk first pml4 table structure===\n");
	printf("1st offset: %#x, pointer val: 0x%08x%08x\n", bit39_47,
			flp_addr_va[1 + INDEX_OFFSET(bit39_47)], flp_addr_va[0 + INDEX_OFFSET(bit39_47)]);

        FLPTPTR_1level = (unsigned long long int)flp_addr_va[1 + INDEX_OFFSET(bit39_47)] << 32
                        | flp_addr_va[0 + INDEX_OFFSET(bit39_47)];

        if (munmap(flp_addr_va, FL_1st_MAP_SIZE) == -1)
                printf("flp_addr_va munmap error\n");

        if (PRESENT_BIT(FLPTPTR_1level) == 0)
                goto entry_not_present;

        FLPTPTR_1level >>= 12;
        FLPTPTR_1level <<= 12;
        printf("FLPTPTR_1level=0x%llx\n", FLPTPTR_1level);

        if (FLPTPTR_1level == 0)
                goto addr_0_err;

        flp_addr_2nd_va = (unsigned int *)(0x8AA66000);

        start = (int *)mmap(flp_addr_2nd_va, FL_2nd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_1level);
	if (start < 0)
        {
		printf("flp_addr_2nd_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("2nd offset: %#x, pointer val: 0x%08x%08x\n", bit30_39,
              flp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)], flp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)]);

        FLPTPTR_2level = (unsigned long long int)flp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)] << 32
                        | flp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)];

        if (munmap(flp_addr_2nd_va, FL_2nd_MAP_SIZE) == -1)
                printf("flp_addr_2nd_va munmap error\n");

        if (HUGEPAGE_BIT(FLPTPTR_2level) == 1)
                goto GB_HUGEPAGE;
        if (PRESENT_BIT(FLPTPTR_2level) == 0)
                goto entry_not_present;

        FLPTPTR_2level >>= 12;
        FLPTPTR_2level <<= 12;
        printf("FLPTPTR_2level=0x%llx\n", FLPTPTR_2level);

        if (FLPTPTR_2level == 0)
                goto addr_0_err;

        flp_addr_3rd_va = (unsigned int *)(0x9AA66000);
        start = (int *)mmap(flp_addr_3rd_va, FL_3rd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_2level);
        if (start < 0)
        {
                printf("flp_addr_3rd_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("3rd offset: %#x, pointer val: 0x%08x%08x\n", bit21_29,
              flp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)], flp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)]);
        FLPTPTR_3level = (unsigned long long int)flp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)] << 32
                        | flp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)];

        if (munmap(flp_addr_3rd_va, FL_3rd_MAP_SIZE) == -1)
                printf("flp_addr_3rd_va munmap error\n");

        if (HUGEPAGE_BIT(FLPTPTR_3level) == 1)
                goto MB_HUGEPAGE;
        if (PRESENT_BIT(FLPTPTR_3level) == 0)
                goto entry_not_present;

        FLPTPTR_3level >>= 12;
        FLPTPTR_3level <<= 12;
        printf("FLPTPTR_3level=0x%llx\n", FLPTPTR_3level);

        if (FLPTPTR_3level == 0)
                goto addr_0_err;

        flp_addr_4th_va = (unsigned int *)(0xaAA66000);
        start = (int *)mmap(flp_addr_4th_va, FL_4th_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_3level);
        if (start < 0)
        {
                printf("flp_addr_4th_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("4th offset: %#x, pointer val: 0x%08x%08x\n", bit12_20,
              flp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)], flp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)]);
        FLPTPTR_4level = (unsigned long long int)flp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)] << 32
                        | flp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)];

        if (munmap(flp_addr_4th_va, FL_4th_MAP_SIZE) == -1)
                printf("flp_addr_4th_va munmap errpr\n");

        if (PRESENT_BIT(FLPTPTR_4level) == 0)
                goto entry_not_present;

        FLPTPTR_4level >>= 12;
        FLPTPTR_4level <<=12;
        printf("FLPTPTR_4level=0x%llx\n", FLPTPTR_4level);

        if (FLPTPTR_4level == 0)
                goto addr_0_err;

        return 0;

addr_0_err:
        printf("this level addr pointer is 0!!\n");
        return -2;

entry_not_present:
        printf("this entry is not present\n");
        return -3;

GB_HUGEPAGE:
        printf("Used 1GB Huge Page\n");
        return 0;

MB_HUGEPAGE:
        printf("Used 2MB Huge Page\n");
        return 0;
}

int walk_sm_structure_entry(int fd, unsigned long long int guest_addr_val,
                            int pasid_val, int bus_num_val, int dev_num_val,
                            int func_num_val, unsigned long long rte_val)
{
        int *start;
        unsigned int *rte_val_va = NULL;
        unsigned int *sm_cte_val_va = NULL;
        unsigned int *sm_pasiddirte_addr_va = NULL;
        unsigned int *sm_pasidte_addr_va = NULL;

        rte_val_va = (unsigned int *)(0x5AA66000);
        sm_cte_val_va = (unsigned int *)(0x6AA66000);
        sm_pasiddirte_addr_va = (unsigned int *)(0x1AA66000);
        sm_pasidte_addr_va = (unsigned int *)(0x2AA66000);

        unsigned long long int SM_CTP;
        unsigned long long int PASIDDIRPTR;
        unsigned long long int PASIDPTR;
        unsigned long long int FLPTPTR;
        unsigned long long int SLPTPTR;

        int ret = 0;

        printf("rte val = %#llx\n", rte_val);

        // sm root entry
        printf("===Used Scalable Mode Root Entry===\n");
        start = (int *)mmap(rte_val_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rte_val);
        if (start < 0)
        {
                printf("rte_val_va mmap failed in %s\n", __func__);
                return -1;
        }

        if (dev_num_val <= 0xF && dev_num_val >= 0)
        {
                printf("dev_num <= 0xF(15) Using LCTP and SM Lower Context Table\n");
                printf("Root Table Entry bus %#x Offset=%#x,   [63-0]:0x%08x%08x\n",
                        bus_num_val, bus_num_val * 2 * 8,
                        rte_val_va[1 + INDEX_OFFSET(bus_num_val * 0x10)], rte_val_va[0 + INDEX_OFFSET(bus_num_val * 0x10)]);
                printf("Root Table Entry bus %#x Offset=%#x, [127-64]:0x%08x%08x\n",
                        bus_num_val, bus_num_val * 2 * 8,
                        rte_val_va[3 + INDEX_OFFSET(bus_num_val * 0x10)], rte_val_va[2 + INDEX_OFFSET(bus_num_val * 0x10)]);

                SM_CTP = (unsigned long long int)rte_val_va[1 + INDEX_OFFSET(bus_num_val * 0x10)] << 32 |
                                rte_val_va[0 + INDEX_OFFSET(bus_num_val * 0x10)];

                printf("SM_CTP=%#llx\n", SM_CTP);

                if (PRESENT_BIT(SM_CTP) == 0)
                {
                        if (munmap(rte_val_va, RTE_MAP_SIZE) == -1)
                                printf("rte_val_va munmap error\n");

                        printf("lower sm root entry lower not present\n");
                        return -3;
                }

                SM_CTP >>= 12;  // clear the control bit
                SM_CTP <<= 12;
        }

        else if (dev_num_val <= 0xFF && dev_num_val >= 0x10)
        {
                printf("dev_num >= 0x10(16) Using UCTP and SM Upper Context Table\n");
                printf("Root Table Entry bus %#x Offset=%#x, [63-0]:0x%08x%08x [127-64]:0x%08x%08x\n",
                        bus_num_val, bus_num_val * 2 * 8,
                        rte_val_va[1 + INDEX_OFFSET(bus_num_val * 0x10)], rte_val_va[0 + INDEX_OFFSET(bus_num_val * 0x10)],
                        rte_val_va[3 + INDEX_OFFSET(bus_num_val * 0x10)], rte_val_va[2 + INDEX_OFFSET(bus_num_val * 0x10)]);

                SM_CTP = (unsigned long long int)rte_val_va[3 + INDEX_OFFSET(bus_num_val * 0x10)] << 32 |
                                rte_val_va[2 + INDEX_OFFSET(bus_num_val * 0x10)];

                printf("SM_CTP=%#llx\n", SM_CTP);


                if (PRESENT_BIT(SM_CTP) == 0)
                {
                        if (munmap(rte_val_va, RTE_MAP_SIZE) == -1)
                                printf("rte_val_va munmap error\n");

                        printf("lower sm root entry upper not present\n");
                        return -3;
                }

                SM_CTP >>= 12;  // clear the control bit
                SM_CTP <<= 12;
        }

        if (munmap(rte_val_va, RTE_MAP_SIZE) == -1)
                printf("rte_val_va munmap error\n");

        // sm context entry
        printf("===Used Scalable Mode Context Entry===\n");
        start = (int *)mmap(sm_cte_val_va, CTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SM_CTP);
        if (start < 0)
        {
                printf("sm cte_addr_va mmap failed in %s\n", __func__);
                return -1;
        }

        int offset_sm_context = dev_num_val << 8 | func_num_val << 5;
        printf("sm context entry dev:0x%x func:%d    [63-0]:0x%08x%08x\n",
               dev_num_val, func_num_val, sm_cte_val_va[1 + offset_sm_context / 8], sm_cte_val_va[0 + offset_sm_context / 8]);
        printf("sm context entry dev:0x%x func:%d  [127-64]:0x%08x%08x\n",
               dev_num_val, func_num_val, sm_cte_val_va[3 + offset_sm_context / 8], sm_cte_val_va[2 + offset_sm_context / 8]);
        printf("sm context entry dev:0x%x func:%d [191-128]:0x%08x%08x\n",
               dev_num_val, func_num_val, sm_cte_val_va[5 + offset_sm_context / 8], sm_cte_val_va[4 + offset_sm_context / 8]);
        printf("sm context entry dev:0x%x func:%d [255-192]:0x%08x%08x\n",
               dev_num_val, func_num_val, sm_cte_val_va[7 + offset_sm_context / 8], sm_cte_val_va[6 + offset_sm_context / 8]);

        PASIDDIRPTR = (unsigned long long int)sm_cte_val_va[1 + offset_sm_context / 8] << 32 |
                                                sm_cte_val_va[0 + offset_sm_context / 8];

        printf("PASIDDIRPTR=%#llx\n", PASIDDIRPTR);

        if (PRESENT_BIT(PASIDDIRPTR) == 0)
        {
                if (munmap(sm_cte_val_va, RTE_MAP_SIZE) == -1)
                        printf("rte_val_va munmap error\n");

                printf("Scalable Mode Context Entry not present\n");
                return -3;
        }

        PASIDDIRPTR >>= 12;
        PASIDDIRPTR <<= 12;

        if (munmap(sm_cte_val_va, RTE_MAP_SIZE) == -1)
                printf("rte_val_va munmap error\n");

        if (pasid_val ==0)
                goto pasid_zero;

        int pasid_val_0_5 = pasid_val & 0x3F;
        int pasid_val_6_19 = pasid_val >> 6;

        if (pasid_val_6_19 != 0)
        {
                start = (int *)mmap(sm_pasiddirte_addr_va, PASIDDIRTE_MAP_SIZE,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, PASIDDIRPTR);

                if (start < 0)
                {
                        printf("sm_pasiddirte_addr_va mmap failed in %s\n", __func__);
                        return -1;
                }
                printf("pasid directory entry pasid_val_6_19:%#x, [63-0]:0x%08x%08x\n", pasid_val_6_19,
                        sm_pasiddirte_addr_va[1 + PASIDDIR_INDEX(pasid_val_6_19)],
                        sm_pasiddirte_addr_va[0 + PASIDDIR_INDEX(pasid_val_6_19)]);

        }

        PASIDPTR = (unsigned long long int)sm_pasiddirte_addr_va[1 + PASIDDIR_INDEX(pasid_val_6_19)] << 32 |
                        sm_pasiddirte_addr_va[0 + PASIDDIR_INDEX(pasid_val_6_19)];

        printf("PASIDPTR=%#llx\n", PASIDPTR);

        if (PRESENT_BIT(PASIDPTR) == 0)
        {
                if (munmap(sm_pasiddirte_addr_va, PASIDDIRTE_MAP_SIZE) == -1)
                        printf("sm_pasiddirte_addr_va");

                printf("Scalable Mode Pasid Directory Entry not present\n");
                return -3;
        }

        PASIDPTR >>= 12;
        PASIDPTR <<= 12;

        if (munmap(sm_pasiddirte_addr_va, PASIDDIRTE_MAP_SIZE) == -1)
                printf("sm_pasiddirte_addr_va");

        if (pasid_val_0_5 != 0)
        {
                start = (int *)mmap(sm_pasidte_addr_va, PASIDTE_MAP_SIZE,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, PASIDPTR);
                if (start < 0)
                {
                        printf("sm_pasidte_addr_va mmap failed in %s\n", __func__);
                        return -1;
                }
                printf("pasid entry pasid_val_0_5:%#x    [63-0]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[1 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[0 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x  [127-64]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[3 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[2 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [191-128]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[5 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[4 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [255-192]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[7 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[6 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [319-256]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[9 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[8 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [383-320]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[11 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[10 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [447-384]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[13 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[12 + PASID_INDEX(pasid_val_0_5)]);
                printf("pasid entry pasid_val_0_5:%#x [511-448]:0x%08x%08x\n", pasid_val_0_5,
                       sm_pasidte_addr_va[15 + PASID_INDEX(pasid_val_0_5)], sm_pasidte_addr_va[14 + PASID_INDEX(pasid_val_0_5)]);
        }

        FLPTPTR = (unsigned long long int)sm_pasidte_addr_va[5 + PASID_INDEX(pasid_val_0_5)] << 32 |
                        sm_pasidte_addr_va[4 + PASID_INDEX(pasid_val_0_5)];

        SLPTPTR = (unsigned long long int)sm_pasidte_addr_va[1 + PASID_INDEX(pasid_val_0_5)] << 32 |
                        sm_pasidte_addr_va[0 + PASID_INDEX(pasid_val_0_5)];

        int PGTT = (SLPTPTR & 0x1C0) >> 6;
        printf("PGTT=%#x\n", PGTT);

        if (munmap(sm_pasidte_addr_va, PASIDTE_MAP_SIZE) == -1)
                printf("sm_pasidte_addr_va munmap error\n");

        if (PGTT == 1)
        {
                FLPTPTR >>= 12;
                FLPTPTR <<= 12;
                if (FLPTPTR != 0)
                {
                        printf("FLPTPTR=0x%llx\n", FLPTPTR);
                        ret = walk_first_page_structure_entry(FLPTPTR, guest_addr_val, fd);
                }
        }
        else if (PGTT == 2)
        {
                SLPTPTR >>= 12;
                SLPTPTR <<= 12;
                if (SLPTPTR != 0)
                {
                        printf("SLPTPTR=0x%llx\n", SLPTPTR);
                        ret = walk_page_structure_entry(SLPTPTR, guest_addr_val, fd);
                }
        }

        return ret;

pasid_zero:
        printf("input pasid is zero\n");

        return 0;
}

int main(int argc, char* argv[], char* envp[])
{
        int file_device;
        unsigned long long int input_guest_addr = 0xffffffff;
        int input_pasid_val = 0;  // pasid 0 reserved for RID_PASID
        int input_bus_num = 0x0;
        int input_dev_num = 0x0 ;
        int input_func_num = 0x0;
        unsigned long long int input_rte_addr = 0xfffffffff;

        int ret = 0;

        char *ptr;
        if (argv[1] != NULL)
                input_guest_addr = strtoll(argv[1], &ptr, 16);
        printf("0x%llx\n", input_guest_addr);

        char *ptr_pasid;
        if (argv[2] != NULL)
                input_pasid_val = strtoll(argv[2], &ptr_pasid, 16);
        printf("0x%x\n", input_pasid_val);

        char *ptr_bus_num;
        if (argv[3] != NULL)
                input_bus_num = strtoll(argv[3], &ptr_bus_num, 16);
        printf("0x%x\n", input_bus_num);

        char *ptr_dev_num;
        if (argv[4] != NULL)
                input_dev_num = strtoll(argv[4], &ptr_dev_num, 16);
        printf("0x%x\n", input_dev_num);

        char *ptr_func_num;
        if (argv[5] != NULL)
                input_func_num = strtoll(argv[5], &ptr_func_num, 16);
        printf("0x%x\n", input_func_num);

        char *ptr_rta;
        if (argv[6] != NULL)
                input_rte_addr = strtoll(argv[6], &ptr_rta, 16);
        printf("0x%llx\n", input_rte_addr);

        file_device = open("/dev/mem", O_RDWR);
        if (file_device < 0)
        {
                printf("cannot open /dev/mem");
                return -1;
        }

        ret = walk_sm_structure_entry(file_device, input_guest_addr,
                                      input_pasid_val, input_bus_num,
                                      input_dev_num, input_func_num, input_rte_addr);

        return ret;
}
