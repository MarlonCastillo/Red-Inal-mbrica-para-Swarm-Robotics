#include <winsock.h>
#include <windows.h>
#include <string>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

//*************************************************************
//VARIABLES PARA CREAR LOS SOCKETS
#define MENSAJE_NOTIFICACION      1048 //mensaje de notificación para los sockets
sockaddr_in client;				//estructura para el cliente
int clientSize = sizeof(client);
WNDCLASSEX wc;
HWND hwnd;
MSG Msg;
HINSTANCE hInstanceTCP;

const int maxconecciones = 10;
IN_ADDR direccionesIP[maxconecciones];		//vectores para las ip, los puertos y los sockets
int puertos[maxconecciones];
SOCKET arregloSockets[maxconecciones];
IN_ADDR direccionCero = direccionesIP[0];	//variables para resetear los valores en los arreglos
SOCKET SocketCero = arregloSockets[0];
int n = 0;						//posición de los vectores
int nTemp;					//almacena el valor de n temporalmente cuando se acepta una conexión
SOCKET  SocketTemp;			//almacena un socket temporalmente cuando se acepta una conexión
CHAR *IPTemp;				//guarda la IP temporalmente cuando se acepta una conexión	
int error;					//almacena el error cuando se envían datos a un host en específico

SOCKET sock;
WSADATA wsData;
WORD ver = MAKEWORD(2, 2);
sockaddr_in hint;				//estructura en la que se almacena la ip y el puerto del socket a utilizar

const char g_szClassName[] = "myWindowClass";

//*************************************************************
//Declaración de funciones
void IniciarSocket(int puerto);
void CerrarSocket();
LRESULT CALLBACK WndProcTCP(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CrearVentanaTCP();

//*****************************************************
//CONFIGURA EL SOCKET A UTILIZAR
//*****************************************************
void IniciarSocket(int puerto) {
	//*****************************************************
	//CREACION DE VENTANA
	CrearVentanaTCP();

	//*****************************************************
	//INICIAR WINSOCK
	int wstartok = WSAStartup(ver, &wsData);
	if (wstartok != 0) {
		MessageBox(hwnd, "No se puede inicializar winsock.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//*****************************************************
	//CREAR SOCKET
	sock = socket(AF_INET, SOCK_STREAM, 6);
	if (sock == INVALID_SOCKET) {
		MessageBox(hwnd, "No se puede crear el socket.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//*****************************************************
	//ENLAZAR IP Y PUERTO AL SOCKET
	hint.sin_family = AF_INET;
	hint.sin_port = htons(puerto);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; 	//INADDR_ANY es para habilitar que escuche en todas las interfaces

	int bindok = bind(sock, (sockaddr*)&hint, sizeof(hint)); //enlazamiento de ip y puerto al socket
	if (bindok == SOCKET_ERROR) {
		MessageBox(hwnd, "No se puede enlazar ip o puerto al socket.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return;
	}


	//*****************************************************
	//HABILITAR QUE EL SOCKET ESCUCHE
	int listenok = listen(sock, SOMAXCONN);
	if (listenok == SOCKET_ERROR) {
		MessageBox(hwnd, "El socket no puede escuchar las conexiones.", "ERROR", MB_OK | MB_ICONINFORMATION);
		return;
	}

	//*****************************************************
	//CAMBIAR A SOCKETS ASINCRONOS
	int asyncok = WSAAsyncSelect(sock, hwnd, MENSAJE_NOTIFICACION, FD_ACCEPT);
	if (asyncok == SOCKET_ERROR) {
		MessageBox(hwnd, "No se puede crear el socket asíncrono.", "ERROR", MB_OK | MB_ICONINFORMATION);
		/*int error = WSAGetLastError();
		LPCSTR merror = to_string(error).c_str();
		MessageBox(hwnd, merror, "ERROR", MB_OK | MB_ICONINFORMATION);*/
		return;
	}
}

//*****************************************************
//CIERRA EL SOCKET Y LIMPIA LA CONEXIÓN
//*****************************************************
void CerrarSocket() {
	//cerrar socket
	closesocket(sock);

	//limpiar winsock
	WSACleanup();
}

bool EnviarTCP(char *direccionIP, char *mensaje, int largo) {
	for (int i = 0; i < maxconecciones; i++) {
		if (strcmp(direccionIP, inet_ntoa(direccionesIP[i])) == 0) { //convertir IP a string
			client.sin_addr = direccionesIP[i];
			client.sin_port = puertos[i];
			error = sendto(arregloSockets[i], mensaje, largo, 0, (sockaddr*)&client, clientSize);
			if (error == -1) {
				//error = WSAGetLastError();
				//cout << error << endl;
				for (int j = i; j < maxconecciones - 1; j++) {
					arregloSockets[j] = arregloSockets[j + 1];
					direccionesIP[j] = direccionesIP[j + 1];
					puertos[j] = puertos[j + 1];
				}
				arregloSockets[9] = SocketCero;
				direccionesIP[9] = direccionCero;
				puertos[9] = 0;
			}
			//send(clientSocket, mensaje, sizeof(mensaje), 0); //envio de datos al cliente
			PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&Msg);
			return TRUE;
		}
	}
	PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);
	DispatchMessage(&Msg);
	return FALSE;
}
//*************************************************************
//FUNCIÓN PARA EL WINDOWS HANDLER
//Se encarga de manejar todos los eventos de la ventana
LRESULT CALLBACK WndProcTCP(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		/*
		case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
		PostQuitMessage(0);
		break;*/
		//*************************************************************
		//aquí se procesan todos los eventos de los sockets
	case MENSAJE_NOTIFICACION: { //se recibió un mensaje?
		switch (lParam) {
		case FD_ACCEPT:
			//listo para aceptar una conexión entrante
			SocketTemp = accept(sock, (sockaddr*)&client, &clientSize);
			IPTemp = inet_ntoa(client.sin_addr); //convertir IP a string
			nTemp = n;
			for (int i = 0; i < maxconecciones; i++) {
				if (strcmp(IPTemp, inet_ntoa(direccionesIP[i])) == 0) {		//verificar si la IP ya existe en el arreglo
					n = i;
					break;
				}
				nTemp = n + 1;
			}

			arregloSockets[n] = SocketTemp;
			direccionesIP[n] = client.sin_addr;
			puertos[n] = client.sin_port;
			//PostQuitMessage(0);		
			PostMessage(hwnd, WM_LBUTTONDOWN, 0, 0); //ayuda a reiniciar las banderas
			n = nTemp;
			break;

			//case FD_READ:
			//	//listo para recibir datos
			//	MessageBox(hwnd, "datos para leer", "This program is:", MB_OK | MB_ICONINFORMATION);
			//	break;

			//case FD_OOB:
			//	//se cerró la conexión del socket
			//	MessageBox(hwnd, "Puede enviar datos.", "Mi ventana", MB_OK | MB_ICONINFORMATION);
			//	break;
		}
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


//*****************************************************
//CREA LA VENTANA PARA MANEJAR LOS EVENTOS DEL SOCKET
//*****************************************************
void CrearVentanaTCP() {
	//Parámetros de la ventana
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProcTCP;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstanceTCP;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//verificar si hubo error en el registro de la ventana
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Fallo en el registro de la ventana.", "ERROR", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//creacion de ventana
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, "Mi ventana",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstanceTCP, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Fallo en la creación de la ventana.", "ERROR", MB_ICONEXCLAMATION | MB_OK);
		return;
	}


	//mostrar ventana
	//ShowWindow(hwnd, nCmdShow);  
	//UpdateWindow(hwnd);

}
