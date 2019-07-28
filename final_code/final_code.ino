#include "esp_camera.h"
#include <WiFi.h>
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"

#define LEDPin 2 
#define LEDin 4
unsigned long currentMillis = 0;
unsigned long openedMillis = 0;
long interval = 5000;           

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7



//esp pins definition....
#define GPIO_PIN_WAKEUP GPIO_NUM_12
#define EXT_WAKEUP_PIN_BITMASK 0x1000  //  2^12
#define MINIMUM_WAKE_PERIOD_MILLIS 10e3

volatile unsigned long lastWakeupPinHigh = 0;


void gotoSleep() {
  Serial.println("Deep sleep enabled");
  esp_sleep_enable_ext1_wakeup(EXT_WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
  Serial.print("hi");
}//defining sleep fn.

void startCameraServer();

unsigned long secs() {
  return millis() / 1e3L;
}



mtmn_config_t init_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.7;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 4;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.4;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}

static face_id_list id_list = {0};
dl_matrix3du_t *image_matrix =  NULL;
camera_fb_t * fb = NULL;
mtmn_config_t mtmn_config = init_config();
dl_matrix3du_t *aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);


void startCameraServer();



void setup() {
  Serial.begin(115200);
   Serial.setDebugOutput(true);
  Serial.println();
  
 digitalWrite(LEDin, LOW);
   
  pinMode(LEDin, OUTPUT);

  digitalWrite(LEDPin, LOW);
  pinMode(LEDPin, OUTPUT);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  read_face_id_from_flash(&id_list);// Read current face data from on-board flash
}

void rzoCheckForFace() {
  currentMillis = millis();
  if (run_face_recognition()) { 
    Serial.println("Face recognised");
    digitalWrite(LEDin,LOW);
    digitalWrite(LEDPin, HIGH); 
    openedMillis = millis(); 
  }
  if (currentMillis - openedMillis >interval){ // current time - face recognised time > 5 secs
    digitalWrite(LEDPin, LOW); //open LED
  }
}

bool run_face_recognition() {
  bool faceRecognised = false; 
  int64_t start_time = esp_timer_get_time();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }

  int64_t fb_get_time = esp_timer_get_time();
  Serial.printf("Get one frame in %u ms.\n", (fb_get_time - start_time) / 1000); // this line can be commented out
  
  image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  uint32_t res = fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
  if (!res) {
    Serial.println("to rgb888 failed");
    dl_matrix3du_free(image_matrix);
  }

  esp_camera_fb_return(fb);

  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

  if (net_boxes) {
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK) {

      digitalWrite(LEDin, HIGH);
      int matched_id = recognize_face(&id_list, aligned_face);
      if (matched_id >= 0) {
        Serial.printf("Match Face ID: %u\n", matched_id);
        faceRecognised = true; 
      } else {
        Serial.println("No Match Found");
         
  
        matched_id = -1;
      }
    } else {
      Serial.println("Face Not Aligned");
      
    }

    free(net_boxes->box);
    free(net_boxes->landmark);
    free(net_boxes);
  }

  dl_matrix3du_free(image_matrix);
  return faceRecognised;
}

void loop() {
  unsigned long now=millis();
  int wakeupPinState=HIGH;//digitalRead(GPIO_PIN_WAKEUP);
  if(wakeupPinState==HIGH){
    lastWakeupPinHigh=now;
    rzoCheckForFace();
  } else {
    if(now-lastWakeupPinHigh >= MINIMUM_WAKE_PERIOD_MILLIS){
      gotoSleep();
      
  }
  
}  
  if(now % 1000 ==0){
    Serial.printf("%u wakeupPinState %u , time %u \n", secs(), wakeupPinState, now);
  }
}
