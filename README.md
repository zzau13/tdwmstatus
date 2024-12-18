# tiny/turbo/throttling dwm status bar 

Test in linux kernel 6.11.*

## Install
```
$ git clone https://github.com/zzau13/tdwmstatus.git
$ cd tdwmstatus
$ make 
$ sudo make install
```
## Shot
![Shot](shot.png)

## Description
Simple status bar for dwm. 
Return in order:
- loadavg
- percentage use 
  - cpu 
  - ram 
  - /home
- cpu temp
- percentage battery 
- battery status 
- volume
- uptime 
- date

For more info http://dwm.suckless.org/dwmstatus/
