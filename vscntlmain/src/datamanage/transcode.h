/*
 * transcode.h
 *
 *  Created on: 2015/12/08
 *      Author: toshio
 */

#ifndef DATAMANAGE_TRANSCODE_H_
#define DATAMANAGE_TRANSCODE_H_

/*
 * コマンド
 */
#define	_COMMAND_GET		'0'	// リモコンからコントローラへデータ転送
#define	_COMMAND_SET		'1'	// コントローラからリモコンへデータ転送
#define	_COMMAND_POL		'2' // リモコンからコントローラへ要求確認
#define	_COMMAND_ERR		'9' // コントローラーからエラー通知

/*
 * レスポンスコード
 */
#define	_RESPONSE_GET		'A'	// _COMMAND_GETに対する回答
#define	_RESPONSE_SET		'B'	// _COMMAND_SETに対する回答
#define	_RESPONSE_NON		'C'	// _COMMAND_POLに対する回答（特に返すものが無い）
#define	_RESPONSE_REQ		'D'	// _COMMAND_POLに対する回答（返すものが有る。返すものは装飾を参照)
#define	_RESPONSE_WNT		'W'	// _COMMAND_POLに対する回答（データを送って欲しい。送るものはitem コードで示す)
#define	_RESPONSE_INT		'I'	// _COMMAND_POLに対する回答（初期データを送って欲しい。itemコードは無視される)

#define	_RESPONSE_GET_ERR	'a'	// _COMMAND_GETに対するエラー回答
#define	_RESPONSE_SET_ERR	'b'	// _COMMAND_SETに対するエラー回答
#define	_RESPONSE_POL_ERR	'c' // _COMMAND_POLに対するエラー回答

/*
 * コマンドの装飾
 */
// item ナンバー
#define	_ITEM_NO_RUNSTOP		0
#define	_ITEM_NO_RUNMODE		1
#define	_ITEM_NO_BASE			2
#define	_ITEM_NO_ONCHO			3
#define	_ITEM_NO_ONDOSETTEI		4
#define	_ITEM_NO_SUII			5
#define	_ITEM_NO_NET			6
#define	_ITEM_NO_TERMNO			7
#define	_ITEM_NO_CLOCK			8
#define	_ITEM_NO_STATE			9
#define	_ITEM_NO_SET_PRESS		10
#define	_ITEM_NO_SET_ALARM		11
#define	_ITEM_NO_INITIAL_A		12
#define	_ITEM_NO_INITIAL_B		13
/*
 * Version 1.05 2016/12/10 T.Ogawa
 */
#define	_ITEM_NO_SUPPLYMON		14
#define	_ITEM_NO_DRIVETIME		15

/*
 * Version 1.06 2016/12/23 T.Ogawa
 */
#define	_ITEM_NO_RC				16
#define	_ITEM_NO_INITIAL_C		17

/*
 * Version 1.06L 2018/02/09 T.Ogawa
 */
#define	_ITEM_NO_FWS_OFF		18
#define	_ITEM_NO_FWS_ON			19

//#define	_ITEM_NO_MAX			14
//#define	_ITEM_NO_MAX			16
//#define	_ITEM_NO_MAX			18
#define	_ITEM_NO_MAX			20

// item コード
#define	_CODE_RUNSTOP		(_ITEM_NO_RUNSTOP + '0')
#define	_CODE_RUNMODE		(_ITEM_NO_RUNMODE + '0')
#define	_CODE_BASE			(_ITEM_NO_BASE + '0')
#define	_CODE_ONCHO			(_ITEM_NO_ONCHO + '0')
#define	_CODE_ONDOSETTEI	(_ITEM_NO_ONDOSETTEI + '0')
#define	_CODE_SUII			(_ITEM_NO_SUII + '0')
#define	_CODE_NET			(_ITEM_NO_NET + '0')
#define	_CODE_TERMNO		(_ITEM_NO_TERMNO + '0')
#define	_CODE_CLOCK			(_ITEM_NO_CLOCK + '0')
#define	_CODE_STATE			(_ITEM_NO_STATE + '0')
#define	_CODE_SET_PRESS		(_ITEM_NO_SET_PRESS + '0')
#define	_CODE_SET_ALARM		(_ITEM_NO_SET_ALARM + '0')
#define	_CODE_INITIAL_A		(_ITEM_NO_INITIAL_A + '0')
#define	_CODE_INITIAL_B		(_ITEM_NO_INITIAL_B + '0')

/*
 * Version 1.05 2016/12/10 T.Ogawa
 */
#define	_CODE_SUPPLYMON		(_ITEM_NO_SUPPLYMON + '0')
#define	_CODE_DRIVETIME		(_ITEM_NO_DRIVETIME + '0')

/*
 * Version 1.06 2016/12/23 T.Ogawa
 */
#define	_CODE_RC			(_ITEM_NO_RC + '0')
#define	_CODE_INITIAL_C		(_ITEM_NO_INITIAL_C + '0')

/*
 * Version 1.06L 2018/02/09 T.Ogawa
 */
#define	_CODE_FWS_OFF		(_ITEM_NO_FWS_OFF + '0')
#define	_CODE_FWS_ON		(_ITEM_NO_FWS_ON + '0')


#endif /* DATAMANAGE_TRANSCODE_H_ */
