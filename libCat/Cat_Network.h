// Cat_Network.h
// �l�b�g���[�N�̏������Ɛڑ������Ȃ�

/*
void draw_callback(void* pvUserData ) {
	// �`�揈��
	// �_�C�A���O�\���̂����̔w�i��`�悷��
	sceGuStart() �` sceGuFinish()
}

void screen_update_callback(void* pvUserData ) {
	// �X�V����
	// �t���[���o�b�t�@�̃X���b�v�Ȃ�
	sceGuSync( 0, 0 ); sceDisplayWaitVblankStartCB(); sceGuSwapBuffers(); �Ȃ�
}


	// �l�b�g���[�N���C�u�����̏�����
	Cat_NetworkInit();
	...
	...

	// �ڑ�����
	pvUserData = 0; // �R�[���o�b�N�֐��ł̑������ƂȂ�
	rc = Cat_NetworkConnect( draw_callback, screen_update_callback, pvUserData );
	if(rc < 0) {
		error
	} else if(rc > 0) {
		cancel
	} else {
		connect
	}

	...
	...
	// �l�b�g���[�N���C�u�����̏I������
	Cat_NetworkTerm();

����
 �E�l�b�g���[�N���W���[�������炩���ߓǂݍ���ł����K�v������
 �E1.0/1.50��kernelMode�ȃX���b�h�œǂݍ���ł�������

*/

#ifndef INCL_Cat_Network_h
#define INCL_Cat_Network_h

#ifdef __cplusplus
extern "C" {
#endif

#include <pspkerneltypes.h>

// �ڑ��֘A

// �l�b�g���[�N�̏�����
extern int Cat_NetworkInit();

// �l�b�g���[�N�̏I������
extern void Cat_NetworkTerm();

// �l�b�g���[�N�ڑ�����
// �Ԃ�l < 0 : �G���[
// �Ԃ�l = 0 : �ڑ�����
// �Ԃ�l > 0 : �L�����Z�����ꂽ
extern int Cat_NetworkConnect( void (*draw_callback)(void*), void (*screen_update_callback)(void*), void* pvUserData );

// �l�b�g���[�N�ڑ���Ԃ��擾
extern int Cat_NetworkIsConnect( void );

#ifdef __cplusplus
}
#endif

#endif
