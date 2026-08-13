/* FreeRTOS*/
#include "sys_config.h"

#if SYS_configUSE_FREERTOS

#include "taskmain.h"
#include "lv_api_map.h"
#include "bsp_adc.h"

/* ========== 任务句柄定义 ========== */
static TaskHandle_t xLVGLGUITaskHandle = NULL;
static TaskHandle_t xGetTempTaskHandle = NULL;

/* ========== 任务函数声明 ========== */
void LVGL_GUI_TASK(void *pvParameters);
void Get_Temp_TASK(void *pvParameters);

/* ========== 任务配置数组 ========== */
static const TaskConfig_t xTaskConfigTable[] = 
{
    /* 任务函数      任务名称        栈大小  参数    优先级  句柄 */
#if SYS_configUSE_LVGL
    {LVGL_GUI_TASK,      "LVGL_GUI_Task",     4096,   NULL,   3,      &xLVGLGUITaskHandle},
#endif
    {Get_Temp_TASK,      "Get_Temp_Task",     256,    NULL,   2,      &xGetTempTaskHandle},
};

/* 任务数量 */
#define TASK_COUNT  (sizeof(xTaskConfigTable) / sizeof(TaskConfig_t))
extern float g_temperature;


void taskmainInit(void)
{
    BaseType_t xReturn;
    uint8_t i;
    
    /* 遍历任务配置表，批量创建任务 */
    for(i = 0; i < TASK_COUNT; i++)
    {
        xReturn = xTaskCreate(
            xTaskConfigTable[i].task_func,
            xTaskConfigTable[i].task_name,
            xTaskConfigTable[i].stack_size,
            xTaskConfigTable[i].task_param,
            xTaskConfigTable[i].priority,
            xTaskConfigTable[i].task_handle
        );
        
        if(xReturn != pdPASS)
        {
            /* 任务创建失败处理 */
            while(1);  // 创建失败，停止系统
        }
    }
    
    /* 启动调度器 */
    vTaskStartScheduler();
}

#if SYS_configUSE_LVGL
void LVGL_GUI_TASK(void *pvParameters)
{
    uint32_t lvgl_delay = 1;
    while (1)
    {
        lvgl_delay = lv_task_handler();
        vTaskDelay(lvgl_delay);
    }
}
#endif

void Get_Temp_TASK(void *pvParameters)
{
    while (1)
    {
        g_temperature = Get_temperature();
        vTaskDelay(1000);
    }
}

/* Hook function called by FreeRTOS when the system is idle */
void vApplicationIdleHook(void)
{
    fwdgt_counter_reload();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    taskDISABLE_INTERRUPTS();
    while(1)
    {
    }
}

#else
void vApplicationIdleHook(void)
{
    /* 空实现 */
}

#endif