#include <Adafruit_LiquidCrystal.h>  // Librería para pantalla LCD

Adafruit_LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Pines de la LCD

const int BotonInicioPin = 6;   // Botón de inicio
const int BotonPararPin = 7;    // Botón de pausa
const int EntradaAnalogPin = A0;  // Pin de entrada de la señal analógica

int EstadoBotonInicio = LOW;     // Estado actual del pulsador de inicio
int UltimoEstadoBotonInicio = LOW; // Estado anterior del pulsador de inicio
int PararBotonEstado = LOW;      // Estado actual del pulsador de pausa
int UltimoEstadoBotonParar = LOW;  // Estado anterior del pulsador de pausa
bool acquiringSignal = false;   // Bandera para saber si estamos adquiriendo señal

unsigned long UltimoTiempoRebote = 0;  // Tiempo desde la última lectura de pulsadores
unsigned long DelayRebote = 50;    // Retardo para el debouncing (en milisegundos)

const int tamanoInicialBuffer = 200;
int* buffer = nullptr;
int* tamanoBuffer = nullptr;
unsigned long* tiempoInicio = nullptr;
unsigned long* tiempoFinal = nullptr;
int* ciclosCompletos = nullptr;
char* formaOnda = nullptr;

const int umbralCuadrada = 300;
const int umbralTriangular = 50;

// Funciones de inicialización y liberación de memoria
void inicializarBuffer(int tamano) {
    buffer = new int[tamano];
    tamanoBuffer = new int;
    *tamanoBuffer = tamano;
}

void liberarMemoria() {
    delete[] buffer;
    delete tamanoBuffer;
    delete tiempoInicio;
    delete tiempoFinal;
    delete ciclosCompletos;
    delete[] formaOnda;
}

// Función para capturar la señal
void capturarSenal() {
    for (int i = 0; i < *tamanoBuffer; i++) {
        buffer[i] = analogRead(EntradaAnalogPin);
        Serial.print(buffer[i]);
        Serial.print(" ");
        delay(5);
    }
    Serial.println();
}

// Función para medir la frecuencia
void medirFrecuencia(float* frecuencia) {
    *ciclosCompletos = 0;
    bool cruzandoPorCero = false;

    *tiempoInicio = millis();

    for (int i = 1; i < *tamanoBuffer; i++) {
        if (buffer[i] > 512 && buffer[i-1] < 512) {
            if (!cruzandoPorCero) {
                (*ciclosCompletos)++;
                cruzandoPorCero = true;
            }
        } else if (buffer[i] < 512 && buffer[i-1] > 512) {
            if (!cruzandoPorCero) {
                (*ciclosCompletos)++;
                cruzandoPorCero = true;
            }
        } else {
            cruzandoPorCero = false;
        }
    }

    *tiempoFinal = millis();
    float tiempoSegundos = (*tiempoFinal - *tiempoInicio) / 1000.0;
    if (tiempoSegundos > 0) {
        *frecuencia = *ciclosCompletos / tiempoSegundos;
    } else {
        *frecuencia = 0;
    }

    Serial.print("Ciclos Completos: ");
    Serial.println(*ciclosCompletos);
    Serial.print("Tiempo (s): ");
    Serial.println(tiempoSegundos);
}

// Función para medir la amplitud
void medirAmplitud(float* amplitud) {
    int valorMaximo = 0;
    int valorMinimo = 1023;

    for (int i = 0; i < *tamanoBuffer; i++) {
        if (buffer[i] > valorMaximo) {
            valorMaximo = buffer[i];
        }
        if (buffer[i] < valorMinimo) {
            valorMinimo = buffer[i];
        }
    }

    int amplitudRaw = valorMaximo - valorMinimo;
    *amplitud = (amplitudRaw * 5.0) / 1023.0;

    Serial.print("Valor Máximo: ");
    Serial.println(valorMaximo);
    Serial.print("Valor Mínimo: ");
    Serial.println(valorMinimo);
    Serial.print("Amplitud Raw: ");
    Serial.println(amplitudRaw);
    Serial.print("Amplitud (V): ");
    Serial.println(*amplitud);
}

// Función para identificar la forma de la onda
void identificarFormaDeOnda() {
    bool esCuadrada = true;
    bool esTriangular = true;
    int cambiosBruscos = 0;

    for (int i = 1; i < *tamanoBuffer; i++) {
        int diferencia = abs(buffer[i] - buffer[i-1]);

        if (diferencia > umbralCuadrada) {
            cambiosBruscos++;
        } else {
            esCuadrada = false;
        }

        if (diferencia > umbralTriangular) {
            esTriangular = false;
        }
    }

    if (cambiosBruscos > (*tamanoBuffer / 4)) {
        esCuadrada = true;
    } else {
        esCuadrada = false;
    }

    if (esCuadrada) {
        strcpy(formaOnda, "C");
    } else if (esTriangular) {
        strcpy(formaOnda, "T");
    } else {
        strcpy(formaOnda, "S");
    }

    Serial.print("Forma de Onda: ");
    Serial.println(formaOnda);
}

// Función para mostrar la forma de onda cuando la señal está pausada
void mostrarFormaDeOnda() {
    lcd.setCursor(0, 1);
    lcd.print("FdO: ");
    
    if (strcmp(formaOnda, "S") == 0) {
        lcd.print("S");
    } else if (strcmp(formaOnda, "T") == 0) {
        lcd.print("T");
    } else if (strcmp(formaOnda, "C") == 0) {
        lcd.print("C");
    } else {
        lcd.print("Des");
    }
}

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.print("Iniciando...");

    pinMode(BotonInicioPin, INPUT);
    pinMode(BotonPararPin, INPUT);

    tiempoInicio = new unsigned long;
    tiempoFinal = new unsigned long;
    ciclosCompletos = new int;
    formaOnda = new char[20];

    inicializarBuffer(tamanoInicialBuffer);

    lcd.setCursor(0, 1);
    lcd.print("Esperando...");
    delay(2000);
}

void loop() {
    int lecturaBotonInicio = digitalRead(BotonInicioPin);
    int lecturaBotonParar = digitalRead(BotonPararPin);

    if (lecturaBotonInicio != UltimoEstadoBotonInicio) {
        UltimoTiempoRebote = millis();
    }
    if (lecturaBotonParar != UltimoEstadoBotonParar) {
        UltimoTiempoRebote = millis();
    }

    if ((millis() - UltimoTiempoRebote) > DelayRebote) {
        if (lecturaBotonInicio != EstadoBotonInicio) {
            EstadoBotonInicio = lecturaBotonInicio;
            if (EstadoBotonInicio == HIGH) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Adquiriendo senal");
                acquiringSignal = true;
                capturarSenal();
            }
        }

        if (lecturaBotonParar != PararBotonEstado) {
            PararBotonEstado = lecturaBotonParar;
            if (PararBotonEstado == HIGH) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Senal en pausa");
                acquiringSignal = false;
                identificarFormaDeOnda();
                mostrarFormaDeOnda();
            }
        }
    }

    if (acquiringSignal) {
        float frecuencia = 0;
        float amplitud = 0;

        medirFrecuencia(&frecuencia);
        medirAmplitud(&amplitud);
        identificarFormaDeOnda();

        lcd.setCursor(0, 0);
        lcd.print("Freq: ");
        lcd.print(frecuencia);
        lcd.print(" Hz ");


        lcd.setCursor(0, 1);
        lcd.print("Ampli: ");
        lcd.print(amplitud);
        lcd.print(" V ");

        Serial.print("Forma de Onda: ");
        Serial.println(formaOnda);
    }

    UltimoEstadoBotonInicio = lecturaBotonInicio;
    UltimoEstadoBotonParar = lecturaBotonParar;

    delay(50);
}