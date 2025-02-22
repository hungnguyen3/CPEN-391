#include <stdio.h>
#include <time.h>
#include <math.h>

#define SWITCHES    (volatile unsigned int *)(0xFF200000)
#define PUSHBUTTONS (volatile unsigned int *)(0xFF200010)

#define LEDS        (volatile unsigned int *)(0xFF200020)
#define HEX0_1      (volatile unsigned int *)(0xFF200030)
#define HEX2_3      (volatile unsigned int *)(0xFF200040)
#define HEX4_5      (volatile unsigned int *)(0xFF200050)
//----------------------------------------------------------------------------------------------
//the memory address of rs232
#define RS232_ReceiverFifo         				(*(volatile unsigned char *)(0xFF210200))
#define RS232_TransmitterFifo      				(*(volatile unsigned char *)(0xFF210200))
#define RS232_InterruptEnableReg   				(*(volatile unsigned char *)(0xFF210202))
#define RS232_InterruptIdentificationReg        (*(volatile unsigned char *)(0xFF210204))
#define RS232_FifoControlReg                    (*(volatile unsigned char *)(0xFF210204))
#define RS232_LineControlReg                    (*(volatile unsigned char *)(0xFF210206))
#define RS232_ModemControlReg                   (*(volatile unsigned char *)(0xFF210208))
#define RS232_LineStatusReg                     (*(volatile unsigned char *)(0xFF21020A))
#define RS232_ModemStatusReg                    (*(volatile unsigned char *)(0xFF21020C))
#define RS232_ScratchReg                        (*(volatile unsigned char *)(0xFF21020E))
#define RS232_DivisorLatchLSB                   (*(volatile unsigned char *)(0xFF210200))
#define RS232_DivisorLatchMSB                   (*(volatile unsigned char *)(0xFF210202))
//...............................................................................................
// the memory address of wifi
#define WiFi_Offset                        		(volatile unsigned char *)(0x00000010)
#define WiFi_ReceiverFifo                  		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_ReceiverFifo))
#define WiFi_TransmitterFifo               		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_TransmitterFifo))
#define WiFi_InterruptEnableReg            		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_InterruptEnableReg))
#define WiFi_InterruptIdentificationReg    		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_InterruptIdentificationReg))
#define WiFi_FifoControlReg                		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_FifoControlReg))
#define WiFi_LineControlReg                		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_LineControlReg))
#define WiFi_ModemControlReg               		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_ModemControlReg))
#define WiFi_LineStatusReg                 		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_LineStatusReg))
#define WiFi_ModemStatusReg                		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_ModemStatusReg))
#define WiFi_ScratchReg                    		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_ScratchReg))
#define WiFi_DivisorLatchLSB               		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_DivisorLatchLSB))
#define WiFi_DivisorLatchMSB               		(*(volatile unsigned char *)((int)WiFi_Offset + (int)&RS232_DivisorLatchMSB))
//................................................................................................
//the memory address of bluetooth
#define Bluetooth_ReceiverFifo        			((volatile unsigned char *)(0xFF210220))
#define Bluetooth_TransmitterFifo     			((volatile unsigned char *)(0xFF210220))
#define Bluetooth_InterruptEnableReg  			((volatile unsigned char *)(0xFF210222))
#define Bluetooth_InterruptIdentificationReg 	((volatile unsigned char *)(0xFF210224))
#define Bluetooth_FifoControlReg 				((volatile unsigned char *)(0xFF210224))
#define Bluetooth_LineControlReg 				((volatile unsigned char *)(0xFF210226))
#define Bluetooth_ModemControlReg 				((volatile unsigned char *)(0xFF210228))
#define Bluetooth_LineStatusReg 				((volatile unsigned char *)(0xFF21022A))
#define Bluetooth_ModemStatusReg 				((volatile unsigned char *)(0xFF21022C))
#define Bluetooth_ScratchReg 					((volatile unsigned char *)(0xFF21022E))
#define Bluetooth_DivisorLatchLSB 				((volatile unsigned char *)(0xFF210220))
#define Bluetooth_DivisorLatchMSB 				((volatile unsigned char *)(0xFF210222))

void Init_bluetooth(void);
int putcharbluetooth (int , volatile unsigned char *,  volatile unsigned char *);
int getcharbluetooth( volatile unsigned char *,  volatile unsigned char *);
int TestForReceivedData(volatile unsigned char *);
void Flush( volatile unsigned char *, volatile unsigned char * );

void Init_RS232(void) {
	// set bit 7 of Line Control Register to 1, to gain access to the baud rate registers
	RS232_LineControlReg = RS232_LineControlReg | 0x80;
	// set Divisor latch (LSB and MSB) with correct value for required baud rate

	int divisor = (int) ((50E6)/(112500 *16));
	RS232_DivisorLatchLSB = divisor & 0xff;
	RS232_DivisorLatchMSB = (divisor >> 8) & 0xff;


	// set bit 7 of Line control register back to 0 and
	RS232_LineControlReg = RS232_LineControlReg & 0x7F;
	// program other bits in that reg for 8 bit data, 1 stop bit, no parity etc
	RS232_LineControlReg = 0x03;
	// Reset the Fifo’s in the FiFo Control Reg by setting bits 1 & 2
	RS232_FifoControlReg = 0x06;
	// Now Clear all bits in the FiFo control registers
	RS232_FifoControlReg = 0x00;
}

int putcharRS232(int c) {
	// wait for Transmitter Holding Register bit (5) of line status register to be '1'
	// indicating we can write to the device
	while((RS232_LineStatusReg & 0x20) != 0x20);
	// write character to Transmitter fifo register
	RS232_TransmitterFifo = c;
	// return the character we printed
	return c;
}

int getcharRS232( void ) {
	// wait for Data Ready bit (0) of line status register to be '1'
	while((RS232_LineStatusReg & 0x01) != 0x01);
	// read new character from ReceiverFiFo register
	int ans = RS232_ReceiverFifo;
	// return new character
	return ans;
}

// the following function polls the UART to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read from the FIFO
int RS232TestForReceivedData(void) {
	// if RS232_LineStatusReg bit 0 is set to 1
	//return TRUE, otherwise return FALSE
	if((RS232_LineStatusReg & 0x01) == 0x01)
		return 1;
	return 0;
}

//
// Remove/flush the UART receiver buffer by removing any unread characters
//
void RS232Flush(void) { // read til nothing
	// while bit 0 of Line Status Register == ‘1’
	//    read unwanted char out of fifo receiver buffer
    // return; // no more characters so return
	while((RS232_LineStatusReg & 0x01) == 0x01) {
		int temp = RS232_ReceiverFifo;
	}
	return;
}

// delay function to work with RS323
void delay(long cycles)
{
    long pause;
    clock_t now,then;

    pause = cycles;
    //pause = (long)milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

void Init_bluetooth(void) {
	// set bit 7 of Line Control Register to 1, to gain access to the baud rate registers
	unsigned char line_control_register= *Bluetooth_LineControlReg;
	line_control_register = line_control_register |  0x80;
	*Bluetooth_LineControlReg= line_control_register;
	// set Divisor latch (LSB and MSB) with correct value for required baud rate
	int divisor = (int) ((50E6)/(38400 *16));
	*Bluetooth_DivisorLatchLSB = divisor & 0xff;
	*Bluetooth_DivisorLatchMSB = (divisor >> 8) & 0xff;

	// set bit 7 of Line control register back to 0 and
	// program other bits in that reg for 8 bit data,
	// 1 stop bit, no parity etc
	*Bluetooth_LineControlReg= 0x03;
	// Reset the Fifo’s in the FiFo Control Reg by setting bits 1 & 2
	*Bluetooth_FifoControlReg = *Bluetooth_FifoControlReg | 0x06;
	// Now Clear all bits in the FiFo control registers
	*Bluetooth_FifoControlReg = *Bluetooth_FifoControlReg ^  0x06;
}

int putcharbluetooth(int c, volatile unsigned char * LineStatusReg ,  volatile unsigned char * TransmitterFifo) {
	// wait for Transmitter Holding Register bit (5) of line status register to be '1'
	// indicating we can write to the device
	while ((*LineStatusReg & 0x20)!= 0x20);
	*TransmitterFifo = (unsigned char)c;
	// write character to Transmitter fifo register
	return c;
	// return the character we printed
}

int getcharbluetooth( volatile unsigned char * LineStatusReg ,  volatile unsigned char *  ReceiverFifo ) {
	while ((*LineStatusReg & 0x01)!= 0x01);
	// wait for Data Ready bit (0) of line status register to be '1'
	// read new character from ReceiverFiFo register
	return (int) *ReceiverFifo;
	// return new character
}

// the following function polls the UART to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read from the FIFO
int TestForReceivedData(volatile unsigned char *  LineStatusReg) {
	// if RS232_LineStatusReg bit 0 is set to 1
	if((*LineStatusReg & 0x01)== 0x01){
		return 1;
	}
	else{
		return 0;
	}
}

//wifi module

void Init_WiFi (void){
	WiFi_LineControlReg = 0x80;

	//set the baud rate here
    int divisor = (int) ((50E6)/(115200 *16));
    WiFi_DivisorLatchLSB = divisor & 0xff;
    WiFi_DivisorLatchMSB = (divisor >> 8) & 0xff;

    //set the data bits and stop bits here
    WiFi_LineControlReg = 0x33;
    WiFi_FifoControlReg = 0x6;
    WiFi_FifoControlReg = 0;
}

int putcharWiFi (int  c){
	while((WiFi_LineStatusReg & 0x20) != 0x20){
		// prevent the while loop break. also prove the code is still writing to the wifi chip
		printf("waiting\r\n");
	};
    //while( ((WiFi_LineStatusReg >> 5) & 1) == 0){}
	//write the data into the WiFi_TransmitterFifo to transmit data
    WiFi_TransmitterFifo = c;
    return c;
}

int getcharWiFi (void){
    // wait for WiFi_LineStatusReg to be '1'.
	//WiFi_ReceiverFifo is valid if WiFi_LineStatusReg is "1".
    while ( (WiFi_LineStatusReg & 1) == 0){}

    // return the value of WiFi_LineStatusReg
    return WiFi_ReceiverFifo;
}

//test whether the wifi receive the data
int WiFi_TestForReceivedData (void){
    // receive the data if WiFi LineStatusReg bit 0 is 1.
	//then return 1
    return (WiFi_LineStatusReg & 1);
}

//Waits for the transmission of outgoing serial data to complete.
void WiFi_Flush (void){
    volatile int temp = 0;

    while(WiFi_LineStatusReg & 1) {
    	//check whether the wifi finish sending the data
    	temp = WiFi_ReceiverFifo;
    	//this is just prevent the while loop break
    }
    return;
    //return if the wifi chip finish sending the data
}


void print_wifi_m (char * Message){
    int i;
    //enter an array of message we need to send, send it one char by one char
    for(i = 0; Message[i] != '\0'; i++){
    	putcharWiFi(Message[i]);
    }
}

//test whether it is receiving the data or not
int testWiFi (void){
    if((WiFi_LineStatusReg & 1)){
        return 1;
    }
    return 0;
}


//#include <io.h>

void wifi_send(char * Message, char * temp){
	printf("\r\nEnter Message for WiFi Controller: ") ;
	printf("\r\nhere wifi send");
	print_wifi_m(Message) ; // write string to bluetooth device

	// if the command string was NOT "$$$" send \r\n to simulate click "enter" button to send the script
	if(strcmp(Message, "$$$") != 0) {
		putcharWiFi('\r') ;
		putcharWiFi('\n') ;
	}

	// now read back acknowledge string from device and display on console,
	// will timeout after no communication for about 2 seconds
	char c;
	for(int i = 0; i < 2000000; i ++) {
		if(testWiFi() == 1) {
			c = getcharWiFi();
			putchar(c);
		}
	}
}
void send_lua (void)
{

	//put lua lines into Message array
    char Message1[100]= "wifi.sta.config('TP-LINK_888','12345687')";
	char Message2[100]= "wifi.sta.connect()";
	//link the wifi first
	char Message3[100]= "tmr.delay(1000000)";
	char Message4[100]= "print(wifi.sta.status())";
	char Message5[100]= "print(wifi.sta.getip())";
	//print the ip address itself to check whether it connects the wifi successfully
	char Message6[100]= "sk=net.createConnection(net.TCP, 0)";
	char Message7[100]= "sk:on('receive', function(sck, c) print(c) end )";
	//print the receive the message
	char Message8[100]= "sk:connect(3000,'52.138.39.36')";
	//the ip address of the backend of the app
	char Message9[100]= "sk:send('GET /sms\\r\\nConnection: keep-alive\\r\\nAccept: */*\\r\\n\\r\\n')";
	//send the request to the backend
    char temp[20]= "\r\n";
    //the "enter" signal

    wifi_send(temp, temp);
    wifi_send(temp, temp);
    //simulate click the "enter" button twice to make sure the wifi chip is ready for enter the lua script
    wifi_send(Message1, temp);
    wifi_send(Message2, temp);
    wifi_send(Message3, temp);
    wifi_send(Message4, temp);
    wifi_send(Message5, temp);
    wifi_send(Message6, temp);
    wifi_send(Message7, temp);
    wifi_send(Message8, temp);
    wifi_send(Message9, temp);
    //enter the lua script, click the "enter" button after each line of the script
    wifi_send(temp, temp);
    //click the "enter" button at finally

}


int main(void) {


	Init_bluetooth();
	Init_RS232();
//	bluetooth_send();
	while(1){
			int usernameCounter = 0;
			int username[100];

			// waiting for sign in customer ID from the user
			while(1){
				if(TestForReceivedData(Bluetooth_LineStatusReg) == 1) {
					int c = getcharbluetooth(Bluetooth_LineStatusReg , Bluetooth_ReceiverFifo);
					if(c != 13 && c != 10){
						printf("received %d from the Bluetooth \n", c);
						username[usernameCounter] = c;

						if(c == 50 || c == 49){ //customer1 and customer2
							break;
							// if it is end with 1 or 2 break the loop go to send the message to raspberry pi
							//and send requests through wifi chip
						}
						usernameCounter ++;
					}

				}
			}

			// send username to Raspberry Pi through rs232
			for(int i = 0; i <= usernameCounter; i++){
				printf("send rs232 to RPI:%d\n", username[i]);
				putcharRS232(username[i]);
			}

			//send the request through the wifi chip
			Init_WiFi();
		    send_lua();

	}

}








