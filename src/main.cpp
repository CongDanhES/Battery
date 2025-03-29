#include <defined.hpp>

TaskHandle_t TaskBattery_Handler;
TaskHandle_t TaskData_Handler;

static void TaskBattery( void *pvParameters );
static void TaskData( void *pvParameters );

void setup(){

  //dji battery
  batteryDevice.init();

  // debug
  Serial.begin(115200);

  // init Display
  displayDevice.init();

  // Create Task score 
  xTaskCreatePinnedToCore(TaskBattery, "TaskDisplay", 4096, NULL, 1, &TaskBattery_Handler, 1 );
  xTaskCreatePinnedToCore(TaskData, "TaskDataHandler", 4096, NULL, 1, &TaskData_Handler, 1);

}

void loop(){

}

void TaskBattery(void *pvParameters){
  while(1){

    // update battery to get data
    batteryDevice.update(Serial1);
    vTaskDelay(1);

  }
}

void TaskData(void *pvParameters){
  //update batfr to display 
  while(1){

    //update number
    Display::batfr::capacity = batteryDevice.get.capacity;
    Display::batfr::percent = batteryDevice.get.percent;
    Display::batfr::voltage = batteryDevice.get.voltage;
    Display::batfr::current = batteryDevice.get.current;
    Display::batfr::temperature = batteryDevice.get.temperature;
    Display::batfr::numberCharge = batteryDevice.get.numberCharge;

    // update string 
    for (int i = 0; i < 4; i++) {
      Display::batfr::version[i] = batteryDevice.get.version[i];
    }
    for (int i = 0; i < 14; i++) {
      Display::batfr::seriNumber[i] = batteryDevice.get.seriNumber[i];
    }
    for(int i =0 ; i < 14; i++){
      Display::batfr::cell[i] = batteryDevice.get.cell[i];
    }
    Display::batfr::countError = batteryDevice.get.countError;
    if(Display::batfr::countError >= 195){
      Display::batfr::reset();
      batteryDevice.get.capacity = 0;
      batteryDevice.get.percent = 0;
      batteryDevice.get.voltage = 0;
      batteryDevice.get.current = 0;
      batteryDevice.get.temperature = 0;
      batteryDevice.get.numberCharge = 0;
      for (int i = 0; i < 4; i++) {
        batteryDevice.get.version[i] = 0;
      }
      for (int i = 0; i < 14; i++) {
        batteryDevice.get.seriNumber[i] = 0;
      }
      for(int i =0 ; i < 14; i++){
        batteryDevice.get.cell[i] = 0;
      }
      // batteryDevice.get.countError = 0;
    }
  
    vTaskDelay(100);
  }
}