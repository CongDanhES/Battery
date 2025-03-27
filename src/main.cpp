#include <defined.hpp>

TaskHandle_t TaskDisPlayBat_Handler;
TaskHandle_t TaskData_Handler;

static void TaskDisPlay( void *pvParameters );
static void TaskData( void *pvParameters );

void setup(){

  // init Display
  displayDevice.init();

  xTaskCreatePinnedToCore(TaskDisPlay, "TaskDisplay", 4096, NULL, 1, &TaskDisPlayBat_Handler, 0);
  xTaskCreatePinnedToCore(TaskData, "TaskDataHandler", 4096, NULL, 1, &TaskData_Handler, 1);

}

void loop(){

}

void TaskDisPlay(void *pvParameters){
  while(1){

  }
}

void TaskData(void *pvParameters){
  while(1){

  }
}