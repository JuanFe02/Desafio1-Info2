#include <Adafruit_LiquidCrystal.h> 

Adafruit_LiquidCrystal lcd(12, 11, 5, 4, 3, 2); 

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

const int tamanoInicialBuffer = 100;  // Tamaño inicial del buffer de almacenamiento
int* buffer = nullptr;                // Puntero para el buffer dinámico
int* tamanoBuffer = nullptr;          // Puntero para almacenar el tamaño del buffer dinámico
char* formaOnda = nullptr;  // Puntero para almacenar el tipo de onda (S, T, C o D)


void inicializarBuffer(int tamano) 
{
    buffer = new int[tamano];  // Asignar memoria dinámica para el buffer
    tamanoBuffer = new int;  // Asignar memoria dinámica para el tamaño del buffer
    *tamanoBuffer = tamano;
}

void liberarMemoria() 
{
    delete[] buffer;
    delete tamanoBuffer;
    delete[] formaOnda;
}

void capturarSenal() 
{
    for (int i = 0; i < *tamanoBuffer; i++) 
    {
        buffer[i] = analogRead(EntradaAnalogPin);
        delay(10);  
    }
}

void medirAmplitud(float* amplitud) 
{
    int valorMaximo = 0;
    int valorMinimo = 1023;

    for (int i = 0; i < *tamanoBuffer; i++) 
    {
        if (buffer[i] > valorMaximo) 
        {
            valorMaximo = buffer[i];
        }
        if (buffer[i] < valorMinimo) 
        {
            valorMinimo = buffer[i];
        }
    }

    int amplitudR = valorMaximo - valorMinimo;
    *amplitud = (amplitudR* 5.0) / 1023.0;
}

// Aqui va la funcion analizar la onda, trabajando en ello




void mostrarFormaDeOnda() 
{
    lcd.setCursor(0, 1);
    lcd.print("FdO: ");
    
    if (strcmp(formaOnda, "S") == 0) 
    {
        lcd.print("S");
    } 
    else if (strcmp(formaOnda, "T") == 0) 
    {
        lcd.print("T");
    } 
    else if (strcmp(formaOnda, "C") == 0)
    {
        lcd.print("C");
    } 
    else {
        lcd.print("D");  // Desconocida
    }
}

void setup() 
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.print("Iniciando...");

    pinMode(BotonInicioPin, INPUT);
    pinMode(BotonPararPin, INPUT);

    formaOnda = new char[20];

    inicializarBuffer(tamanoInicialBuffer);

    lcd.setCursor(0, 1);
    lcd.print("Esperando...");
    delay(2000);
}

void loop() 
{
    int lecturaBotonInicio = digitalRead(BotonInicioPin);
    int lecturaBotonParar = digitalRead(BotonPararPin);

    if (lecturaBotonInicio != UltimoEstadoBotonInicio || lecturaBotonParar != UltimoEstadoBotonParar) 
    {
        UltimoTiempoRebote = millis();
    }

    if ((millis() - UltimoTiempoRebote) > DelayRebote) 
    {
        if (lecturaBotonInicio != EstadoBotonInicio) 
        {
            EstadoBotonInicio = lecturaBotonInicio;
            if (EstadoBotonInicio == HIGH) 
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Obten senal");
                acquiringSignal = true;
                capturarSenal();
            }
        }

        if (lecturaBotonParar != PararBotonEstado) 
        {
            PararBotonEstado = lecturaBotonParar;
            if (PararBotonEstado == HIGH) 
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Senal en pausa");
                acquiringSignal = false;
                identificarFormaDeOnda();
                mostrarFormaDeOnda();  // Mostrar la forma de onda en pausa
            }
        }
    }

    if (acquiringSignal) 
    {
        float amplitud = 0;

        medirAmplitud(&amplitud);
        identificarFormaDeOnda();

        lcd.setCursor(0, 0);
        lcd.print("Analizando senal");
      	
      	lcd.setCursor(0, 1);
        lcd.print("Ampli: ");
        lcd.print(amplitud);
        lcd.print(" V");

        Serial.print("Forma de Onda: ");
        Serial.println(formaOnda);
    }

    UltimoEstadoBotonInicio = lecturaBotonInicio;
    UltimoEstadoBotonParar = lecturaBotonParar;

    delay(50);
}
