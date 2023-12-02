# Arduino Setting for Sensor part

## 필요한 보드 메니저:
- esp8266

## 필요한 라이브러리:
- ESP8266WebServer
- Adafruit_SSD1306
- WiFiManager
- FS
- PubSubClient
- DHTesp

## 센서 및 엑추에이터 핀 연결:
- LED : D0
- GAS (MQ-7) : A0
- DHT22 : D3
- PIR : D5
- SDD1306 : D1(SCL) D2(SDA)

# Arduino Setting for ESP32-CAM-MB

필요한 보드 매니저 : AI Thinker ESP32-CAM

1. Arduino IDE의 개인 설정에서 추가적인 보드 매니저 URLS에 https://dl.espressif.com/dl/package_esp32_index.json를 추가한다.
2. 보드매니저 검색 창에 ESP32를 검색하여 ESP32 by Espressif Systems를 설치한다.
3. ESP32 보드 중 AI Thinker ESP32-CAM을 선택하고, 사용중인 포트와 연결한다.
4. CameraWebServer.ino 파일을 열고, 사용중인 WiFi의 SSID와 Password를 입력하여 컴파일한다.
5. 시리얼 모니터에 출력되는 로컬 IP 주소를 활용하여 configuration 파일의 panel_iframe url에 입력한다.

# GPSLogger

Home Assistant의 Devices & Service에서 GPSLogger Integration을 추가하고, 추가 시 나오는 WebHook URL을 복사해둔다.
개발자의 깃허브(https://github.com/mendhak/gpslogger/releases)에서 안드로이드용 애플리케이션을 설치한다.
https://www.home-assistant.io/integrations/gpslogger의 설명에 따라 설정을 진행한다.
Home Assistant 모바일 애플리케이션에서 Settings - Companion App - 센서 관리에 들어가 위치 센서 항목들을 전부 활성화해준다.
