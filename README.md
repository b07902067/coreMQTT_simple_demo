# MQTT simple demo


- Preprocessing
    - git clone coreMQTT  and  build libcoverity_analysis.a
    - Download mosquitto or emqx as an mqtt broker
    - Run broker in localhost:1883
    - Place this directory in the top directory of coreMQTT repository which was cloned in the desktop
- Usage
    - `$ gcc mytest.c ../lib/libcoverity_analysis.a -o mytest -DMQTT_DO_NOT_USE_CUSTOM_CONFIG=1`
    - `$ ./mytest`