/*
 *PSP DIALOGS LIBRARY
 *Thomas Klein			tommydanger@aon.at

 *This library will provide some
 *basic functions to call the
 *psp internal
 *  -messagebox
 *  -error
 *  -keyboard
 *dialogs as seen in psp games
 */

#ifndef __PSP_DIALOGS_H__
#define __PSP_DIALGOS_H__

#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <psputility.h>
#include <psputility_osk.h>
#include <pspgu.h>

/*
  Autoset language according to your psp system
  see psputility_sysparam.h for language codes
*/
#define DIALOG_LANGUAGE_AUTO			(100)

extern void* framebuffer;

typedef struct
{
	char message[512];
	int options;
	pspUtilityMsgDialogPressed buttonPressed;
	int result;
}MESSAGE_HELPER;

typedef struct
{
	int action;
	int result;
}NETCONF_HELPER;

/*
  Struct to help initialize OSK
*/
typedef struct
{
	unsigned short *title;		/* Title of the OSK */
	unsigned short *pretext;	/* Text at startup */
	u16 textlength;				/* Size of textarray */
	u16 textlimit;				/* Limit for entering text */
	unsigned short text[128];		/* After OSK is called the entered text will be here */
	u16 lines;					/* Textbox lines*/
}OSK_HELPER;

/*
  Show a message dialog

  msg - Message to show
  language - set the language to display the message box in
  pass DIALOG_LANGUAGE_AUTO or see psputility_sysparam.h
*/
void pspShowMessageDialog(MESSAGE_HELPER *mh, u8 language);

/*
  Display a error code

  error - error to display, see pspkerror.h
  language - set the language to display the message box in
  pass DIALOG_LANGUAGE_AUTO or see psputility_sysparam.h
*/
void pspShowErrorDialog(const int error, u8 language);

/*
  Show Onscreenkeyboard

  oskhelper - structure to initialize OSK, recieved text will be stored in here
  language - set the language to display the message box in
  pass DIALOG_LANGUAGE_AUTO or see psputility_sysparam.h
*/
void pspShowOSK(OSK_HELPER *oskhelper, u8 language);

/*
  Minimal rendering routine during dialog is active
*/
void minimalRender();


#endif

