#include "ieinc.h"

#define VENDOR  "2bcd"
#define PRODUCT "0200"



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



//--- GetSerialNumber ---

static char *GetSerialNumber(int iUSB)
{
   #define RESULT_TAG "ATTRS{serial}==\""
   FILE *fp;
   static char rd_line[128];
   char *p, *p2;
   char cmd[128];
   BOOL bError = FALSE;

   sprintf(cmd, "udevadm info -a -n /dev/ttyUSB%d | grep 'ATTRS{serial}==' | head -n1", iUSB * 2);

   if ((fp = popen(cmd, "r")) != NULL) { // Result should be like : ATTRS{serial}=="1552042"<CR><LF>
      if (fgets(rd_line, sizeof(rd_line), fp) != NULL) {
         p = strstr(rd_line, RESULT_TAG);
         if (p == NULL) {
            fprintf(stderr, "GetSerialNumber: can't find [%s] in %s\n", RESULT_TAG, rd_line);
            bError++;
         }
         p2 = strstr(rd_line, "\n");
         *p2 = '\0';
      }
      else {
         fprintf(stderr, "GetSerialNumber: reading popen fails\n");
         bError++;
      }
      pclose(fp);
   }
   else {
      fprintf(stderr, "GetSerialNumber: popen fails\n");
      bError++;
   }

   return bError ? NULL : p;
}



//--- main ---

int main(int argc, char **argv)
{
   #define FILENAME "/etc/udev/rules.d/99-usb-serial.rules"
   FILE *rules = NULL;
   char *aszNumbers[] = {"first", "second", "thirst", "last"};
   int  bQuit = 0, iUSB = 0;
   char *pSnr, *pKey, szOut[512] = "";
   BOOL bOK = FALSE;
 
   rules = fopen(FILENAME, "w+");
   if (rules == NULL) {
      fprintf(stderr, "Can not open %s   Are you root?\n", FILENAME);
      exit(-1);
   }

   // Start the process loop...

   printf("\nConnect the %s orion, power on, wait for the beep and then hit: 'n'(next)  'r'(ready)  'q'(quit) + <CR>\n\n", aszNumbers[iUSB]);

   while (!bQuit) {
      MsSleep(100);

      pKey = kbhit();
      if (!pKey) 
         continue;

      switch (*pKey) {
         case 'q':
            bQuit++;
            break;

         case 'n':
            pSnr = GetSerialNumber(iUSB);
            if (!pSnr) {
               bQuit++;
               break;
            }
            sprintf(&szOut[strlen(szOut)], "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"%s\", ATTRS{idProduct}==\"%s\", %s, SYMLINK+=\"tty0%%n\"\n", VENDOR, PRODUCT, pSnr);

            printf("usb[%d,%d] ==> %s\n\n", iUSB*2, iUSB*2+1, pSnr);
            if (++iUSB == 4) {
               bQuit++;
               bOK++;
               break;
            }
            printf("Connect the %s orion, power on, wait for the beep and then hit: 'n'(next)  'r'(ready)  'q'(quit) + <CR>\n", aszNumbers[iUSB]);
            break;

         case 'r':
            bQuit++;
            if (iUSB > 0)
               bOK++;
      }
   }

   if (!bOK)
      fprintf(stderr, "Some failure, try again\n");
   else {
      if (fwrite(szOut, 1, strlen(szOut), rules) == strlen(szOut))
         printf("OK, write success:\n%s\n", szOut);
      else {
         fprintf(stderr, "Can not write to %s\n", FILENAME);
         bOK = FALSE;
      }
   }
   
   fclose(rules);

   if (!bOK)
      exit(-1);

   printf("Ready, power off-on all...\n");
   // ls -l /dev/tty* | grep tty[0,U][0-7,S]"
   exit(0);
}




