//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** ȫ�ֱ��� ********************************/

#define KEY1_EVENT (0x01 << 0)//�����¼������λ 0 
#define KEY2_EVENT (0x01 << 1)//�����¼������λ 1 

uint32_t r_data;
uint32_t r_tempdata;


/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t LED_Task_Handle = NULL;/*LED_Task ������ */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task ������ */







//��������
static void LED_Task(void* parameter);
static void KEY_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BSP_Init();
	printf("����һ��[Ұ��]-STM32 ȫϵ�п�����-FreeRTOS ����֪ͨ������Ϣ����ʵ�飡\n");
	printf("���� KEY1|KEY2 ���������¼�֪ͨ��\n");

	

	
	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}



static void LED_Task(void* parameter)
{
	BaseType_t xReturn=pdFALSE;

	while(1)
	{
		
	  /* BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, 
		uint32_t ulBitsToClearOnExit, 
		uint32_t *pulNotificationValue, 
		TickType_t xTicksToWait ); 
		* ulBitsToClearOnEntry����û�н��յ�����֪ͨ��ʱ������ֵ֪ͨ��˲�����ȡ 
		��ֵ���а�λ�����㣬���˲���Ϊ Oxfffff ���� ULONG_MAX ��ʱ��ͻὫ����ֵ֪ͨ���㡣 
		* ulBits ToClearOnExit��������յ�������֪ͨ����������Ӧ�Ĵ����˳�����֮ǰ�� 
		����ֵ֪ͨ��˲�����ȡ��ֵ���а�λ�����㣬���˲���Ϊ 0xfffff ���� ULONG MAX ��ʱ�� 
		�ͻὫ����ֵ֪ͨ���㡣 
		* pulNotification Value���˲���������������ֵ֪ͨ�� 
		* xTick ToWait������ʱ�䡣 
		* 
		* ����ֵ��pdTRUE����ȡ��������֪ͨ��pdFALSE������֪ͨ��ȡʧ�ܡ� 
		*/ 
		//��ȡ����֪ͨ ,û��ȡ����һֱ�ȴ� 
		xReturn=xTaskNotifyWait(0x0,               //���뺯����ʱ��������� bit
		                ULONG_MAX,         //�˳�������ʱ��������е� bitR 
										&r_data,          //��������ֵ֪ͨ 
		                portMAX_DELAY);    //����ʱ�� 
		if(xReturn==pdTRUE)
		{
			r_tempdata|=r_data;
			if(r_tempdata== (KEY1_EVENT|KEY2_EVENT))
			{
				printf("LED_Task ����֪ͨ��ȡ�ɹ�!\n");
				LED_G_TOGGLE();
				r_tempdata=0;
			}
			else
			{
				
			}
		}


	}
}


static void KEY_Task(void* parameter)
{
	while(1)
	{
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			printf ( "KEY1 ������\n" );
			xTaskNotify(LED_Task_Handle,KEY1_EVENT,eSetBits);
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			printf ( "KEY2 ������\n" );
			xTaskNotify(LED_Task_Handle,KEY2_EVENT,eSetBits);
		}
		vTaskDelay(20); 
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	taskENTER_CRITICAL();           //�����ٽ���
	
	xReturn=xTaskCreate((TaskFunction_t	)LED_Task,		//������
															(const char* 	)"LED_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&LED_Task_Handle);/* ������ƿ�ָ�� */ 
															
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("LED_Task���񴴽��ɹ�!\n");
	else
		printf("LED_Task���񴴽�ʧ��!\n");
	

	xReturn=xTaskCreate((TaskFunction_t	)KEY_Task,		//������
															(const char* 	)"KEY_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)4, 	//�������ȼ�
															(TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("KEY_Task���񴴽��ɹ�!\n");
	else
		printf("KEY_Task���񴴽�ʧ��!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}


//��̬�����������Ҫ
///**
//  **********************************************************************
//  * @brief  ��ȡ��������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* �����ջ�ڴ� */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* �����ջ��С */
//}



///**
//  *********************************************************************
//  * @brief  ��ȡ��ʱ������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* �����ջ�ڴ� */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* �����ջ��С */
//}
