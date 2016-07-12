#include "ieinc.h"


static int  aiVirtualComDescriptors[8] = {-1, -1, -1, -1, -1, -1, -1, -1};



//--- MsSleep ---

static void MsSleep(long lMsec)
{
   struct timeval tdelay;

   // delays lMsec milliseconds

   tdelay.tv_usec = (lMsec % 1000) * 1000;
   tdelay.tv_sec  = lMsec / 1000;
   select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tdelay);
}



//--- kbhit ---

static char *kbhit()
{
   int nFdStdIn = fileno(stdin);
   struct timeval sTijdSlot;
   fd_set FdInSet;
   int iSelRdy, iLen;
   static char szStdin[1];

   FD_ZERO(&FdInSet);
   FD_SET(nFdStdIn, &FdInSet);
   sTijdSlot.tv_sec = 0l;
   sTijdSlot.tv_usec = 0l;
   iSelRdy = select(64, &FdInSet, NULL, NULL, (struct timeval *) &sTijdSlot);
   if (iSelRdy > 0) {
      if(FD_ISSET(nFdStdIn, &FdInSet)){
        iLen = read(nFdStdIn, szStdin, 10);
        if (iLen > 0)
          return szStdin;
      }
   }
   return (char *) NULL;
}



//--- VComOpenPort ---

static int VComOpenPort(char *szPortName, int iPortMode)
{
   // Opens a serial port, e.g. "/dev/ttya".
   // iPortMode indicates the baud rate, stopbits, etc.
   // The following flags can be or-ed into iPortMode:
   //
   // - nr of data bits : CS8, CS7, CS6 or CS5
   // - nr of stop bits : <none>	1 stop bit, CSTOPB 2 stop bits
   // - baud rate       : B1200, B2400, B4800, B9600 or B19200...
   // - parity          : <none>	no parity, PARENB even parity, (PARENB | PARODD) Odd parity

   int fd;
   struct termios sTtySet;                       // get port filedescriptor

   fd = open(szPortName, O_RDWR | O_NOCTTY | O_NONBLOCK);
   if (fd < 0) {
      printf("VComOpenPort: can not open <%s>\n", szPortName);
      return -1;
   }
   
   printf("VComOpenPort: name=<%s> iPortMode=%d -> fd=%d\n", szPortName, iPortMode, fd);

   tcgetattr(fd, &sTtySet);                      // get port settings & set new ...

   sTtySet.c_iflag = IGNBRK;                     // Ignore breaks, note that IXON, IXOFF and ISTRIP are not set.
   sTtySet.c_oflag &= ~OPOST;                    // no post processing
   sTtySet.c_cflag = iPortMode | CREAD;          // set baudrate parity and so on
   sTtySet.c_lflag = 0;                          // disable canonical mode, reset the hard way
   sTtySet.c_cc[VMIN] = sTtySet.c_cc[VTIME] = 0; // set mode to non-blocking
   sTtySet.c_cc[VTIME] = 0;
   tcflush(fd, TCIFLUSH);                        // flush input + output queues

   tcsetattr(fd, TCSANOW, &sTtySet);             // initialise port with new settings...
   ioctl(fd, TIOCEXCL, 0);                       // make port access exclusive
   return (fd);
}



static void VComInit(void)
{
   aiVirtualComDescriptors[0] = VComOpenPort("/dev/tty00", (B1200 | CS8));
   aiVirtualComDescriptors[1] = VComOpenPort("/dev/tty01", (B1200 | CS8));
   aiVirtualComDescriptors[2] = VComOpenPort("/dev/tty02", (B1200 | CS8));
   aiVirtualComDescriptors[3] = VComOpenPort("/dev/tty03", (B1200 | CS8));
   aiVirtualComDescriptors[4] = VComOpenPort("/dev/tty04", (B1200 | CS8));
   aiVirtualComDescriptors[5] = VComOpenPort("/dev/tty05", (B1200 | CS8));
   aiVirtualComDescriptors[6] = VComOpenPort("/dev/tty06", (B1200 | CS8));
   aiVirtualComDescriptors[7] = VComOpenPort("/dev/tty07", (B1200 | CS8));
}



/*
static void VComRead(void)
{
   int ii, ret;
   char sz[128];

   for (ii = 0; ii < MAX_ORION; ii++) {
      if (aiVirtualComDescriptors[ii*2] >= 0) {
         ret = read(aiVirtualComDescriptors[ii*2], sz, 127);
         if (ret > 0)
            printf("VComRead[%d]: RS422: %s\n", ii, sz);
      }
      if (aiVirtualComDescriptors[(ii*2)+1] >= 0) {
         ret = read(aiVirtualComDescriptors[(ii*2)+1], sz, 127);
         if (ret > 0)
            printf("VComRead[%d]: RS485: %s\n", ii, sz);
      }
   }
}
*/



static void VComWrite(int port)
{
   int ret;
   char *sz = "This is a very simple example of writing a string of data to the virtual serial port of the orion\n\r";

   // port 0 = rs422, 1 = rs485
   if (aiVirtualComDescriptors[port] >= 0) {
      ret = write(aiVirtualComDescriptors[port], sz, strlen(sz)+1);
      if (ret <= 0)
         printf("VComWrite[%d]: error (%d)\n", port, ret);
   }
}



static void VComExit()
{
   int ii;
   for (ii = 0; ii < 7; ii++)
      if (aiVirtualComDescriptors[ii] >= 0) 
         close(aiVirtualComDescriptors[ii]);
}


#ifdef TEST

//--- main ---

int main(int argc, char **argv)
{
   BOOL bQuit = FALSE;
   char *pKey;

   VComInit();

   printf("vcom tester : '0' = tty00 ... '7' = tty07 (even=422,odd=485), 'q' to quit\n");

   while (!bQuit) {
      MsSleep(100);

      pKey = kbhit();
      if (!pKey) 
         continue;

      switch (*pKey) {
         case 'q':
            bQuit++;
            break;

         case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            VComWrite(*pKey - '0');
            break;
      }
   }

   VComExit();
   exit(0);
}

#endif


