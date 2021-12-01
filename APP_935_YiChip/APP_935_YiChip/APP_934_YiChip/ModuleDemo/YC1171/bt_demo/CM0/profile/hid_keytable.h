
#ifndef __HID_KEYTABLE_H__
#define __HID_KEYTABLE_H__

#include "blueware.h"

#if MAX_NO_HID_KEYBOARD

#define ENABLE_WINDOWS_CHINESE				1
#define ENABLE_MULTI_COUNTRY_KEYBOARD		1

typedef enum
{
 	HID_KEY_ENGLISH = 1,
	HID_KEY_USA,
	HID_KEY_TURKEY,
	HID_KEY_SPAIN,
	HID_KEY_PORTUGAL,
	HID_KEY_FRANCE,
	HID_KEY_GERMANY,
	HID_KEY_ITALY,
	HID_KEY_CZECH,
	HID_KEY_JAPAN,
	HID_KEY_MAX,
} HID_KEYTABLE_T;


//¶àÃ½Ìå¼üÅÌ
#define	next_track_hid		0		//ÏÂÒ»Çú
#define	prev_track_hid		1		//ÉÏÒ»Çú
#define	stop_hid			2		//Í£Ö¹
#define	play_pause_hid		3		//²¥·Å/ÔÝÍ£
#define	mute_hid			4		//¾²Òô
#define	media_hid			5		//¶àÃ½Ìå
#define	volume_dec_hid		6		//ÒôÁ¿¼Ó
#define	volume_add_hid		7		//ÒôÁ¿¼õ

#define	www_web_hid			8		//ÍøÒ³
#define	www_back_hid		9		//ºóÍË
#define	www_forward			10		//Ç°½ø
#define	www_stop_hid		11		//Í£Ö¹
#define	www_refresh_hid		12		//Ë¢ÐÂ
#define	www_favorite_hid	13		//ÊÕ²Ø¼Ð
#define	www_search_hid		14		//ÊÕË÷
#define	mail_hid			15		//ÓÊ¼þ

#define	my_computer_hid		16		//ÎÒµÄµçÄÔ
#define	calculator_hid		17		//¼ÆËãÆ÷

#define	power_hid			24		//¹Ø»ú
#define	sleep_hid			25		//Ë¯Ãß
#define	wake_up_hid			26		//»½ÐÑ


//±ê×¼¼üÅÌ
#define	hid_k42				0x32
#define	hid_k45				0x64
#define	hid_enter			0x28
#define	hid_esc				0x29
#define	hid_semicolon		0x33	//·ÖºÅ" ; "
#define	hid_quotation_mark	0x34	//ÒýºÅ " '",
#define	hid_comma			0x36	//¶ººÅ
#define	hid_full_stop		0x37	//¾äºÅ
#define	hid_solidus_r		0x38	//ÓÒÐ±Ïß"/"
#define	hid_space			0x2c
#define	hid_scroll			0x47
#define	hid_print			0x46
#define	hid_pause			0x48
#define	hid_wave_line		0x35	//²¨ÀËÏß" `( ~ )"		

#define	hid_back			0x2a
#define	hid_tab				0x2b
#define	hid_underline		0x2d	//ÏÂ»®Ïß"-(_)"
#define	hid_equal_mark		0x2e	//µÈºÅ"=(+)"
#define	hid_braces_l		0x2f	//×ó°ë´óÀ¨ºÅ
#define	hid_braces_r		0x30	//ÓÒ°ë´óÀ¨ºÅ
#define	hid_solidus_l		0x31	//×óÐ±Ïß"\"
#define	hid_caps			0x39
#define	hid_f1				0x3a
#define	hid_f2				0x3b
#define	hid_f3				0x3c
#define	hid_f4				0x3d
#define	hid_f5				0x3e
#define	hid_f6				0x3f
#define	hid_f7				0x40
#define	hid_f8				0x41
#define	hid_f9				0x42
#define	hid_f10				0x43
#define	hid_f11				0x44
#define	hid_f12				0x45
#define	hid_ins				0x49
#define	hid_home			0x4a
#define	hid_page_up			0x4b
#define	hid_del				0x4c
#define	hid_end				0x4d
#define	hid_page_down		0x4e
#define	hid_right			0x4f
#define	hid_left			0x50
#define	hid_down			0x51
#define	hid_up				0x52
#define	hid_menu			0x65	//²Ëµ¥¼ü


//Êý×Ö¼üÅÌ(Ð¡¼üÅÌÇø)
#define	hid_nk_num			0x53
#define	hid_nk_div			0x54	//³ýºÅ"/"
#define	hid_nk_asterisk		0x55	//³ËºÅ,ÐÇºÅ¼ü"*"
#define	hid_nk_sub			0x56	//¼õºÅ"-"
#define	hid_nk_plus			0x57	//¼ÓºÅ"+"
#define	hid_nk_enter		0x58
#define	hid_nk_1			0x59
#define	hid_nk_2			0x5a
#define	hid_nk_3			0x5b
#define	hid_nk_4			0x5c
#define	hid_nk_5			0x5d
#define	hid_nk_6			0x5e
#define	hid_nk_7			0x5f
#define	hid_nk_8			0x60
#define	hid_nk_9			0x61
#define	hid_nk_0			0x62
#define	hid_nk_del			0x63

//Ó¢ÎÄ×ÖÄ¸Çø
#define	hid_a				0x04
#define	hid_b				0x5
#define	hid_c				0x6
#define	hid_d				0x7
#define	hid_e				0x8
#define	hid_f				0x9
#define	hid_g				0xA
#define	hid_h				0xb
#define	hid_i				0xc
#define	hid_j				0xd
#define	hid_k				0xe
#define	hid_l				0xf
#define	hid_m				0x10
#define	hid_n				0x11
#define	hid_o				0x12
#define	hid_p				0x13
#define	hid_q				0x14
#define	hid_r				0x15
#define	hid_s				0x16
#define	hid_t				0x17
#define	hid_u				0x18
#define	hid_v				0x19
#define	hid_w				0x1a
#define	hid_x				0x1b
#define	hid_y				0x1c
#define	hid_z				0x1d

//Êý×Ö¼üÇø
#define	hid_1				0x1e
#define	hid_2				0x1f
#define	hid_3				0x20
#define	hid_4				0x21
#define	hid_5				0x22
#define	hid_6				0x23
#define	hid_7				0x24
#define	hid_8				0x25
#define	hid_9				0x26
#define	hid_0				0x27

//×éºÏ¼ü
#define	hid_ctrl_l			0xe0
#define	hid_shift_l			0xE1
#define	hid_alt_l			0xe2
#define	hid_win_l			0xe3
#define	hid_ctrl_r			0xe4
#define	hid_shift_r			0xE5
#define	hid_alt_r			0xe6
#define	hid_win_r			0xe7
	
#define IMAGE(a,b)			  (a << 8) + b	
#define CHAR_SHIFT			  0x20
#define CHAR_ALT			  0x40

#define KEYTABLE_MAX_SIZE	  140	

static const uint16_t Keytable_Default [KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na

	0x00, 0x00, 0x2c, IMAGE(CHAR_SHIFT,hid_1), IMAGE(CHAR_SHIFT,0x34), IMAGE(CHAR_SHIFT,hid_3), IMAGE(CHAR_SHIFT,hid_4), IMAGE(CHAR_SHIFT,hid_5), IMAGE(CHAR_SHIFT,hid_7), 0x34, // Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_9), IMAGE(CHAR_SHIFT,hid_0), IMAGE(CHAR_SHIFT,hid_8), IMAGE(CHAR_SHIFT,hid_equal_mark),0x36, 0x2d, 0x37, 0x38, 0x27, 0x1e, // ( ) * + , - . / 0 1  
	0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, IMAGE(CHAR_SHIFT,hid_semicolon), 0x33, // 2 3 4 5 6 7 8 9 : ;
	IMAGE(CHAR_SHIFT,hid_comma), 0x2e, IMAGE(CHAR_SHIFT,0x37), IMAGE(CHAR_SHIFT,0x38), IMAGE(CHAR_SHIFT,0x1f), IMAGE(CHAR_SHIFT,0x04), IMAGE(CHAR_SHIFT,0x05), IMAGE(CHAR_SHIFT,0x06), IMAGE(CHAR_SHIFT,0x07), IMAGE(CHAR_SHIFT,0x08), // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,0x09), IMAGE(CHAR_SHIFT,0x0a), IMAGE(CHAR_SHIFT,0x0b), IMAGE(CHAR_SHIFT,0x0c), IMAGE(CHAR_SHIFT,0x0d), IMAGE(CHAR_SHIFT,0x0e), IMAGE(CHAR_SHIFT,0x0f), IMAGE(CHAR_SHIFT,0x10), IMAGE(CHAR_SHIFT,0x11), IMAGE(CHAR_SHIFT,0x12), // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,0x13), IMAGE(CHAR_SHIFT,0x14), IMAGE(CHAR_SHIFT,0x15), IMAGE(CHAR_SHIFT,0x16), IMAGE(CHAR_SHIFT,0x17), IMAGE(CHAR_SHIFT,0x18), IMAGE(CHAR_SHIFT,0x19), IMAGE(CHAR_SHIFT,0x1a), IMAGE(CHAR_SHIFT,0x1b), IMAGE(CHAR_SHIFT,0x1c), // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,0x1d), 0x2f, 0x31, 0x30, IMAGE(CHAR_SHIFT,hid_6), IMAGE(CHAR_SHIFT,0x2d), 0x35, 0x04, 0x05, 0x06, // Z [ \ ] ^ _ ` a b c
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, // d e f g h i j k l m
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, // n o p q r s t u v w
	0x1b, 0x1c, 0x1d, IMAGE(CHAR_SHIFT,0x2f), IMAGE(CHAR_SHIFT,0x31), IMAGE(CHAR_SHIFT,0x30), IMAGE(CHAR_SHIFT,0x35), 0x00, 0x00, 0x00, // x y z { | } ~ Na Na Na

	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
/*	
	0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // F5	  F6	F7	  F8	F9	  F10	F11   F12  PrintScreen ScrollLock
	0x00, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, // Na Pause Insert Home PageUp Delete End PageDown RightArrow LeftArrow
	0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, // DownArrow UpArrow NumLock Keypad Keypad* Keypad- Keypad+ KeypadEnter
	0x77, 0x7a, 0x7b, 0x7c, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, // Select Undo Cut Copy Paste Na Na Na Na Na
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0x00, 0x00, // LeftCtrl LeftCHAR_SHIFT LeftAlt LeftGUI RightCtrl RightCHAR_SHIFT RightAlt RightGUI Na Na
*/
};
#if (ENABLE_MULTI_COUNTRY_KEYBOARD == 1)
static const uint16_t Keytable_USA[KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, hid_back, hid_tab, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, hid_enter, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, hid_esc, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00,		0x00,			hid_space,		IMAGE(CHAR_SHIFT,hid_1),	IMAGE(CHAR_SHIFT,hid_quotation_mark),	IMAGE(CHAR_SHIFT,hid_3),			IMAGE(CHAR_SHIFT,hid_4),	IMAGE(CHAR_SHIFT,hid_5),	IMAGE(CHAR_SHIFT,hid_7),			hid_quotation_mark,	// Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_9), 	IMAGE(CHAR_SHIFT,hid_0),			IMAGE(CHAR_SHIFT,hid_8),			IMAGE(CHAR_SHIFT,hid_equal_mark),	hid_comma,		hid_underline,		hid_full_stop,	hid_solidus_r,hid_0,		hid_1,			// ( ) * + , - . / 0 1	
	hid_2, 	hid_3,			hid_4,			hid_5,			hid_6,			hid_7,				hid_8,			hid_9,		IMAGE(CHAR_SHIFT,hid_semicolon),	hid_semicolon,		// 2 3 4 5 6 7 8 9 : ;
	IMAGE(CHAR_SHIFT,hid_comma),	hid_equal_mark,	IMAGE(CHAR_SHIFT,hid_full_stop),	IMAGE(CHAR_SHIFT,hid_solidus_r),	IMAGE(CHAR_SHIFT,hid_2),		IMAGE(CHAR_SHIFT,hid_a),			IMAGE(CHAR_SHIFT,hid_b),		IMAGE(CHAR_SHIFT,hid_c),		IMAGE(CHAR_SHIFT,hid_d),			IMAGE(CHAR_SHIFT,hid_e),			// < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f), 	IMAGE(CHAR_SHIFT,hid_g),	IMAGE(CHAR_SHIFT,hid_h),			IMAGE(CHAR_SHIFT,hid_i),			IMAGE(CHAR_SHIFT,hid_j),		IMAGE(CHAR_SHIFT,hid_k),			IMAGE(CHAR_SHIFT,hid_l),		IMAGE(CHAR_SHIFT,hid_m),		IMAGE(CHAR_SHIFT,hid_n),			IMAGE(CHAR_SHIFT,hid_o),			// F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p), 	IMAGE(CHAR_SHIFT,hid_q),	IMAGE(CHAR_SHIFT,hid_r),			IMAGE(CHAR_SHIFT,hid_s),			IMAGE(CHAR_SHIFT,hid_t),		IMAGE(CHAR_SHIFT,hid_u),			IMAGE(CHAR_SHIFT,hid_v),		IMAGE(CHAR_SHIFT,hid_w),		IMAGE(CHAR_SHIFT,hid_x),			IMAGE(CHAR_SHIFT,hid_y),			// P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z), 	hid_braces_l,	hid_solidus_l,	hid_braces_r,	IMAGE(CHAR_SHIFT,hid_6),			IMAGE(CHAR_SHIFT,hid_underline),		hid_wave_line,	hid_a,		hid_b,			hid_c,			// Z [ \ ] ^ _ ` a b c
	hid_d, 	hid_e,			hid_f,			hid_g,			hid_h,			hid_i,				hid_j,			hid_k,		hid_l,			hid_m,			// d e f g h i j k l m
	hid_n, 	hid_o,			hid_p,			hid_q,			hid_r,			hid_s,				hid_t,			hid_u,		hid_v,			hid_w,			// n o p q r s t u v w
	hid_x, 	hid_y,			hid_z,			IMAGE(CHAR_SHIFT,hid_braces_l),	IMAGE(CHAR_SHIFT,hid_solidus_l),	IMAGE(CHAR_SHIFT,hid_braces_r),		IMAGE(CHAR_SHIFT,hid_wave_line),	hid_del, 0x00, 0x00,	
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
	/*------------------------------below are not standard ascii--------------------------*/	
#if 0
	hid_f1,hid_f2,hid_f3,hid_f4,hid_f5,hid_f6,hid_f7,hid_f8,hid_f9,hid_f10,
	hid_f11,hid_f12,IMAGE(0x02,0xF1),IMAGE(0x02,0xF8),IMAGE(0x20,0xF1),IMAGE(0x20,0xF8),IMAGE(0x04,0xF1),IMAGE(0x04,0xF8),IMAGE(0x40,0xF1),IMAGE(0x40,0xF8),
	IMAGE(0x01,0xF1),IMAGE(0x01,0xF8),IMAGE(0x10,0xF1),IMAGE(0x10,0xF8),hid_nk_div,hid_nk_asterisk,hid_nk_sub,hid_nk_plus,hid_nk_del,hid_nk_enter,
	hid_nk_0,hid_nk_1,hid_nk_2,hid_nk_3,hid_nk_4,hid_nk_5,hid_nk_6,hid_nk_7,hid_nk_8,hid_nk_9,
	hid_ins,hid_del,hid_home,hid_end,hid_page_up,hid_page_down,hid_up,hid_down,hid_left,hid_right,
	0x00,hid_ins,hid_del,hid_home,hid_end,hid_page_up,hid_page_down,hid_up,hid_down,hid_left,
	hid_right,0x00,hid_nk_num,hid_caps,hid_scroll,
#endif
};
static const uint16_t Keytable_Turkey[KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space,	   IMAGE(CHAR_SHIFT,hid_1),hid_wave_line,IMAGE(CHAR_ALT,hid_3),IMAGE(CHAR_ALT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),IMAGE(CHAR_SHIFT,hid_2),  // Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),hid_underline,IMAGE(CHAR_SHIFT,hid_4),hid_solidus_l,hid_equal_mark,hid_solidus_r,IMAGE(CHAR_SHIFT,hid_7),hid_0,hid_1,		   // ( ) * + , - . / 0 1  
	hid_2,	 	hid_3,		   hid_4,		   hid_5,		   hid_6,		   hid_7,			   hid_8,		   hid_9,IMAGE(CHAR_SHIFT,hid_solidus_r),IMAGE(CHAR_SHIFT,hid_solidus_l),	   // 2 3 4 5 6 7 8 9 : ;
	IMAGE(CHAR_ALT,hid_wave_line),IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_ALT,hid_1),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_q),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),		   // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),		   // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),		   // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z),IMAGE(CHAR_ALT,hid_8),  IMAGE(CHAR_ALT,hid_underline),IMAGE(CHAR_ALT,hid_9),IMAGE(CHAR_SHIFT,hid_3),IMAGE(CHAR_SHIFT,hid_equal_mark),IMAGE(CHAR_ALT,hid_solidus_l),hid_a,	   hid_b,		   hid_c,		   // Z [ \ ] ^ _ ` a b c
	hid_d,	   hid_e,		   hid_f,		   hid_g,		   hid_h,		   hid_quotation_mark, hid_j,		   hid_k,	   hid_l,		   hid_m,		   // d e f g h i j k l m
	hid_n,	   hid_o,		   hid_p,		   hid_q,		   hid_r,		   hid_s,			   hid_t,		   hid_u,	   hid_v,		   hid_w,		   // n o p q r s t u v w
	hid_x,	   hid_y,		   hid_z,		   IMAGE(CHAR_ALT,hid_7),IMAGE(CHAR_ALT,hid_equal_mark),IMAGE(CHAR_ALT,hid_0),IMAGE(CHAR_ALT,hid_braces_r),hid_del, 0x00, 0x00,		   // x y z { | } ~ DEL Na Na	
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};

static const uint16_t Keytable_Spain[KEYTABLE_MAX_SIZE] = 
{
 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space,IMAGE(CHAR_SHIFT,hid_1),IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_ALT,hid_3),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),hid_quotation_mark,  // Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),IMAGE(CHAR_SHIFT,hid_braces_r),hid_braces_r,hid_comma,hid_solidus_r,hid_full_stop,IMAGE(CHAR_SHIFT,hid_7),hid_0,hid_1,		   // ( ) * + , - . / 0 1  
	hid_2,	   hid_3,		   hid_4,		   hid_5,		   hid_6,		   hid_7,			   hid_8,		   hid_9,	   IMAGE(CHAR_SHIFT,hid_full_stop),  IMAGE(CHAR_SHIFT,hid_comma),	   // 2 3 4 5 6 7 8 9 : ;
	hid_k45,   IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_SHIFT,hid_k45),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_2),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),		   // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),		   // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),		   // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z),IMAGE(CHAR_ALT,hid_braces_l),IMAGE(CHAR_ALT,hid_wave_line),IMAGE(CHAR_ALT,hid_braces_r),IMAGE(CHAR_SHIFT,hid_braces_l),IMAGE(CHAR_SHIFT,hid_solidus_r),hid_braces_l,   hid_a,	   hid_b,		   hid_c,		   // Z [ \ ] ^ _ ` a b c
	hid_d,	   hid_e,		   hid_f,		   hid_g,		   hid_h,		   hid_i,			   hid_j,		   hid_k,	   hid_l,		   hid_m,		   // d e f g h i j k l m
	hid_n,	   hid_o,		   hid_p,		   hid_q,		   hid_r,		   hid_s,			   hid_t,		   hid_u,	   hid_v,		   hid_w,		   // n o p q r s t u v w
	hid_x,	   hid_y,		   hid_z,		   IMAGE(CHAR_ALT,hid_quotation_mark),IMAGE(CHAR_ALT,hid_1),IMAGE(CHAR_ALT,hid_solidus_l),IMAGE(CHAR_ALT,hid_4),      hid_del,  0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};

static const uint16_t Keytable_Portugal[KEYTABLE_MAX_SIZE] = 
{
 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space,IMAGE(CHAR_SHIFT,hid_1),IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_SHIFT,hid_3),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),hid_underline,  // Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),IMAGE(CHAR_SHIFT,hid_braces_l),   hid_braces_l,   hid_comma,	   hid_solidus_r,	   hid_full_stop,  IMAGE(CHAR_SHIFT,hid_7),	   hid_0,		   hid_1,		   // ( ) * + , - . / 0 1  
	hid_2,	   hid_3,		   hid_4,		   hid_5,		   hid_6,		   hid_7,			   hid_8,		   hid_9,IMAGE(CHAR_SHIFT,hid_full_stop),IMAGE(CHAR_SHIFT,hid_comma),	   // 2 3 4 5 6 7 8 9 : ;
	hid_k45,   IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_SHIFT,hid_k45),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_2),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),		   // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),		   // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),		   // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z),IMAGE(CHAR_ALT,hid_8),hid_wave_line,IMAGE(CHAR_ALT,hid_9),IMAGE(CHAR_SHIFT,hid_solidus_l),IMAGE(CHAR_SHIFT,hid_solidus_r),IMAGE(CHAR_SHIFT,hid_braces_r),   hid_a,	   hid_b,		   hid_c,		   // Z [ \ ] ^ _ ` a b c
	hid_d,	   hid_e,		   hid_f,		   hid_g,		   hid_h,		   hid_i,			   hid_j,		   hid_k,	   hid_l,		   hid_m,		   // d e f g h i j k l m
	hid_n,	   hid_o,		   hid_p,		   hid_q,		   hid_r,		   hid_s,			   hid_t,		   hid_u,	   hid_v,		   hid_w,		   // n o p q r s t u v w
	hid_x,	   hid_y,		   hid_z,		   IMAGE(CHAR_ALT,hid_7),IMAGE(CHAR_SHIFT,hid_wave_line),IMAGE(CHAR_ALT,hid_0),	hid_solidus_l,  hid_del, 0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};
static const uint16_t Keytable_France[KEYTABLE_MAX_SIZE] =
{
 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space,	   hid_solidus_r,    hid_3,		   IMAGE(CHAR_ALT,hid_3), 			hid_braces_r,   IMAGE(CHAR_SHIFT,hid_quotation_mark),	   	hid_1,		   hid_4,  // Na Na space ! " # $ % & '
	hid_5,	   hid_underline,  hid_k42,        IMAGE(CHAR_SHIFT,hid_equal_mark),   hid_m,	       hid_6,	   		IMAGE(CHAR_SHIFT,hid_comma),IMAGE(CHAR_SHIFT,hid_full_stop),IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_SHIFT,hid_1),		   // ( ) * + , - . / 0 1  
	IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_SHIFT,hid_3),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),IMAGE(CHAR_SHIFT,hid_7),IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),hid_full_stop, hid_comma,	   // 2 3 4 5 6 7 8 9 : ;
	hid_k45,   hid_equal_mark, IMAGE(CHAR_SHIFT,hid_k45),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_ALT,hid_0),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),		   // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_semicolon),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),		   // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_z),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),		   // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_ALT,hid_5),IMAGE(CHAR_ALT,hid_8),IMAGE(CHAR_ALT,hid_underline),IMAGE(CHAR_ALT,hid_9),        hid_8,IMAGE(CHAR_ALT,hid_7),		   	hid_q,	   				hid_b,		   hid_c,		   // Z [ \ ] ^ _ ` a b c
	hid_d,	   hid_e,		   hid_f,		   hid_g,		     hid_h,		   hid_i,			hid_j,		   	hid_k,	   				hid_l,		   hid_semicolon,		   // d e f g h i j k l m
	hid_n,	   hid_o,		   hid_p,		   hid_a,		     hid_r,		   hid_s,			hid_t,		   	hid_u,	   				hid_v,		   hid_z,		   // n o p q r s t u v w
	hid_x,	   hid_y,		   hid_w,		   IMAGE(CHAR_ALT,hid_4),IMAGE(CHAR_ALT,hid_6),IMAGE(CHAR_ALT,hid_equal_mark),IMAGE(CHAR_ALT,hid_2),   		hid_del,    0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};
static const uint16_t Keytable_Germany[KEYTABLE_MAX_SIZE] = 
{
 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space,	   IMAGE(CHAR_SHIFT,hid_1),IMAGE(CHAR_SHIFT,hid_2),hid_k42,IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),IMAGE(CHAR_SHIFT,hid_k42),  // Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),IMAGE(CHAR_SHIFT,hid_braces_r),   hid_braces_r,   hid_comma,	   hid_solidus_r,	   hid_full_stop,IMAGE(CHAR_SHIFT,hid_7),	   hid_0,		   hid_1,		   // ( ) * + , - . / 0 1  
	hid_2,	   hid_3,		   hid_4,		   hid_5,		   hid_6,		   hid_7,			   hid_8,		   hid_9,IMAGE(CHAR_SHIFT,hid_full_stop),IMAGE(CHAR_SHIFT,hid_comma),	   // 2 3 4 5 6 7 8 9 : ;
	hid_k45,IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_SHIFT,hid_k45),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_q),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),		   // < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),		   // F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_z),		   // P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_y),IMAGE(CHAR_ALT,hid_8),IMAGE(CHAR_ALT,hid_underline),IMAGE(CHAR_ALT,hid_9),hid_wave_line,IMAGE(CHAR_SHIFT,hid_solidus_r),IMAGE(CHAR_SHIFT,hid_equal_mark),		   hid_a,	   hid_b,		   hid_c,		   // Z [ \ ] ^ _ ` a b c
	hid_d,	   hid_e,		   hid_f,		   hid_g,		   hid_h,		   hid_i,			   hid_j,		   hid_k,	   hid_l,		   hid_m,		   // d e f g h i j k l m
	hid_n,	   hid_o,		   hid_p,		   hid_q,		   hid_r,		   hid_s,			   hid_t,		   hid_u,	   hid_v,		   hid_w,		   // n o p q r s t u v w
	hid_x,	   hid_z,		   hid_y,		   IMAGE(CHAR_ALT,hid_7),IMAGE(CHAR_ALT,hid_k45),IMAGE(CHAR_ALT,hid_0),IMAGE(CHAR_ALT,hid_braces_r),   hid_del,   0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};
static const uint16_t Keytable_Italy[KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space, 		IMAGE(CHAR_SHIFT,hid_1),IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_ALT,hid_quotation_mark),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6), 			hid_underline, 	// Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),IMAGE(CHAR_SHIFT,hid_braces_r),	hid_braces_r,	hid_comma,		hid_solidus_r,		hid_full_stop,	IMAGE(CHAR_SHIFT,hid_7),		hid_0,			hid_1,			// ( ) * + , - . / 0 1  
	hid_2,		hid_3,			hid_4,			hid_5,			hid_6,			hid_7,				hid_8,			hid_9,		IMAGE(CHAR_SHIFT,hid_full_stop),	IMAGE(CHAR_SHIFT,hid_comma), 		// 2 3 4 5 6 7 8 9 : ;
	hid_k45,	IMAGE(CHAR_SHIFT,hid_0),IMAGE(CHAR_SHIFT,hid_k45),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_semicolon),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),			// < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),			// F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),			// P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z),IMAGE(CHAR_ALT,hid_braces_l),hid_wave_line,IMAGE(CHAR_ALT,hid_braces_r),IMAGE(CHAR_SHIFT,hid_equal_mark),IMAGE(CHAR_SHIFT,hid_solidus_r),		0x00,hid_a,		hid_b,			hid_c,			// Z [ \ ] ^ _ ` a b c
	hid_d,		hid_e,			hid_f,			hid_g,			hid_h,			hid_i,				hid_j,			hid_k,		hid_l,			hid_m,			// d e f g h i j k l m
	hid_n,		hid_o,			hid_p,			hid_q,			hid_r,			hid_s,				hid_t,			hid_u,		hid_v,			hid_w, 			// n o p q r s t u v w
	hid_x,		hid_y,			hid_z,			0x00,			IMAGE(CHAR_SHIFT,hid_wave_line),	0x00,				0x00,			hid_del,	0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};

static const uint16_t Keytable_Czech[KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space, 		IMAGE(CHAR_SHIFT,hid_quotation_mark),IMAGE(CHAR_SHIFT,hid_semicolon),IMAGE(CHAR_ALT,hid_x),IMAGE(CHAR_ALT,hid_semicolon),IMAGE(CHAR_SHIFT,hid_underline),IMAGE(CHAR_ALT,hid_c), IMAGE(CHAR_SHIFT,hid_solidus_l), 	// Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_braces_r),hid_braces_r,IMAGE(CHAR_ALT,hid_solidus_r),	hid_1,	hid_comma,		hid_solidus_r,		hid_full_stop,	IMAGE(CHAR_SHIFT,hid_braces_l),		IMAGE(CHAR_SHIFT,hid_0),	IMAGE(CHAR_SHIFT,hid_1),			// ( ) * + , - . / 0 1  
	IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_SHIFT,hid_3),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),IMAGE(CHAR_SHIFT,hid_7),IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),		IMAGE(CHAR_SHIFT,hid_full_stop),	hid_wave_line, 		// 2 3 4 5 6 7 8 9 : ;
	IMAGE(CHAR_ALT,hid_comma),	hid_underline,IMAGE(CHAR_ALT,hid_full_stop),IMAGE(CHAR_SHIFT,hid_comma),IMAGE(CHAR_ALT,hid_v),IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),			// < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),			// F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_z),			// P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_y),IMAGE(CHAR_ALT,hid_f),IMAGE(CHAR_ALT,hid_q),IMAGE(CHAR_ALT,hid_g),IMAGE(CHAR_ALT,hid_3),IMAGE(CHAR_SHIFT,hid_solidus_r),IMAGE(CHAR_ALT,hid_7),hid_a,		hid_b,			hid_c,			// Z [ \ ] ^ _ ` a b c
	hid_d,		hid_e,			hid_f,			hid_g,			hid_h,			hid_i,				hid_j,			hid_k,		hid_l,			hid_m,			// d e f g h i j k l m
	hid_n,		hid_o,			hid_p,			hid_q,			hid_r,			hid_s,				hid_t,			hid_u,		hid_v,			hid_w, 			// n o p q r s t u v w
	hid_x,		hid_z,			hid_y,			IMAGE(CHAR_ALT,hid_b),		IMAGE(CHAR_ALT,hid_w),	IMAGE(CHAR_ALT,hid_n),	IMAGE(CHAR_ALT,hid_1),			hid_del,	0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};

static const uint16_t Keytable_Japan[KEYTABLE_MAX_SIZE] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Na Na Na Na Na Na Na Na Na Na
	0x00, 0x00,
	hid_space, 	IMAGE(CHAR_SHIFT,hid_1),IMAGE(CHAR_SHIFT,hid_2),IMAGE(CHAR_SHIFT,hid_3),IMAGE(CHAR_SHIFT,hid_4),IMAGE(CHAR_SHIFT,hid_5),IMAGE(CHAR_SHIFT,hid_6),IMAGE(CHAR_SHIFT,hid_7),// Na Na space ! " # $ % & '
	IMAGE(CHAR_SHIFT,hid_8),IMAGE(CHAR_SHIFT,hid_9),IMAGE(CHAR_SHIFT,hid_quotation_mark),IMAGE(CHAR_SHIFT,hid_semicolon),hid_comma,	hid_underline,	hid_full_stop,	hid_solidus_r,	hid_0,	hid_1,			// ( ) * + , - . / 0 1  
	hid_2,		hid_3,		hid_4,		hid_5,		hid_6,		hid_7,		hid_8,		hid_9,		hid_quotation_mark,			hid_semicolon, 		// 2 3 4 5 6 7 8 9 : ;
	IMAGE(CHAR_SHIFT,hid_comma),IMAGE(CHAR_SHIFT,hid_underline)	,IMAGE(CHAR_SHIFT,hid_full_stop),IMAGE(CHAR_SHIFT,hid_solidus_r),hid_braces_l,IMAGE(CHAR_SHIFT,hid_a),IMAGE(CHAR_SHIFT,hid_b),IMAGE(CHAR_SHIFT,hid_c),IMAGE(CHAR_SHIFT,hid_d),IMAGE(CHAR_SHIFT,hid_e),			// < = > ? @ A B C D E
	IMAGE(CHAR_SHIFT,hid_f),IMAGE(CHAR_SHIFT,hid_g),IMAGE(CHAR_SHIFT,hid_h),IMAGE(CHAR_SHIFT,hid_i),IMAGE(CHAR_SHIFT,hid_j),IMAGE(CHAR_SHIFT,hid_k),IMAGE(CHAR_SHIFT,hid_l),IMAGE(CHAR_SHIFT,hid_m),IMAGE(CHAR_SHIFT,hid_n),IMAGE(CHAR_SHIFT,hid_o),			// F G H I J K L M N O
	IMAGE(CHAR_SHIFT,hid_p),IMAGE(CHAR_SHIFT,hid_q),IMAGE(CHAR_SHIFT,hid_r),IMAGE(CHAR_SHIFT,hid_s),IMAGE(CHAR_SHIFT,hid_t),IMAGE(CHAR_SHIFT,hid_u),IMAGE(CHAR_SHIFT,hid_v),IMAGE(CHAR_SHIFT,hid_w),IMAGE(CHAR_SHIFT,hid_x),IMAGE(CHAR_SHIFT,hid_y),			// P Q R S T U V W X Y
	IMAGE(CHAR_SHIFT,hid_z),hid_braces_r,0x87,hid_solidus_l,hid_equal_mark,IMAGE(CHAR_SHIFT,0x87),IMAGE(CHAR_SHIFT,hid_braces_l),hid_a,		hid_b,			hid_c,			// Z [ \ ] ^ _ ` a b c
	hid_d,		hid_e,			hid_f,			hid_g,			hid_h,			hid_i,				hid_j,			hid_k,		hid_l,			hid_m,			// d e f g h i j k l m
	hid_n,		hid_o,			hid_p,			hid_q,			hid_r,			hid_s,				hid_t,			hid_u,		hid_v,			hid_w, 			// n o p q r s t u v w
	hid_x,		hid_y,			hid_z,			IMAGE(CHAR_SHIFT,hid_braces_r),	IMAGE(CHAR_SHIFT,0x89),	IMAGE(CHAR_SHIFT,hid_solidus_l),IMAGE(CHAR_SHIFT,hid_equal_mark),	hid_del,	0x00, 0x00,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, // Return Escape Backspace Tab Space CapsLock F1 F2 F3 F4
};
#endif

#endif
#endif

