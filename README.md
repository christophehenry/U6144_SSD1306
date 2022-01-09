# U6144_SSD1306

This is an heavily modified version of [UCTRONICS/U6144_SSD1306](https://github.com/UCTRONICS/U6144_SSD1306.git)

Those are the modifications to the original library:

- display temperatur in celsius degrees
- display all the informations on one screen

## C code
-  Enable i2c 
```bash

echo dtparam=i2c_arm=on >> /boot/config-user.txt
echo i2c_dev >> /etc/modules
```

- Download library 
```bash
git clone https://github.com/christophehenry/U6144_SSD1306
```
- Compile the source code 
```bash
cd U6144_SSD1306/C
make clean
make
```

- Run the display

```bash 
cd U6144_SSD1306/C
./display 
```

## Add automatic start script
- Open the rc.local file
```bash
sudo nano /etc/rc.local
```
- Add command to the rc.local file
```bash
`cd /home/pi/U6144_SSD1306/C 
sudo make clean 
sudo make 
sudo ./display &`
```
- reboot your system