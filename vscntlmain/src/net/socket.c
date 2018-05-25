/*
 * socket.c
 *
 *  Created on: 2016/03/22
 *      Author: toshio
 */

#include "socket.h"
#include "tick.h"
#include "datarom.h"


// ソケット時間監視用タイマ200mSec
#define	_SOCK_TIMEOUT	2
#define	_SOCK_TIMER		_ForSockTimeout


// 送信中フラグ
static uint16_t Sock_is_Sending = 0;


bool bSockInit = false;

/*
 * 動的ポートアドレスはLinuxに合わせとく
 */
#define	SOURCE_START_PORT_NO	32768
#define	SOURCE_END_PORT_NO		61000
#define	SOURCE_END_PORT_ADD		1440

uint16_t gSurcePortNumber;



/*
 * コマンド完了待ち
 */
void WaitForCommandComplete(uint8_t sn)
{
	_SOCK_TIMER = _SOCK_TIMEOUT;
	while(true) {
		if (GetSocketCommand(sn) == 0) {
			break;
		}
		if (!_SOCK_TIMER) {
			break;
		}
	}
}


/*
 * コマンド送信マクロ
 */
#define	SEND_SOCKET_COMMAND(sn, com)	{ SetSocketCommand(sn, com); WaitForCommandComplete(sn); }



/*
 * ソース側のポートを取得する
 * 自動インクリメントする(そうしないとサーバー側のKeepAliveで一定期間接続を拒否される)
 * 電源投入時に一度だけ一定値を加算する
 * 例えばサーバーのアライブが１日とし、コントローラが１分周期でアクセスすると仮定すると
 * 24時間 x 60分 = 1440加算すれば大丈夫
 *
 */

uint16_t GetSourcePortAddress(void)
{
	if (!bSockInit) {
		// 電源投入１回めはROMから読む
		gSurcePortNumber = GetCurrentPortNumber();
		// 一定数加算しROMへ保存
		SetCurrentPortNumber(gSurcePortNumber + SOURCE_END_PORT_ADD);
		// 初期化済フラグセット
		bSockInit = true;
	}
	else {
		// ROMから読み込み済みなら加算のみ
		gSurcePortNumber++;
		// 4時間毎にROMから更新する
		if ((gSurcePortNumber & 0xff) == 0x00) {
			bSockInit = false;
		}
	}
	if (gSurcePortNumber < SOURCE_START_PORT_NO || gSurcePortNumber > SOURCE_END_PORT_NO) {
		gSurcePortNumber = SOURCE_START_PORT_NO;
		// ROMへ保存
		SetCurrentPortNumber(gSurcePortNumber);
	}

	return gSurcePortNumber;
}
/*
 * もし接続できなかった場合に備え、ポートアドレスをクリアする
 * 一定数加算する
 * 実際にはbSockInitをクリアするだけ
 * 連続してコールするとEEROMが劣化するので注意
 */
uint16_t ReGetSourcePortAddress(void)
{
	bSockInit = false;
	return GetSourcePortAddress();
}

int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag)
{
	uint8_t rc;

	/*
	 * もしポートがゼロだったら自動取得モード
	 */
	if (port == 0) {
		port = GetSourcePortAddress();
	}

	// 一旦クローズ
	close_socket(sn);
	// モード設定
	SetSocketMode(sn, (protocol | (flag & 0xf0)));
	// ポート番号設定
	SetSocketSourcePort(sn, port);
	// オープンコマンド実行
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_OPEN);

	_SOCK_TIMER = _SOCK_TIMEOUT;
	while(true) {
		rc = GetSocketStatus(sn);
		if (rc != STATUS_SOCK_CLOSED) {
			// クローズ以外であれば成功
			break;
		}
		if (!_SOCK_TIMER) {
			break;
		}
	}

	// 送信中フラグクリア
	Sock_is_Sending &= ~(1 << sn);

	return (int8_t)sn;

}

int8_t close_socket(uint8_t sn)
{
	uint8_t rc;

	rc = GetSocketCommand(sn);
	// クローズコマンド実行
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_CLOSE);

	// 全ての割り込みをクリアする
	// 1を書くとクリアされる
	SetSocketInterrupt(sn, 0xff);

	_SOCK_TIMER = _SOCK_TIMEOUT;
	while(true) {
		rc = GetSocketStatus(sn);
		if (rc == STATUS_SOCK_CLOSED) {
			// クローズであれば成功
			break;
		}
		if (!_SOCK_TIMER) {
			break;
		}
	}

	// 送信中フラグクリア
	Sock_is_Sending &= ~(1 << sn);

	return SOCK_OK;

}

int8_t listen(uint8_t sn)
{
	uint8_t rc;

	// ソケットオープンチェック
	rc = GetSocketStatus(sn);
	if (rc != STATUS_SOCK_INIT) {
		// 初期化状態でなければエラー
			return SOCKERR_SOCKINIT;
	}
	// LISTEN
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_LISTEN);

	_SOCK_TIMER = _SOCK_TIMEOUT;
	while(true) {
		rc = GetSocketStatus(sn);
		if (rc == STATUS_SOCK_LISTEN || rc == STATUS_SOCK_ESTABLISHED) {
			break;
		}
		else if (rc == STATUS_SOCK_CLOSED || rc == STATUS_SOCK_CLOSE_WAIT) {
			close_socket(sn);
			return SOCKERR_SOCKCLOSED;
		}
		if (!_SOCK_TIMER) {
			break;
		}
	}

	return SOCK_OK;
}

typedef struct {
	uint8_t sn;
	uint8_t * addr;
	uint16_t port;
	int8_t Result;
	int8_t RequestStatus;
} SOCK_CONNECT_VALUE, *PSOCK_CONNECT_VALUE;

SOCK_CONNECT_VALUE SOCK_Connect_Value = { 0, 0, 0, 0, SOCK_CONNECT_STATUS_IDLE } ;


int8_t nonBlockingConnect(uint8_t sn, uint8_t * addr, uint16_t port)
{
	uint8_t rc;

	switch(SOCK_Connect_Value.RequestStatus) {
		case SOCK_CONNECT_STATUS_IDLE:
		case SOCK_CONNECT_COMPLETE:
		case SOCK_CONNECT_ABORT:
		{
			// パラメータ保存
			SOCK_Connect_Value.RequestStatus = SOCK_WAIT_FOR_CONNECT;
			SOCK_Connect_Value.sn		= sn;
			SOCK_Connect_Value.addr		= addr;
			SOCK_Connect_Value.port		= port;
			SOCK_Connect_Value.Result	= 0;

			/*
			 *  ソケットオープンチェック
			 */
			rc = GetSocketStatus(SOCK_Connect_Value.sn);
			if (rc != STATUS_SOCK_INIT) {
				// 初期化状態でなければエラー
				SOCK_Connect_Value.Result 			= SOCKERR_SOCKINIT;
				SOCK_Connect_Value.RequestStatus	= SOCK_CONNECT_ABORT;
				return SOCK_Connect_Value.RequestStatus;
			}

			/*
			 * 接続先IP
			 * 接続先ポート
			 * 接続！
			 */
			SetSocketDestnationIP(SOCK_Connect_Value.sn, addr);
			SetSocketDestnationPort(SOCK_Connect_Value.sn, port);

			SEND_SOCKET_COMMAND(SOCK_Connect_Value.sn, COMMAND_SOCK_CONNECT);

			/*
			 * ブレークせずに接続ステータスを確認する
			 */
		}
		case SOCK_WAIT_FOR_CONNECT:
		{
			/*
			 * ステータスチェック
			 */
			rc = GetSocketStatus(SOCK_Connect_Value.sn);
			if (rc == STATUS_SOCK_ESTABLISHED) {
				/*
				 * 確立！
				 */
				SOCK_Connect_Value.Result 			= SOCK_OK;
				SOCK_Connect_Value.RequestStatus	= SOCK_CONNECT_COMPLETE;
				return SOCK_Connect_Value.RequestStatus;
			}
			else if (rc == STATUS_SOCK_CLOSED) {
				/*
				 * 拒否
				 */
				SOCK_Connect_Value.Result 			= SOCKERR_SOCKCLOSED;
				SOCK_Connect_Value.RequestStatus	= SOCK_CONNECT_ABORT;
				return SOCK_Connect_Value.RequestStatus;
			}
			/*
			 * タイムアウトチェック
			 */
			rc = GetSocketInterrupt(SOCK_Connect_Value.sn);
			if (rc & Sn_IR_TIMEOUT) {
				/*
				 * タイムアウト
				 */
				SetSocketInterrupt(SOCK_Connect_Value.sn, Sn_IR_TIMEOUT);
				SOCK_Connect_Value.Result 			= SOCKERR_TIMEOUT;
				SOCK_Connect_Value.RequestStatus	= SOCK_CONNECT_ABORT;
				return SOCK_Connect_Value.RequestStatus;
			}
		}
	}

	return SOCK_Connect_Value.RequestStatus;
}



typedef struct {
	uint8_t sn;
	int8_t Result;
	int8_t RequestStatus;
} SOCK_DISCONNECT_VALUE, *PSOCK_DISCONNECT_VALUE;

SOCK_DISCONNECT_VALUE SOCK_Disconnect_Value = { 0, 0, SOCK_DISCONNECT_STATUS_IDLE } ;

int8_t nonBlockingDisconnect(uint8_t sn)
{
	uint8_t rc;

	switch(SOCK_Disconnect_Value.RequestStatus) {
		case SOCK_DISCONNECT_STATUS_IDLE:
		case SOCK_DISCONNECT_COMPLETE:
		case SOCK_DISCONNECT_ABORT:
		{
			SOCK_Disconnect_Value.sn		= sn;
			SOCK_Disconnect_Value.Result	= 0;

			/*
			 * 切断
			 */
			SEND_SOCKET_COMMAND(SOCK_Disconnect_Value.sn, COMMAND_SOCK_DISCON);

			/*
			 * 次は切断待ちステータス
			 */
			SOCK_Disconnect_Value.RequestStatus = SOCK_WAIT_FOR_DISCONNECT;

		}
		case SOCK_WAIT_FOR_DISCONNECT:
		{
			rc = GetSocketStatus(sn);
			if (rc == STATUS_SOCK_CLOSED) {
				/*
				 * 切断完了
				 */
				SOCK_Disconnect_Value.Result		= SOCK_OK;
				SOCK_Disconnect_Value.RequestStatus = SOCK_DISCONNECT_COMPLETE;
				return SOCK_Disconnect_Value.RequestStatus;
			}
			int8_t iSts = GetSocketInterrupt(SOCK_Disconnect_Value.sn);
			if (iSts & Sn_IR_TIMEOUT) {
				/*
				 * タイムアウト
				 */
				SetSocketInterrupt(SOCK_Disconnect_Value.sn, Sn_IR_TIMEOUT);
				SOCK_Disconnect_Value.Result		= SOCKERR_TIMEOUT;
				SOCK_Disconnect_Value.RequestStatus = SOCK_DISCONNECT_ABORT;
			}
		}

	}

	return SOCK_Disconnect_Value.RequestStatus;
}







uint8_t dbc = 0;

uint8_t dbStatus[10];
uint8_t dbIndex = 0;
int8_t connect(uint8_t sn, uint8_t * addr, uint16_t port)
{
	uint8_t rc, rt;

	// ソケットオープンチェック
	rc = GetSocketStatus(sn);
	if (rc != STATUS_SOCK_INIT) {
		// 初期化状態でなければエラー
			return SOCKERR_SOCKINIT;
	}

	// 接続先IP
	SetSocketDestnationIP(sn, addr);
	// 接続先ポート
	SetSocketDestnationPort(sn, port);

	// 接続！
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_CONNECT);

	int i;
	for (i = 0; i < 10; i++) {
		dbStatus[i] = 0xff;
	}
	dbIndex = 0;
	// 接続チェック
	while(true) {
		rc = GetSocketStatus(sn);
		if (dbIndex == 0 || dbStatus[dbIndex - 1] != rc) {
			dbStatus[dbIndex] = rc;
			dbIndex++;
		}
		if (rc == STATUS_SOCK_ESTABLISHED) {
			// 確立！
			break;
		}
		else if (rc == STATUS_SOCK_CLOSED) {
			//
			return SOCKERR_SOCKCLOSED;

		}
		rt = GetSocketInterrupt(sn);
		if (rt & Sn_IR_TIMEOUT) {
			// タイムアウト
			SetSocketInterrupt(sn, Sn_IR_TIMEOUT);
			return SOCKERR_TIMEOUT;
		}
	}

	return SOCK_OK;
}

uint8_t sss;

int8_t disconnect(uint8_t sn)
{
	uint8_t rc;

	// 切断！
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_DISCON);

	// 送信中フラグクリア
	Sock_is_Sending &= ~(1 << sn);

	sss =  GetSocketStatus(sn);
	// 切断チェック
	while(true) {
		rc = GetSocketStatus(sn);
		if (rc == STATUS_SOCK_CLOSED) {
			// 切断！
			SetSocketInterrupt(sn, Sn_IR_DISCON);
			break;
		}
		int8_t iSts = GetSocketInterrupt(sn);
		if (iSts & Sn_IR_TIMEOUT) {
			// タイムアウト
			SetSocketInterrupt(sn, Sn_IR_TIMEOUT);
			return SOCKERR_TIMEOUT;
		}
	}

	return SOCK_OK;
}

int32_t send(uint8_t sn, uint8_t *pBuffer, uint16_t len)
{
	uint8_t rc;
	uint16_t FreeSize;

	rc = GetSocketStatus(sn);

	if (rc != STATUS_SOCK_ESTABLISHED && rc != STATUS_SOCK_CLOSE_WAIT) {
		close_socket(sn);
		// 送信不可能な状態だったらエラー
		return SOCKERR_SOCKSTATUS;
	}
	// 送信中フラグクリア
	Sock_is_Sending &= ~(1 << sn);

	if (Sock_is_Sending & (1 << sn)) {
		// 送信中
		rc = GetSocketInterrupt(sn);
		if (rc & Sn_IR_SENDOK) {
			 SetSocketInterrupt(sn, Sn_IR_SENDOK);
			// 送信中フラグクリア
			Sock_is_Sending &= ~(1 << sn);
		}
		else if (rc & Sn_IR_TIMEOUT) {
			// 送信タイムアウト
			close_socket(sn);
			return SOCKERR_TIMEOUT;
		}
		else {
			return SOCK_BUSY;
		}
	}

	// これから送信です
	FreeSize = GetSocketTransmitBufferSize(sn);
	if (FreeSize < len) {
		// 送信可能最大サイズは送信バッファサイズ
		len = FreeSize;
	}

	while(true) {
		FreeSize = GetSocketSendFreeSize(sn);
		rc = GetSocketStatus(sn);
		if ((rc != STATUS_SOCK_ESTABLISHED) && (rc != STATUS_SOCK_CLOSE_WAIT)) {
			close_socket(sn);
			// 送信不可能な状態だったらエラー
			return SOCKERR_SOCKSTATUS;
		}
		if (len <= FreeSize) {
			// 送信バッファに空きがある
			break;
		}
		else {
			// 前の送信データが残っててバッファが小さいからとりあえず帰る
			return SOCK_BUSY;
		}
	}
	W5500_Send_Data(sn, pBuffer, len);

	// 送信コマンド
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_SEND);

	// 送信中フラグセット
	Sock_is_Sending |= (1 << sn);

	return len;
}


int32_t recv(uint8_t sn, uint8_t *pBuffer, uint16_t len)
{
	uint8_t rc;
	uint16_t RecvSize;

	RecvSize = GetSocketReceiveBufferSize(sn);
	if (len < RecvSize) {

	}
	while(true) {
		// 受信データ数
		RecvSize = GetSocketRXReceivedSize(sn);
		rc = GetSocketStatus(sn);
		if (rc != STATUS_SOCK_ESTABLISHED) {
			if (rc == STATUS_SOCK_CLOSE_WAIT) {
				close_socket(sn);
				// 受信不可能な状態だったらエラー
				return SOCKERR_SOCKSTATUS;
			}
			else {
				close_socket(sn);
				// 受信不可能な状態だったらエラー
				return SOCKERR_SOCKSTATUS;
			}
		}
		if (RecvSize == 0) {
			// まだ受信データがなければとりあえず帰る
			return SOCK_BUSY;
		}
		else {
			break;
		}
	}
	if (RecvSize < len) {
		len = RecvSize;
	}
	W5500_Receive_Data(sn, pBuffer, len);
	// 受信コマンド
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_RECV);

	return len;
}

/*
 * クライアント用の受信
 */
int32_t recv_for_Client(uint8_t sn, uint8_t *pBuffer, uint16_t len)
{
	uint8_t rc;
	uint16_t RecvSize;

	RecvSize = GetSocketReceiveBufferSize(sn);
	if (len < RecvSize) {

	}
	while(true) {
		// 受信データ数
		RecvSize = GetSocketRXReceivedSize(sn);
		rc = GetSocketStatus(sn);
		if (rc != STATUS_SOCK_ESTABLISHED) {
			if (rc == STATUS_SOCK_CLOSE_WAIT) {
				if (RecvSize > 0) {
					break;
				}
				else if (GetSocketReceivedSize(sn) >= GetSocketReceiveBufferSize(sn)) {
					close_socket(sn);
					// 受信不可能な状態だったらエラー
					return SOCKERR_SOCKSTATUS;
				}
			}
			else {
				close_socket(sn);
				// 受信不可能な状態だったらエラー
				return SOCKERR_SOCKSTATUS;
			}
		}
		if (RecvSize == 0) {
			// まだ受信データがなければとりあえず帰る
			return SOCK_BUSY;
		}
		else {
			break;
		}
	}
	if (RecvSize < len) {
		len = RecvSize;
	}
	W5500_Receive_Data(sn, pBuffer, len);
	// 受信コマンド
	SEND_SOCKET_COMMAND(sn, COMMAND_SOCK_RECV);

	return len;
}
