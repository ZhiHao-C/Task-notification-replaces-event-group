//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** 全局变量 ********************************/

#define KEY1_EVENT (0x01 << 0)//设置事件掩码的位 0 
#define KEY2_EVENT (0x01 << 1)//设置事件掩码的位 1 

uint32_t r_data;
uint32_t r_tempdata;


/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t LED_Task_Handle = NULL;/*LED_Task 任务句柄 */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY_Task 任务句柄 */







//声明函数
static void LED_Task(void* parameter);
static void KEY_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	BSP_Init();
	printf("这是一个[野火]-STM32 全系列开发板-FreeRTOS 任务通知代替消息队列实验！\n");
	printf("按下 KEY1|KEY2 发送任务事件通知！\n");

	

	
	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
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
		* ulBitsToClearOnEntry：当没有接收到任务通知的时候将任务通知值与此参数的取 
		反值进行按位与运算，当此参数为 Oxfffff 或者 ULONG_MAX 的时候就会将任务通知值清零。 
		* ulBits ToClearOnExit：如果接收到了任务通知，在做完相应的处理退出函数之前将 
		任务通知值与此参数的取反值进行按位与运算，当此参数为 0xfffff 或者 ULONG MAX 的时候 
		就会将任务通知值清零。 
		* pulNotification Value：此参数用来保存任务通知值。 
		* xTick ToWait：阻塞时间。 
		* 
		* 返回值：pdTRUE：获取到了任务通知。pdFALSE：任务通知获取失败。 
		*/ 
		//获取任务通知 ,没获取到则一直等待 
		xReturn=xTaskNotifyWait(0x0,               //进入函数的时候不清除任务 bit
		                ULONG_MAX,         //退出函数的时候清除所有的 bitR 
										&r_data,          //保存任务通知值 
		                portMAX_DELAY);    //阻塞时间 
		if(xReturn==pdTRUE)
		{
			r_tempdata|=r_data;
			if(r_tempdata== (KEY1_EVENT|KEY2_EVENT))
			{
				printf("LED_Task 任务通知获取成功!\n");
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
			printf ( "KEY1 被按下\n" );
			xTaskNotify(LED_Task_Handle,KEY1_EVENT,eSetBits);
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			printf ( "KEY2 被按下\n" );
			xTaskNotify(LED_Task_Handle,KEY2_EVENT,eSetBits);
		}
		vTaskDelay(20); 
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	taskENTER_CRITICAL();           //进入临界区
	
	xReturn=xTaskCreate((TaskFunction_t	)LED_Task,		//任务函数
															(const char* 	)"LED_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&LED_Task_Handle);/* 任务控制块指针 */ 
															
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("LED_Task任务创建成功!\n");
	else
		printf("LED_Task任务创建失败!\n");
	

	xReturn=xTaskCreate((TaskFunction_t	)KEY_Task,		//任务函数
															(const char* 	)"KEY_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)4, 	//任务优先级
															(TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("KEY_Task任务创建成功!\n");
	else
		printf("KEY_Task任务创建失败!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}


//静态创建任务才需要
///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}



///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}
