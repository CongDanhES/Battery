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
    // Serial.println("TaskDisPlay");
    //batteryDevice.update(Serial);
    // Display::batfr::capacity[0] = Display::BATTERY_TYPE::BATTERY_T30;
    // Display::batfr::percent[0] = random(1, 100);
    // Display::batfr::voltage[0] = random(1, 10000);
    // Display::batfr::current[0] = random(1, 10000);
    // Display::batfr::temperature[0] = random(1, 100);
    // Display::batfr::numberCharge[0] = random(1, 10000);
    batteryDevice.update(Serial1);
    vTaskDelay(1);
  }
}

void TaskData(void *pvParameters){
  while(1){

    //update batfr to display 

    //number
    Display::batfr::capacity[0] = batteryDevice.get.capacity;
    Display::batfr::percent[0] = batteryDevice.get.percent;
    Display::batfr::voltage[0] = batteryDevice.get.voltage;
    Display::batfr::current[0] = batteryDevice.get.current;
    Display::batfr::temperature[0] = batteryDevice.get.temperature;
    Display::batfr::numberCharge[0] = batteryDevice.get.numberCharge;

    // string 
    for (int i = 0; i < 4; i++) {
      Display::batfr::version[0][i] = batteryDevice.get.version[i];
    }
    for (int i = 0; i < 14; i++) {
      Display::batfr::seriNumber[0][i] = batteryDevice.get.seriNumber[i];
    }
    for(int i =0 ; i < 14; i++){
      Display::batfr::cell[0][i] = batteryDevice.get.cell[i];
    }
    Display::batfr::countError[0] = batteryDevice.get.countError;
  
    vTaskDelay(100);
  }
}