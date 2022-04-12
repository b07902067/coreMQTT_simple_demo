# MQTT simple demo


- Preprocessing
    - git clone coreMQTT repository and  build libcoverity_analysis.a
    - Download mosquitto or emqx as an mqtt broker
    - Run broker in localhost:1883
    - Place this directory in the top directory of coreMQTT repository which has been cloned
- Usage
    - `$ cd [coreMQTT Path]/coreMQTT_simple_demo`
    - `$ gcc mytest.c ../lib/libcoverity_analysis.a -o mytest -DMQTT_DO_NOT_USE_CUSTOM_CONFIG=1`
    - `$ ./mytest`

- Reference
    - https://github.com/aws/aws-iot-device-sdk-embedded-C/tree/main/demos/mqtt