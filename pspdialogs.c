#include "pspdialogs.h"

/* pspdlg list */
extern unsigned int list[];

void pspShowErrorDialog(const int error, u8 language)
{
    int done = 0;
    pspUtilityMsgDialogParams dialog;
    SceSize dialog_size = sizeof(dialog);

    memset(&dialog, 0, dialog_size);
    dialog.base.size = dialog_size;
    if(language == DIALOG_LANGUAGE_AUTO)
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&dialog.base.language);
    else
        dialog.base.language = language;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &dialog.base.buttonSwap);
    dialog.base.graphicsThread = 0x11;
    dialog.base.accessThread = 0x13;
    dialog.base.fontThread = 0x12;
    dialog.base.soundThread = 0x10;
    //dialog.unknown2[1] = 0;
    //dialog.unknown2[2] = error;
    dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_ERROR;
    dialog.errorValue = error;

    sceUtilityMsgDialogInitStart(&dialog);

    while(!done)
    {
        minimalRender();

        switch(sceUtilityMsgDialogGetStatus())
        {
            case PSP_UTILITY_DIALOG_NONE:
                break;
            case PSP_UTILITY_DIALOG_INIT :
                break;
            case PSP_UTILITY_DIALOG_VISIBLE:
                sceUtilityMsgDialogUpdate(2);
                break;
            case PSP_UTILITY_DIALOG_QUIT:
                sceUtilityMsgDialogShutdownStart();
                break;
            case PSP_UTILITY_DIALOG_FINISHED :
                done = 1;
                break;
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}

void pspShowMessageDialog(MESSAGE_HELPER *mh, u8 language)
{
    int done=0;
    pspUtilityMsgDialogParams dialog;
    SceSize dialog_size = sizeof(dialog);

    memset(&dialog, 0, dialog_size);
    dialog.base.size = dialog_size;
    if(language == DIALOG_LANGUAGE_AUTO)
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&dialog.base.language);
    else
        dialog.base.language = language;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &dialog.base.buttonSwap);
    dialog.base.graphicsThread = 0x11;
    dialog.base.accessThread = 0x13;
    dialog.base.fontThread = 0x12;
    dialog.base.soundThread = 0x10;
    //dialog.unknown2[1] = 1;
    dialog.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
    strcpy(dialog.message, mh->message);
    dialog.options = mh->options;

    sceUtilityMsgDialogInitStart(&dialog);

    while(!done)
    {
        minimalRender();

        switch(sceUtilityMsgDialogGetStatus())
        {
            case PSP_UTILITY_DIALOG_NONE:
                break;
            case PSP_UTILITY_DIALOG_INIT :
                break;
            case PSP_UTILITY_DIALOG_VISIBLE:
                sceUtilityMsgDialogUpdate(2);
                break;
            case PSP_UTILITY_DIALOG_QUIT:
                sceUtilityMsgDialogShutdownStart();
                break;
            case PSP_UTILITY_DIALOG_FINISHED :
                done = 1;
                break;
        }
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
    mh->result = dialog.base.result;
    mh->buttonPressed = dialog.buttonPressed;
}

void pspShowOSK(OSK_HELPER *oskhelper, u8 language)
{
    int done=0;
    SceUtilityOskData data;
    SceUtilityOskParams osk;

    memset(&data, 0, sizeof(data));
    data.unk_00 = 1;           // 1=kanji
    data.unk_04 = 0;
    data.language = 0;         // key glyphs: 0-1=hiragana, 2+=western
    data.unk_12 = 0;
    data.unk_16 = 0;           //12;    //0;
    data.lines = oskhelper->lines;
    data.unk_24 = 0;
    data.desc = oskhelper->title;
    data.intext = oskhelper->pretext;
    data.outtextlength = oskhelper->textlength;
    data.outtextlimit = oskhelper->textlimit;
    data.outtext = oskhelper->text;

    memset(&osk, 0, sizeof(osk));
    osk.base.size = sizeof(osk);
    if(language == DIALOG_LANGUAGE_AUTO)
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&osk.base.language);
    else
        osk.base.language = language;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &osk.base.buttonSwap);
    osk.base.graphicsThread = 0x11;
    osk.base.accessThread = 0x13;
    osk.base.fontThread = 0x12;
    osk.base.soundThread = 0x10;
    osk.unk_48 = 1;
    osk.data = &data;

    sceUtilityOskInitStart(&osk);

    while(!done)
    {
        minimalRender();

        switch(sceUtilityOskGetStatus())
        {
            case PSP_UTILITY_DIALOG_NONE :
                break;
            case PSP_UTILITY_DIALOG_INIT :
                break;
            case PSP_UTILITY_DIALOG_VISIBLE :
                sceUtilityOskUpdate(2); // 2 is taken from ps2dev.org recommendation
                break;
            case PSP_UTILITY_DIALOG_QUIT :
                sceUtilityOskShutdownStart();
                break;
            case PSP_UTILITY_DIALOG_FINISHED :
                done = 1;
                break;
        }

        sceKernelDelayThread(10*1000);
        sceDisplayWaitVblankStart();
        framebuffer = sceGuSwapBuffers();
    }
}

#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
void minimalRender()
{
	sceGuStart(GU_DIRECT,list);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuClearColor(0xff000000);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);
}


