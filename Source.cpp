//código basado en la documentación de https://www.postgresql.org/docs/10/static/index.html
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "libpq-fe.h"	//libreria para Postgres
#include "SocketTCP.h"

using namespace std;

//*****************************************************
//Declaración de variables para la base de datos
PGconn *Conector;			//objeto de conección
char *InfoConector;			//guarda la informacion para conectarse a la base de datos
PGresult   *Res;		//objeto de los Result
int         nFields;
int         i,j;

int main() {
	//CrearVentanaTCP();
	IniciarSocket(54000);
	
	//*****************************************************
	//CONECTAR A BASE DE DATOS
	InfoConector = "host=Mesarobot hostaddr=169.254.139.103 port=5432 dbname=mbots user=postgres password=123";
	Conector = PQconnectdb(InfoConector);
	if (PQstatus(Conector) != CONNECTION_OK) {//verificar la conexión 
		cout << "ERROR" << endl;
		cout << "No se puede conectar a la base de datos" << endl;
		cout << PQerrorMessage(Conector);
	}

	Res = PQexec(Conector, "SELECT * FROM mybots");

	if (PQresultStatus(Res) != PGRES_TUPLES_OK) {

		printf("No data retrieved\n");
		PQclear(Res);
	}

	int rows = PQntuples(Res);
	int colum = PQnfields(Res);
	for (int i = 0; i<rows; i++) {
		for (int j = 0; j < colum; j++) {
			cout << PQgetvalue(Res, i, j) << " | ";
		}
		cout << endl;
		/*printf("%s %s %s\n", PQgetvalue(Res, i, 0),
			PQgetvalue(Res, i, 1), PQgetvalue(Res, i, 2));*/
	}

	PQclear(Res);
	
	////*****************************************************
	////CREAR RUTA SEGURA
	////evita que otros usuarios tomen el control
	//Res = PQexec(Conector,"SELECT pg_catalog.set_config('search_path', '', false)"); 
	//if (PQresultStatus(Res) != PGRES_TUPLES_OK) { //verificar resultado
	//	cout << "ERROR" << endl;
	//	cout << "No se puede crear una ruta segura" << endl;
	//	cout << PQerrorMessage(Conector);
	//}

	//PQclear(Res);		//libera la memoria utilizada por el resultado del comando

	////*****************************************************
	////INICIAR BLOQUE DE TRANSACCION
	//Res = PQexec(Conector, "BEGIN"); //indica que comienza una secuencia de comandos
	//if (PQresultStatus(Res) != PGRES_COMMAND_OK){ //verificar resultado
	//	cout << "ERROR" << endl;
	//	cout << "Comando BEGIN fallo" << endl;
	//	cout << PQerrorMessage(Conector);
	//}
	//PQclear(Res);		

	////crea una variable de tipo cursor para la base de datos
	////Res = PQexec(Conector, "DECLARE myportal CURSOR FOR select * from mbots"); 
	//Res = PQexec(Conector, "TRUNCATE TABLE mybots");
	//if (PQresultStatus(Res) != PGRES_COMMAND_OK){
	//	cout << "ERROR" << endl;
	//	cout << "Comando DECLARE CURSOR fallo" << endl;
	//	cout << PQerrorMessage(Conector);
	//	
	//}
	//PQclear(Res);
	//
	//////*****************************************************
	//////CERRAR PORTAL
	////Res = PQexec(Conector, "CLOSE myportal");	//libera la variable creada
	////PQclear(Res);

	//////*****************************************************
	//////TERMINAR TRANSACCIÓN
	//Res = PQexec(Conector, "END");		//finaliza la secuencia de comandos
	//PQclear(Res);

	PQfinish(Conector);  //cierra la conexión y libera la memoria utilizada
	while (1) {
		EnviarTCP("192.168.1.3", "-L", 2);
	}
	CerrarSocket();
}