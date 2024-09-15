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
