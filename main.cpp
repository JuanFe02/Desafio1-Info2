#include <Adafruit_LiquidCrystal.h> 

Adafruit_LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int BotonInicioPin = 6;
const int BotonPararPin = 7;
const int EntradaAnalogPin = A0;

int EstadoBotonInicio = LOW;
int UltimoEstadoBotonInicio = LOW;
int PararBotonEstado = LOW;
int UltimoEstadoBotonParar = LOW;
bool acquiringSignal = false;

unsigned long UltimoTiempoRebote = 0;
unsigned long DelayRebote = 50;

const int tamanoInicialBuffer = 100;
int* buffer = nullptr;
int* tamanoBuffer = nullptr;
unsigned long* tiempoInicio = nullptr;
unsigned long* tiempoFinal = nullptr;
int* ciclosCompletos = nullptr;
char* formaOnda = nullptr;

void inicializarBuffer(int tamano) 
{
    buffer = new int[tamano];
    tamanoBuffer = new int;
    *tamanoBuffer = tamano;
}

void liberarMemoria() 
{
    delete[] buffer;
    delete tamanoBuffer;
    delete tiempoInicio;
    delete tiempoFinal;
    delete ciclosCompletos;
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

void medirFrecuencia(float* frecuencia) 
{
    *ciclosCompletos = 0;
    bool cruzandoPorCero = false;
    
    *tiempoInicio = millis();
    
    for (int i = 1; i < *tamanoBuffer; i++) 
    {
        if (buffer[i] > 512 && buffer[i-1] < 512) 
        {
            if (!cruzandoPorCero)
            {
                (*ciclosCompletos)++;
                cruzandoPorCero = true;
            }
        } 
        else if (buffer[i] < 512 && buffer[i-1] > 512) 
        {
            if (!cruzandoPorCero) 
            {
                (*ciclosCompletos)++;
                cruzandoPorCero = true;
            }
        } 
        else 
        {
            cruzandoPorCero = false;
        }
    }
    
    *tiempoFinal = millis();
    float tiempoSegundos = (*tiempoFinal - *tiempoInicio) / 1000.0;
    *frecuencia = *ciclosCompletos / tiempoSegundos;
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
    
    int amplitudRaw = valorMaximo - valorMinimo;
    *amplitud = (amplitudRaw * 5.0) / 1023.0;
}

int detectarCicloCompleto() 
{
    int inicioCiclo = -1;
    for (int i = 1; i < *tamanoBuffer; i++) 
    {
        if ((buffer[i - 1] < 512 && buffer[i] >= 512) || (buffer[i - 1] > 512 && buffer[i] <= 512)) 
        {
            inicioCiclo = i;
            break;
        }
    }
    return inicioCiclo;
}


//detectar forma de ondas, faltante


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
    else 
    {
        lcd.print("Des");  // Desconocida
    }
}

void setup() 
{
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

void loop()
{
    int lecturaBotonInicio = digitalRead(BotonInicioPin);
    int lecturaBotonParar = digitalRead(BotonPararPin);

}
