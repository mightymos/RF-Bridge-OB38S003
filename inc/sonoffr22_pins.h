// pin defines
// avoid exposing to other compilation units
#define BUZZER      P0_0
#define TDATA       P0_7
#define UART_TX_PIN P1_0

// reset pin does not appear to be connected to anything except copper pad so can use as a debug output
// apparently need to set as gpio in PC flashing utility when flashing (i.e., disable reset function)
#define RESET_PIN   P1_5
#define RDATA       P1_6
#define RF_ENABLE   P1_7
#define LED         P3_0