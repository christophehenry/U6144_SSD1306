#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include "ssd1306_i2c.h"
#include "bmp.h"
#include "oled_fonts.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

#define FIRST_LINE 3
#define SECOND_LINE 5
#define THIRD_LINE 7

#define MAX_LEN 20
#define FORMAT(dest, ...) sprintf(dest, "%-*s", MAX_LEN, __VA_ARGS__)

char IPSource[MAX_LEN + 1] = {0};
int i2cd;

// Init SSD1306
void ssd1306_begin(unsigned int vccstate, unsigned char i2caddr) {
    unsigned char i2c[20] = "/dev/i2c-1";
    // I2C Init
    i2cd = open(i2c, O_RDWR);//"/dev/i2c-1"
    if (i2cd < 0) {
        fprintf(stderr, "Device I2C-1 failed to initialize\n");
        return;
    }
    if (ioctl(i2cd, I2C_SLAVE_FORCE, i2caddr) < 0) {
        return;
    }

    OLED_WR_Byte(0xAE, OLED_CMD);//--display off
    OLED_WR_Byte(0xAE, OLED_CMD);//--display off
    OLED_WR_Byte(0x00, OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10, OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40, OLED_CMD);//--set start line address
    OLED_WR_Byte(0xB0, OLED_CMD);//--set page address
    OLED_WR_Byte(0x81, OLED_CMD); // contract control
    OLED_WR_Byte(0xFF, OLED_CMD);//--128
    OLED_WR_Byte(0xA1, OLED_CMD);//set segment remap
    OLED_WR_Byte(0xA6, OLED_CMD);//--normal / reverse
    OLED_WR_Byte(0xA8, OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F, OLED_CMD);//--1/32 duty
    OLED_WR_Byte(0xC8, OLED_CMD);//Com scan direction
    OLED_WR_Byte(0xD3, OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00, OLED_CMD);//

    OLED_WR_Byte(0xD5, OLED_CMD);//set osc division
    OLED_WR_Byte(0x80, OLED_CMD);//

    OLED_WR_Byte(0xD8, OLED_CMD);//set area color mode off
    OLED_WR_Byte(0x05, OLED_CMD);//

    OLED_WR_Byte(0xD9, OLED_CMD);//Set Pre-Charge Period
    OLED_WR_Byte(0xF1, OLED_CMD);//

    OLED_WR_Byte(0xDA, OLED_CMD);//set com pin configuartion
    OLED_WR_Byte(0x12, OLED_CMD);//

    OLED_WR_Byte(0xDB, OLED_CMD);//set Vcomh
    OLED_WR_Byte(0x30, OLED_CMD);//

    OLED_WR_Byte(0x8D, OLED_CMD);//set charge pump enable
    OLED_WR_Byte(0x14, OLED_CMD);//

    OLED_WR_Byte(0xAF, OLED_CMD);//--turn on oled panel
}


//Displays a string of characters
void OLED_ShowString(unsigned char x, unsigned char y, unsigned char *chr, unsigned char Char_Size) {
    unsigned char j = 0;
    int max = MAX_LEN * 8;
    while (chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > max) {
            x = 0;
            y += 2;
        }
        j++;
    }
}

void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char Char_Size) {
    unsigned char c = 0, i = 0;
    c = chr - ' ';    //Get the offset value
    if (x > SSD1306_LCDWIDTH - 1) {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16) {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
    } else {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_Byte(F6x8[c][i], OLED_DATA);

    }
}

unsigned int oled_pow(unsigned char m, unsigned char n) {
    unsigned int result = 1;
    while (n--)result *= m;
    return result;
}

//According to digital
//x,y :Starting point coordinates	 
//len :Number of digits
//size:The font size
//mode:	0,Fill mode;1,Stacking patterns
//num:(0~4294967295);	 		  
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len, unsigned char size2) {
    unsigned char t, temp;
    unsigned char enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x + (size2 / 2) * t, y, ' ', size2);
                continue;
            } else enshow = 1;

        }
        OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0', size2);
    }
}


//Coordinate setting
void OLED_Set_Pos(unsigned char x, unsigned char y) {
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f), OLED_CMD);
}

//Write a byte
void OLED_WR_Byte(unsigned dat, unsigned cmd) {

    if (cmd) {
        Write_IIC_Data(dat);
    } else {
        Write_IIC_Command(dat);
    }

    usleep(500);
}

//To send data
void Write_IIC_Data(unsigned char IIC_Data) {
    unsigned char msg[2] = {0x40, 0};
    msg[1] = IIC_Data;
    write(i2cd, msg, 2);
}

//Send the command
void Write_IIC_Command(unsigned char IIC_Command) {
    unsigned char msg[2] = {0x00, 0};
    msg[1] = IIC_Command;
    write(i2cd, msg, 2);
}


/***********Display the BMP image  128X64  Starting point coordinates(x,y),The range of x 0~127   The range of y 0~4*****************/
void OLED_DrawBMP() {
    unsigned char x0 = 0;
    unsigned char y0 = 0;
    unsigned char x1 = 128;
    unsigned char y1 = 8;
    unsigned int j = 0;
    unsigned char x, y;

    if (y1 % 8 == 0) y = y1 / 8;
    else y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++) {
            OLED_WR_Byte(BMP[j++], OLED_DATA);
        }
    }
}


void OLED_DrawPartBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[1024]) {
    unsigned int j = x1 * y0;
    unsigned char x, y;

    if (y1 % 8 == 0) y = y1 / 8;
    else y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++) {
            OLED_WR_Byte(BMP[j++], OLED_DATA);
        }
    }
}

/*
*	Clear specified row
*/
void OLED_ClearLint(unsigned char x1, unsigned char x2) {
    unsigned char i, n;
    for (i = x1; i < x2; i++) {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);    //Set page address
        OLED_WR_Byte(0x00, OLED_CMD);      //Sets the display location - column low address
        OLED_WR_Byte(0x10, OLED_CMD);      //Sets the display location - column high address
        for (n = 0; n < 128; n++)OLED_WR_Byte(0, OLED_DATA);
    }
}

void OLED_Clear(void) {
    unsigned char i, n;
    for (i = 0; i < 8; i++) {
        OLED_WR_Byte(0xb0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);
        for (n = 0; n < 128; n++)OLED_WR_Byte(0, OLED_DATA);
    }
}

/*
*    LCD displays CPU temperature and other information
*/
void LCD_DisplayTemperature(void) {
    unsigned int temp = Obaintemperature();
    FILE *fp;
    unsigned char buffer[80] = {0};
    fp = popen("top -bn1 | grep load | awk '{printf \"%.2f\", $(NF-2)}'", "r");    //Gets the load on the CPU
    fgets(buffer, sizeof(buffer), fp);                                    //Read the CPU load
    pclose(fp);
    buffer[3] = '\0';

    GetIpAddress();   //Get the IP address of the device's wireless network card
    OLED_ShowString(0, 0, IPSource, 8);          //Send the IP address to the lower machine

    if (temp >= 100) {
        OLED_ShowChar(41, FIRST_LINE, temp / 100 + '0', 8);                        //According to the temperature
        OLED_ShowChar(50, FIRST_LINE, temp / 10 % 10 + '0', 8);                        //According to the temperature
        OLED_ShowChar(59, FIRST_LINE, temp % 10 + '0', 8);
    } else if (temp < 100 && temp >= 10) {
        OLED_ShowChar(50, FIRST_LINE, temp / 10 + '0', 8);                        //According to the temperature
        OLED_ShowChar(59, FIRST_LINE, temp % 10 + '0', 8);
    } else {
        OLED_ShowChar(59, FIRST_LINE, temp + '0', 8);
    }
    OLED_ShowString(92, FIRST_LINE, buffer, 8);                        //Display CPU load
}

unsigned int Obaintemperature(void) {
    FILE *fd;
    unsigned int temp = 0;
    char buff[256] = {0};
    fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%d", &temp);
    fclose(fd);
    return temp / 1000;

}

/*
*  LCD displays CPU memory and  SD card memory other information
*/
void LCD_DisPlayCpuSdMemory(void) {
    struct sysinfo s_info;
    float totalram = 0.0;
    float usedram = 0.0;
    unsigned char buffer[100] = {0};
    unsigned char famer[100] = {0};
    unsigned char Total[10] = {0};
    unsigned char free[10] = {0};
    char usedsize_GB[10] = {0};
    char totalsize_GB[10] = {0};
    unsigned int MemSize = 0;
    unsigned int size = 0;
    unsigned int value = 0;
    struct statfs diskInfo;
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        return;
    }
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (sscanf(buffer, "%s%u", famer, &value) != 2) {
            continue;
        }
        if (strcmp(famer, "MemTotal:") == 0) {
            totalram = value;
        } else if (strcmp(famer, "MemFree:") == 0) {
            usedram = value;
        }
    }
    fclose(fp);

    usedram = (totalram - usedram) / 1024.0 / 1024.0;
    totalram = totalram / 1024.0 / 1024.0;

    Total[0] = (unsigned char) totalram + '0';
    Total[1] = '.';
    Total[2] = ((unsigned char) (totalram * 10)) % 10 + '0';
    free[0] = (unsigned char) usedram + '0';
    free[1] = '.';
    free[2] = ((unsigned char) (usedram * 10)) % 10 + '0';
    OLED_ShowString(50, SECOND_LINE, free, 8);
    OLED_ShowString(88, SECOND_LINE, Total, 8);


    statfs("/", &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;// The number of bytes per block
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;//Total number of bytes
    MemSize = (unsigned int) (totalsize >> 30);
    snprintf(totalsize_GB, 7, "%d", MemSize);
    if (MemSize > 0 && MemSize < 10) {
        OLED_ShowString(104, THIRD_LINE, totalsize_GB, 8);
    } else if (MemSize >= 10 && MemSize < 100) {
        OLED_ShowString(96, THIRD_LINE, totalsize_GB, 8);
    } else {
        OLED_ShowString(88, THIRD_LINE, totalsize_GB, 8);
    }


    unsigned long long freesize = blocksize * diskInfo.f_bfree; //Now let's figure out how much space we have left
    size = freesize >> 30;
    size = MemSize - size;
    snprintf(usedsize_GB, 7, "%d", size);
    if (size > 0 && size < 10) {
        OLED_ShowString(60, THIRD_LINE, usedsize_GB, 8);
    } else if (size >= 10 && size < 100) {
        OLED_ShowString(53, THIRD_LINE, usedsize_GB, 8);
    } else {
        OLED_ShowString(50, THIRD_LINE, usedsize_GB, 8);
    }
}

/*
*According to the information
*/
void LCD_Display() {
    LCD_DisplayTemperature();
    LCD_DisPlayCpuSdMemory();
}

void GetIpAddress(void) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "wlan0" */
    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ - 1);

    int symbol = ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    if (symbol == 0) FORMAT(IPSource, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
    else FORMAT(IPSource, "No IPv4 conn.");
}
