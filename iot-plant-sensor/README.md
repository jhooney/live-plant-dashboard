
### General Description of Project Hardware
The iot devices (of which I am building a dozen to start) are fairly simple and compose of the monk makes plant monitor: https://monkmakes.com/pmon
and the Seeed Studio XIAO ESP32C3. The plant monitor is recording humidity, temperature, and light. The esp32 pulls the plant monitor every minute for data, and then report that do a prometheus database running on a home server (which is just a Beelink mini PC on our home wifi)
