  
#include "cat_stdio.h"
#include "sdio_sdcard.h"     
#include "src/ff.h"  
#include "exfuns/exfuns.h"    
//ALIENTEK��ӢSTM32F103������ ʵ��36
//FATFS ʵ�� 
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
 
int test_fatfs_func(void)
{		
 	u32 total,free;
	u8 t=0;	
	u8 res=0;	 
 
	if(0 != SD_Init())//��ⲻ��SD��
	{
		CAT_SYS_PRINTF("SD Card Error!\r\n");			
		CAT_SYS_PRINTF("Please Check! \r\n");
	}
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  	f_mount(fs[0],"0:",1); 					//����SD�� 

	uint16_t i;
	while(exf_getfree("0",&total,&free))	//�õ�SD������������ʣ������
	{
		CAT_SYS_PRINTF("SD Card Fatfs Error!\r\n");
		for(i=0xffff; i>1; i--);
	}													  			    
	   
	CAT_SYS_PRINTF("fatfs OK!\r\n");	 	   
	CAT_SYS_PRINTF("card(free/total)=%d/%d\r\n", free, total); 		    
	
}

#if (CATOS_ENABLE_CAT_SHELL == 1)
#include "cat_shell.h"

void *do_test_fatfs(void *arg)
{
    (void)arg;
	
	test_fatfs_func();
		
	return NULL;
}
CAT_DECLARE_CMD(test_fatfs, test fatfs, do_test_fatfs);

#endif
