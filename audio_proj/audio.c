#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <math.h>

#define HW_REGS_BASE 0xFF200000
#define HW_REGS_SPAN 0x00200000
#define HW_REGS_MASK HW_REGS_SPAN-1

#define AUDIO_PIO_BASE 0x3040

#define PI  3.141592
#define SAMPLE_RATE 48000

void write_to_audio (double freq, volatile unsigned int* h2p_lw_audio_addr);
void musicplay (volatile unsigned int* h2p_lw_audio_addr);

double tone[8] = {
    261.626, 293.655, 329.628, 349.228, 391.992, 440.000, 493.883, 523.251
};

int hex_display;
int dev_hex;

int main(){
    int dev_sw, data, rdata;

    //sw file open
    dev_sw = open("/dev/sw", O_RDWR);
    if(dev_sw<0) { //if open failed
        fprintf(stderr, "Cannot open SW device.\n");
        return 1;
    }

    //hex file open
    dev_hex = open("/dev/hex", O_RDWR);
    if(dev_hex<0){ //if open failed
        fprintf(stderr, "cannot open HEX device.\n");
        return 2;
    }

    //audio part initialize
    volatile unsigned int* h2p_lw_audio_addr = NULL;

    void * virtual_base;
    int fd;

    //open /dev/mem
    if((fd=open("/dev/mem", (O_RDWR|O_SYNC)))==-1){
        printf(" ERROR : could not open /dev/mem.. \n");
        return 1;
    }

    //get virtual addr that maps to physical
    virtual_base = mmap(
        NULL,
        HW_REGS_SPAN,
        (PROT_READ | PROT_WRITE),
        MAP_SHARED,
        fd,
        HW_REGS_BASE
    );

    if(virtual_base == MAP_FAILED){
        printf(" ERROR : mmap() failed... \n");
        close(fd);
        return 1;
    }

    //get virtual addr of AUDIO
    h2p_lw_audio_addr = (unsigned int*)(virtual_base+(AUDIO_PIO_BASE) & (HW_REGS_MASK));

    //first, clear the WRITE FIFO by using the CW bit in Control Reg
    *(h2p_lw_audio_addr) = 0xC;     //CW=1, CR=1, WE=0, RE=0
    //and need to reset to 0 to use
    *(h2p_lw_audio_addr) = 0x0;

    double freq;
    
    //execute part
    while(1){
        read(dev_sw, &rdata, 4);

        switch(rdata){
            case 0x01:
                freq = tone[0];
                hex_display = 0;
                break;
            case 0x02:
                freq = tone[1];
                hex_display = 1;
                break;
            case 0x04:
                freq = tone[2];
                hex_display = 2;
                break;
            case 0x08:
                freq = tone[3];
                hex_display = 3;
                break;
            case 0x10:
                freq = tone[4];
                hex_display = 4;
                break;
            case 0x20:
                freq = tone[5];
                hex_display = 5;
                break;
            case 0x40:
                freq = tone[6];
                hex_display = 6;
                break;
            case 0x80:
                freq = tone[7];
                hex_display = 7;
                break;

            case 0x200:         //music func output
                musicplay(h2p_lw_audio_addr);
                hex_display = 7;
                sleep(1);
                break;
            
            default:
                hex_display = 7;
                wrtite(dev_hex, &hex_display, 4);
        }
        if(hex_display != 7){
            write_to_audio(freq, h2p_lw_audio_addr);
            sleep(1);
        }
    }//while(1)

    //close
    close(dev_sw);
    return 0;
}

//주파수, audio port의 주소를 받아 출력하는 함수
void write_to_audio(double freq,volatile unsigned int* h2p_lw_audio_addr){
    usleep(10000);
    //write data in LeftData & RightData
    int nth_sample;

    write(dev_hex, &hex_display, 4);

    //Max volume when multiplied by sin() which ranges from -1 to 1
    int vol = 0x3FFFFFFF;

    for(nth_sample = 0; nth_sample<SAMPLE_RATE*5; nth_sample++){
        *(h2p_lw_audio_addr+2) = vol * sin(nth_sample*freq*2*PI / (SAMPLE_RATE*5));
        *(h2p_lw_audio_addr+3) = vol * sin(nth_sample*freq*2*PI/(SAMPLE_RATE*5));
    }
}

//twinkle twinkle little star
void musicplay(volatile unsigned int* h2p_lw_audio_addr){
    write_to_audio(tone[0], h2p_lw_audio_addr); //do
    write_to_audio(tone[0], h2p_lw_audio_addr); //do
    write_to_audio(tone[4], h2p_lw_audio_addr); //sol
    write_to_audio(tone[4], h2p_lw_audio_addr); //sol
    write_to_audio(tone[5], h2p_lw_audio_addr);
    write_to_audio(tone[5], h2p_lw_audio_addr);
    write_to_audio(tone[4], h2p_lw_audio_addr);
    usleep(500000);

    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr); 
    write_to_audio(tone[1], h2p_lw_audio_addr);
    write_to_audio(tone[1], h2p_lw_audio_addr);
    write_to_audio(tone[0], h2p_lw_audio_addr);
    usleep(500000);

    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr);
    write_to_audio(tone[2], h2p_lw_audio_addr);
    write_to_audio(tone[1], h2p_lw_audio_addr);
    usleep(500000);

    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr);
    write_to_audio(tone[2], h2p_lw_audio_addr);
    write_to_audio(tone[1], h2p_lw_audio_addr);
    usleep(500000);

    write_to_audio(tone[0], h2p_lw_audio_addr); 
    write_to_audio(tone[0], h2p_lw_audio_addr); 
    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[4], h2p_lw_audio_addr); 
    write_to_audio(tone[5], h2p_lw_audio_addr);
    write_to_audio(tone[5], h2p_lw_audio_addr);
    write_to_audio(tone[4], h2p_lw_audio_addr);
    usleep(500000);

    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[3], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr); 
    write_to_audio(tone[2], h2p_lw_audio_addr); 
    write_to_audio(tone[1], h2p_lw_audio_addr);
    write_to_audio(tone[1], h2p_lw_audio_addr);
    write_to_audio(tone[0], h2p_lw_audio_addr);
    usleep(500000);

}


