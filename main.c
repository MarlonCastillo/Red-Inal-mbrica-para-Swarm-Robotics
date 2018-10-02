/*
 * File:   main.c
 * Author: MARLON
 *
 * Created on 22 de abril de 2018, 11:58 AM
 */
// CONFIG1
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)
#pragma config CCPMX = RB0      // CCP1 Pin Selection bit (CCP1 function on RB0)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// CONFIG2
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode disabled)

#include <xc.h>
#include <pic16f88.h>
#include <stdio.h>
#include <string.h>
#define _XTAL_FREQ 8000000

//********************************
//DECLARACION DE FUNCIONES
//********************************
void Start_UART(void);
void SendChar_UART(char c);
void SendString_UART(char *c);
char ReceiveChar_UART(void);
void ReceiveString_UART(void);
void Start_ESP(void);
void WaitResponse_ESP(char *c);
//********************************
//DECLARACION DE VARIABLES
//********************************
char mensaje[10]="";        //almacena los mensajes recibidos por serial
int recibido=0;               //bandera para indicar la recepcion de un mensaje
char *puntero=&mensaje[0];               //puntero hacia variable mensaje
char *puntero2=&mensaje[0];               //puntero hacia valor de comando
int contador=0;             //contador para delimitar el tamaño del mensaje

void main(void) {
//********************************
//OSCILADOR INTERNO
//********************************
    OSCCONbits.IRCF=7; //osiclador interno a 8MHZ
    OSCCONbits.IOFS=1; //frecuencia estable
    OSCCONbits.SCS=2; //fc del sistema igual a fc interna

//********************************
//CONFIGURACION DE INTERRUPCIONES
//********************************
    PIR1bits.RCIF=0;        //limpia bandera de RX   
    PIE1bits.RCIE=1;        //habilitar interrupcion de RX
    INTCONbits.PEIE=1;      //habilitar interrupciones de perifericos
    
    TRISBbits.TRISB0=0;     //pines para led RGB
    TRISBbits.TRISB1=0;
    TRISBbits.TRISB3=0;
    PORTBbits.RB0=0;
    PORTBbits.RB1=0;
    PORTBbits.RB3=0;
    __delay_ms(500);         //esperamos a que reinicie el ESP
    Start_UART();           //inicar UART
    

    Start_ESP();            //iniciar ESP
    
    INTCONbits.GIE=1;       //habilitar interrupciones globales
    
    while(1){
        
//        if(recibido==1){
//            PIE1bits.RCIE=0;           //deshabilitar interrupciones globales
//            if(*puntero2=='L'){ 		//comparar que el paquete recibido sea lo que esta almacenado en a
//                puntero2++;
//                if(*puntero2=='a'){
//                    PORTBbits.RB1=!PORTBbits.RB1;
//                    puntero2=&mensaje[0];
//                }
//            }
//            recibido=0;			//reiniciar recepcion de paquetes
//            PIE1bits.RCIE=1;           //habilitar interrupciones globales
//        }
        
    }
}

//********************************************
//RUTINA DE INTERRUPCION
void interrupt interrupcion(){
    //verificar que bandera se disparo
    if(PIR1bits.RCIF==1){
        if(RCREG=='-'){
            //recibido=2; 	//si se recibio caracter de inicio de paquete habilitar almacenamiento de mensaje
            PORTBbits.RB1=!PORTBbits.RB1;
            PIR1bits.RCIF=0;    //limpiar banderas
            return;
        }
		
		//*************************************
		//CONCATENAMIENTO DE MENSAJE
//        if(recibido==2){
//            *puntero=RCREG;         //almacenar dato
//            if (*puntero== '\0' || contador==5){ 
//                recibido=1;         //indica que ya se recibio un mensaje completo
//                contador=0;         //reiniciar contador
//                puntero=&mensaje[0];   //reiniciar puntero
//                PIR1bits.RCIF=0;    //limpiar banderas
//                return;
//            }
//            contador++;
//            puntero++;
//        }
		//*************************************
        
    }
    //limpiar banderas
    PIR1bits.RCIF=0;
    return;
}

//********************************************
//CONFIGURACION INICIAL DEL PUERTO SERIAL
void Start_UART(void){
    //configurar puerto
   
    TXSTAbits.BRGH=1;   //baudrate de alta velocidad
    SPBRG=1;           //baudrate de 250000
    TXSTAbits.SYNC=0;   //modo asincrono
    RCSTAbits.SPEN=1;   //habilita el puerto serial
    TRISBbits.TRISB5=0;    //salita TX
    TRISBbits.TRISB2=1;    //entrada RX
    
    //configurar TX
    TXSTAbits.TX9=0;    //transmision de 8 bits
    
    //configurar RX
    RCSTAbits.RX9=0;    //recepcion de 8 bits
    
    TXSTAbits.TXEN=1;           //habilitar transmision
    RCSTAbits.CREN=1;           //habilitar recepcion
   
}

//********************************************
//ENVIAR UN CHAR POR SERIAL
void SendChar_UART(char c) {
    TXREG = c;                  //cargar dato
    while (!TXSTAbits.TRMT);    //espera a que termine la transmision
}

//***************************
//ENVIAR STRING POR SERIAL
void SendString_UART(char *c){
    while(*c!='\n'){ 
        SendChar_UART(*(c++)); 
    }
    SendChar_UART(*c); 
}

//***************************
//RECIBIR CARACTER POR SERIAL
char ReceiveChar_UART(void) {
    while (!PIR1bits.RCIF);     //espera a que se reciba el caracter
    return RCREG;               //devolver dato recibido
}

//***************************
//RECIBIR STRING POR SERIAL
void ReceiveString_UART(void){
    while(1){
        *puntero = ReceiveChar_UART(); //recibe y almacena caracter
        if (*puntero== '\n' || contador==8){ //verifica el final de linea
            break; 
        } 
        puntero++;  //incrementa puntero de buffer
        contador++; //incrementa contador
    }
    puntero++;
    *puntero = '\0';  //agrega caracter de final de cadena para realizar comparacion
    contador=0;         //reiniciar contador
    puntero=&mensaje[0];   //reiniciar puntero
} 
//***************************
//CONFIGURACION INICIAL ESP
void Start_ESP(void){
    PORTBbits.RB0=1;
    SendString_UART("AT+RST\r\n");      //RESET
    WaitResponse_ESP("ready\r\n");    
    SendString_UART("ATE0\r\n");        //desactivar echo de serial
    WaitResponse_ESP("OK\r\n");    
    SendString_UART("AT+CWMODE_CUR=1\r\n"); //modo cliente
    WaitResponse_ESP("OK\r\n");    
    //SendString_UART("AT+CWJAP_CUR=\"Megaproyecto\",\"ABC1234567\"\r\n"); //conectarse a la red
    SendString_UART("AT+CWJAP_CUR=\"MARLON\",\"CASTILLO@5186\"\r\n"); //conectarse a la red
    WaitResponse_ESP("OK\r\n"); 
    SendString_UART("AT+CIFSR\r\n");    //obtener IP
    WaitResponse_ESP("OK\r\n");
    PORTBbits.RB0=0;
    PORTBbits.RB3=1;
    SendString_UART("AT+CIPSTART=\"TCP\",\"192.168.1.7\",54000\r\n"); //iniciar conexion TCP
    WaitResponse_ESP("OK\r\n");
    PORTBbits.RB3=0;
}

//******************************
//ESPERAR RESPUESTA DE ESP8266
void WaitResponse_ESP(char *c){
    while(1){
            ReceiveString_UART();
            if(strcmp(mensaje, c) == 0) break; //si hay respuesta valida sale del loop
    }
}