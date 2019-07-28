#include "esp_camera.h"
#include <WiFi.h>
//#include "soc/soc.h"
//#include "soc/rtc_cntl_reg.h"
//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//


#define CAMERA_MODEL_AI_THINKER

const char* ssid = "U CAN'T SEE ME!!!";
const char* password = "02092001";





// defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27 

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//extra added
#define GPIO_PIN_WAKEUP GPIO_NUM_12//
#define EXT_WAKEUP_PIN_BITMASK 0x1000  //  2^12
#define MINIMUM_WAKE_PERIOD_MILLIS 10e3//

volatile unsigned long lastWakeupPinHigh = 0;//

void gotoSleep() {
  Serial.println("Deep sleep enabled");
  esp_sleep_enable_ext1_wakeup(EXT_WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
  Serial.print("hi");
}
void startCameraServer();

unsigned long secs() {
  return millis() / 1e3L;
}


void loop() {
 
  unsigned long now=millis();
  int wakeupPinState=digitalRead(GPIO_PIN_WAKEUP);
  if(wakeupPinState==HIGH){
    lastWakeupPinHigh=now;
  } else {
    if(now-lastWakeupPinHigh >= MINIMUM_WAKE_PERIOD_MILLIS){
      gotoSleep();
      
  }
  
}
 if(now % 1000 ==0){
    Serial.printf("%u wakeupPinState %u , \n", secs(), wakeupPinState);
  }

}
void setup() {
//  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();//

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  pinMode(GPIO_PIN_WAKEUP, INPUT);
  lastWakeupPinHigh = millis();
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}
