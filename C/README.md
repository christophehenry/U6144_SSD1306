# U6144_ssd1306
## Enable i2c 
```bash
sudo raspi-config
```
Choose Interface Options 
Enable i2c

##  Download library 
```bash
git clone https://github.com/UCTRONICS/U6144_SSD1306.git
```
## Compile the source code 
```bash
cd U6144_SSD1306/C 
```
```bash
make 
```


## Run the display

```bash 
cd U6144_SSD1306/C 
```
```bash 
./display 
```

## Install

```bash
make
make install
```

This will compile, install the gennerated binary under `/usr/bin/uc_display` and installl and activate a systemd service.



