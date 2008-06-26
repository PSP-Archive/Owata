
#include <stdlib.h>
#include "psp2ch.h"
#include "psphtmlviewer.h"

extern S_2CH s2ch;

#define MEM_SIZE 8*1024*1024
void pspShowBrowser(char *url, char *dldir)
{
    int done=0;
    pspUtilityHtmlViewerParam html;
    SceSize html_size = sizeof(html);
	SceUID vpl;

	vpl = sceKernelCreateVpl("BrowserVpl", PSP_MEMORY_PARTITION_USER, 0, MEM_SIZE + 256, NULL);

    memset(&html, 0, html_size);
    html.base.size = html_size;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&html.base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &html.base.buttonSwap);
    html.base.graphicsThread = 0x11;
    html.base.accessThread = 0x13;
    html.base.fontThread = 0x12;
    html.base.soundThread = 0x10;

	html.memsize = MEM_SIZE;
	html.unknown1 = 0;
	html.unknown2 = 0;
	html.initialurl = url;
	html.numtabs = 1;
	html.interfacemode = PSP_UTILITY_HTMLVIEWER_INTERFACEMODE_FULL;
	html.options = PSP_UTILITY_HTMLVIEWER_DISABLE_STARTUP_LIMITS | PSP_UTILITY_HTMLVIEWER_ENABLE_FLASH;
	// WITHOUT 'ms0:' on the paths
	html.dldirname = dldir;
	html.cookiemode = PSP_UTILITY_HTMLVIEWER_COOKIEMODE_DEFAULT;
	html.unknown3 = 0;
	html.homeurl = url;
	html.textsize = PSP_UTILITY_HTMLVIEWER_TEXTSIZE_NORMAL;
	html.displaymode = PSP_UTILITY_HTMLVIEWER_DISPLAYMODE_SMART_FIT;
	html.connectmode = PSP_UTILITY_HTMLVIEWER_CONNECTMODE_MANUAL_ALL;
	html.disconnectmode = PSP_UTILITY_HTMLVIEWER_DISCONNECTMODE_CONFIRM;

    if (sceKernelAllocateVpl(vpl, html.memsize, &html.memaddr, NULL) < 0)
	{
		sceKernelDeleteVpl(vpl);
		return;
	}
    if (sceUtilityHtmlViewerInitStart(&html) < 0)
	{
		sceKernelFreeVpl(vpl, html.memaddr);
		sceKernelDeleteVpl(vpl);
		return;
	}

    while(!done)
    {
        minimalRender();

        switch(sceUtilityHtmlViewerGetStatus())
        {
            case PSP_UTILITY_DIALOG_NONE:
                break;
            case PSP_UTILITY_DIALOG_INIT :
                break;
            case PSP_UTILITY_DIALOG_VISIBLE:
                sceUtilityHtmlViewerUpdate(1);
                break;
            case PSP_UTILITY_DIALOG_QUIT:
                sceUtilityHtmlViewerShutdownStart();
                break;
            case PSP_UTILITY_DIALOG_FINISHED :
                done = 1;
                break;
        }
        sceKernelDelayThread(5*1000);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
	sceKernelFreeVpl(vpl, html.memaddr);
	sceKernelDeleteVpl(vpl);
}
