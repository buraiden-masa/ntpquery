# ntpquery
Send a sample query to the NTP server and receive a response 

# How to use
First compile the program and give it to the IP address of the NTP server as an argument.<br>
rxTm is the timestamp when the NTP server received the ntp packet.<br>
txTm is the timestamp when the NTP server transmit the ntp packet.<br>
The number on the left indicates Unix time :
```
# gcc ntpquery.c -o ntpquery
# ./ntpquery xxx.xxx.xxx.xxx
[xxx.xxx.xxx.xxx] rxTm: 1641293009.374749 .. Tue Jan  4 19:43:29 2022
[xxx.xxx.xxx.xxx] txTm: 1641293009.374868 .. Tue Jan  4 19:43:29 2022
```
