// -----------------------------------------------------------------------------------
// Command processing
#include "../OnStepX.h"
#include "../tasks/OnTask.h"
extern Tasks tasks;

#include "../coordinates/Convert.h"
#include "../telescope/Telescope.h"
#include "ProcessCmds.h"

// command processors
#ifdef SERIAL_A
  CommandProcessor processCommandsA(SERIAL_A_BAUD_DEFAULT,'A');
  void processCmdsA() { processCommandsA.poll(); }
#endif
#ifdef SERIAL_B
  CommandProcessor processCommandsB(SERIAL_B_BAUD_DEFAULT,'B');
  void processCmdsB() { processCommandsB.poll(); }
#endif
#ifdef SERIAL_C
  CommandProcessor processCommandsC(SERIAL_C_BAUD_DEFAULT,'C');
  void processCmdsC() { processCommandsC.poll(); }
#endif
#ifdef SERIAL_D
  CommandProcessor processCommandsD(SERIAL_D_BAUD_DEFAULT,'D');
  void processCmdsD() { processCommandsD.poll(); }
#endif
#ifdef SERIAL_ST4
  CommandProcessor processCommandsST4(9600,'S');
  void processCmdsST4() { processCommandsST4.poll(); }
#endif
#if SERIAL_BT_MODE == SLAVE
  CommandProcessor processCommandsBT(9600,'T');
  void processCmdsBT() { processCommandsBT.poll(); }
#endif
#ifdef SERIAL_IP
  CommandProcessor processCommandsIP(9600,'I');
  void processCmdsIP() { processCommandsIP.poll(); }
#endif

CommandProcessor::CommandProcessor(long baud, char channel) {
  this->channel = channel;
  serialBaud = baud;
}

CommandProcessor::~CommandProcessor() {
  SerialPort.end();
}

void CommandProcessor::poll() {
  if (!serialReady) { delay(200); SerialPort.begin(serialBaud); serialReady = true; }
  if (SerialPort.available()) buffer.add(SerialPort.read()); else return;
  if (buffer.ready()) {
    char reply[50] = "";
    bool numericReply = true;
    bool supressFrame = false;

    commandError = command(reply, buffer.getCmd(), buffer.getParameter(), &supressFrame, &numericReply);

    if (numericReply) {
      if (commandError != CE_NONE) strcpy(reply,"0"); else strcpy(reply,"1");
      supressFrame = true;
    }
    if (strlen(reply) > 0 || buffer.checksum) {
      if (buffer.checksum) {
        appendChecksum(reply);
        strcat(reply,buffer.getSeq());
        supressFrame = false;
      }
      if (!supressFrame) strcat(reply,"#");
      SerialPort.write(reply);
    }

    // debug, log errors and/or commands
    #if DEBUG_ECHO_COMMANDS == ON
      VF("MSG: cmd"); V(channel); V(" = "); V(buffer.getCmd()); V(buffer.getParameter()); VF(", reply = "); V(reply);
    #endif
    if (commandError != CE_NULL) {
      lastCommandError = commandError;
      #if DEBUG_ECHO_COMMANDS == ON
        if (commandError > CE_0) { VF(", Error "); V(commandErrorStr[commandError]); }
      #endif
    }
    #if DEBUG_ECHO_COMMANDS == ON
      VL("");
    #endif

    buffer.flush();
  }
}

CommandError CommandProcessor::command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply) {
  commandError = CE_NONE;

  // Handle telescope commands
  if (telescope.command(reply, command, parameter, supressFrame, numericReply, &commandError)) return commandError;

  // :SB[n]#    Set Baud Rate where n is an ASCII digit (1..9) with the following interpertation
  //            0=115.2K, 1=56.7K, 2=38.4K, 3=28.8K, 4=19.2K, 5=14.4K, 6=9600, 7=4800, 8=2400, 9=1200
  //            Returns: 1 (at the current baud rate and then changes to the new rate for further communication)
  if (cmdP("SB")) {
    int rate = parameter[0] - '0';
    if (rate >= 0 && rate <= 9) {
      const static long baud[10] = {115200, 56700, 38400, 28800, 19200, 14400, 9600, 4800, 2400, 1200};
      SerialPort.print("1");
      tasks.yield(50);
      SerialPort.begin(baud[rate]);
      *numericReply = false;
    } else commandError = CE_PARAM_RANGE;
    return commandError;
  } else

  // internal MCU temperature in deg. C
  if (cmd2("GX9F")) {
    float t = HAL_TEMP();
    if (!isnan(t)) sprintF(reply, "%1.0f", t); else { *numericReply = true; commandError = CE_0; }
    return commandError;
  } else

  return CE_CMD_UNKNOWN;
}

void CommandProcessor::appendChecksum(char *s) {
  char HEXS[3] = "";
  uint8_t cks = 0; for (unsigned int cksCount0 = 0; cksCount0 < strlen(s); cksCount0++) { cks += s[cksCount0]; }
  sprintf(HEXS, "%02X", cks);
  strcat(s, HEXS);
}

void commandChannelInit() {
  // Command processing
  // add tasks to process commands
  // period ms (0=idle), duration ms (0=forever), repeat, priority (highest 0..7 lowest), task_handle
  uint8_t handle;
  #ifdef HAL_SLOW_PROCESSOR
    long comPollRate = 2000;
  #else
    long comPollRate = 250;
  #endif
  #ifdef SERIAL_A
    VF("MSG: Setup, start command channel A task (priority 6)... ");
    handle = tasks.add(0, 0, true, 6, processCmdsA, "PrcCmdA");
    if (handle) { VL("success"); } else { VL("FAILED!"); }
    tasks.setPeriodMicros(handle, comPollRate);
  #endif
  #ifdef SERIAL_B
    VF("MSG: Setup, start command channel B task (priority 6)... ");
    handle = tasks.add(0, 0, true, 6, processCmdsB, "PrcCmdB");
    if (handle) { VL("success"); } else { VL("FAILED!"); }
    tasks.setPeriodMicros(handle, comPollRate);
  #endif
  #ifdef SERIAL_C
    VF("MSG: Setup, start command channel C task (priority 6)... ");
    handle = tasks.add(0, 0, true, 6, processCmdsC, "PrcCmdC");
    if (handle) { VL("success"); } else { VL("FAILED!"); }
    tasks.setPeriodMicros(handle, comPollRate);
  #endif
  #ifdef SERIAL_D
    VF("MSG: Setup, start command channel D task (priority 6)... ");
    handle = tasks.add(0, 0, true, 6, processCmdsD, "PrcCmdD")) { VL("success"); } else { VL("FAILED!"); }
    if (handle) { VL("success"); } else { VL("FAILED!"); }
    tasks.setPeriodMicros(handle, comPollRate);
  #endif
  #ifdef SERIAL_ST4
    VF("MSG: Setup, start command channel ST4 task (priority 6)... ");
    if (tasks.add(3, 0, true, 6, processCmdsST4, "PrcCmdS")) { VL("success"); } else { VL("FAILED!"); }
  #endif
  #if SERIAL_BT_MODE == SLAVE
    VF("MSG: Setup, start command channel BT task (priority 6)... ");
    handle = tasks.add(0, 0, true, 6, processCmdsBT, "PrcCmdT");
    if (handle) { VL("success"); } else { VL("FAILED!"); }
    tasks.setPeriodMicros(handle, comPollRate);
  #endif
  #ifdef SERIAL_IP
    VF("MSG: Setup, start command channel IP task (priority 6)... ");
    if (tasks.add(1, 0, true, 6, processCmdsIP, "PrcCmdI")) { VL("success"); } else { VL("FAILED!"); }
  #endif
}
