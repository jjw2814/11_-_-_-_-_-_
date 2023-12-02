# 동아리방 IOT 시스템

## 개발자
21800677 정지우   
21900059 김가륜   
22001020 박석환

## 기능 개요
이 프로젝트는 다양한 기기들을 연동하여 홈 자동화 및 데이터 수집을 위한 것입니다. 아래는 프로젝트의 각 부분에 대한 간단한 설명과 사용된 언어 및 라이브러리입니다.

## Requirements
- NodeMCU(ESP8266)
- RaspberryPi 4
- ESP32-CAM-MB
- DHT22
- Adafruit_SSD1306
- Home Assistant
- PIR Sensor(HC-SR501)
- 국토교통부_(TAGO)_버스도착정보 API
- Gas Sensor(MQ-7)

## Repository 개요
 - Docs : 해당 프로젝트의 보고서를 포함하는 저장소입니다.
 - Design : 해당 프로젝트를 구현하는데 사용된 소스코드들을 포함하는 저장소입니다.
 - Design/Arduino : 소스코드 중 Arduino 코드를 포함하는 저장소입니다.
 - Design/HomeAssistant : 소스코드 중 Home Assistant를 구동하는데 사용되는 yaml 파일 및 python 파일을 포함하는 저장소입니다.

## 코드 별 설명
### 1. CameraWebServer.ino (언어: Arduino, 라이브러리: esp_camera, WiFi)

이 코드는 ESP32 기반 카메라 모듈을 사용하여 웹 서버를 구축하는 코드입니다. 카메라에서 캡처한 영상을 스트리밍하고, WiFi를 통해 웹 페이지에 접근할 수 있도록 합니다.

### 2. project.ino (언어: Arduino, 라이브러리: DHTesp, PubSubClient, Adafruit_SSD1306, WiFiManager)

이 코드는 DHT22 온습도 센서 및 가스 센서와 연동하여 데이터를 수집하고, MQTT 프로토콜을 사용하여 데이터를 전송합니다. 또한 OLED 디스플레이를 사용하여 센서 데이터를 표시하고, WiFiManager를 통해 WiFi 및 MQTT 설정을 관리합니다.

### 3. automations.yaml

이 YAML 파일은 Home Assistant에서 사용되는 자동화 설정입니다. 움직임 감지 시 조명을 켜거나 끄는 등의 작업을 정의합니다.

### 4. bus_info.py (언어: Python, 라이브러리: requests, json)

이 Python 스크립트는 공공데이터포털에서 제공하는 버스 도착 정보 API를 활용하여 버스 도착 정보를 가져오고, MQTT를 통해 Home Assistant에 전송합니다.

### 5. configuration.yaml

이 YAML 파일은 Home Assistant의 전반적인 설정을 담고 있습니다. MQTT 센서 설정, Home Assistant의 customization 및 Lovelace 설정 등이 포함되어 있습니다.

프로젝트 전체의 흐름은 ESP32 카메라 모듈에서 데이터를 수집하고, MQTT를 통해 Home Assistant로 전송하여 자동화 및 시각화를 구현하는 것입니다.

### 6. ui-lovelace.yaml

이 yaml 파일은 Home Assistant의 ui를 설정하는 파일입니다. 각각의 Card의 종류 및 포함할 정보를 수정할 수 있습니다.
