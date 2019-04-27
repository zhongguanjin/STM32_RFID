
/*******************************
********时间片轮询法的架构******
********************************/
#include "Task_Main.h"

#include "stdio.h"

#include "dbg.h"
#include "Rfid.h"
#include "ml_fpm.h"


void Taskpro(void);


// 定义结构体变量
static TASK_COMPONENTS TaskComps[] =
{
    {0, 50, 50, Taskpro},//500ms
};
// 任务清单
typedef enum _TASK_LIST
{

    TAST_PRO,             //
    TASKS_MAX                // 总的可供分配的定时任务数目
} TASK_LIST;






/*****************************************************************************
 函 数 名  : Taskpro
 功能描述  : 进程任务
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月25日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Taskpro(void)
{
    //Rfid_Task_Process();
    mlst_task();
}

/*****************************************************************************
 函 数 名  : TaskRemarks
 功能描述  : 任务标记处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月24日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/

void TaskRemarks(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)                                 // 逐个任务时间处理
    {
         if (TaskComps[i].Timer)                                // 时间不为0
        {
            TaskComps[i].Timer--;                                // 减去一个节拍
            if (TaskComps[i].Timer == 0)                            // 时间减完了
            {
                 TaskComps[i].Timer = TaskComps[i].ItvTime;       // 恢复计时器值，从新下一次
                 TaskComps[i].Run = ON;                             // 任务可以运行
            }
        }
   }
}

/*****************************************************************************
 函 数 名  : TaskProcess
 功能描述  : 任务进程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月24日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskProcess(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)                // 逐个任务时间处理
    {
        if (TaskComps[i].Run)                 // 时间不为0
        {
             TaskComps[i].TaskHook();        // 运行任务
             TaskComps[i].Run = 0;          // 标志清0
        }
    }
}

