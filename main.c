/*
* $Id$
*/

#include <pspctrl.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <psppower.h>
#include <stdio.h>
#include "pg.h"
#include "psp2ch.h"

/* Define the module info section */
PSP_MODULE_INFO("2ch Browser for PSP", PSP_MODULE_USER, 0, 8);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(4*1024); // psphtmlviewer.c ‚ÌMEM_SIZE‚Æ‚ÌŒ“‚Ë‡‚¢‚Å

extern S_2CH s2ch; // psp2ch.c

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
    s2ch.running = 0;
    sceKernelExitGame();
    return 0;
}

int power_callback(int unknown, int pwrflags, void *common)
{
    /* check for power switch and suspending as one is manual and the other automatic */
    if (pwrflags & PSP_POWER_CB_POWER_SWITCH || pwrflags & PSP_POWER_CB_SUSPENDING) {
    } else if (pwrflags & PSP_POWER_CB_RESUMING) {
    } else if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE) {
    } else if (pwrflags & PSP_POWER_CB_STANDBY) {
    } else {
    }
    sceDisplayWaitVblankStart();

	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);
    sceKernelSleepThreadCB();

    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0x10000, 0, 0);
    if (thid >= 0)
    sceKernelStartThread(thid, 0, 0);
    return thid;
}

int main(int argc, char *argv[])
{
    char* ch;

    strcpy(s2ch.cwDir, argv[0]);
    ch = strrchr(s2ch.cwDir, '/');
    *ch = '\0';
    SetupCallbacks();
    pgSetupGu();
    //pspDebugScreenInit();
    psp2chInit();
    pgFontLoad();
    psp2ch(); // main loop
    pgTermGu();
    psp2chTerm();
    sceKernelExitGame();
    return 0;
}
