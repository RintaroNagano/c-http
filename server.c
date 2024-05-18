#include <stdio.h>

// exitを使うためのライブラリ
# include <stdlib.h>

// memsetを使うためのライブラリ
#include <string.h>

// addrinfoを使うためのライブラリ
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// dupを使うためのライブラリ
#include <unistd.h>

#define PORTNO_BUFSIZE 16
#define PORTNO 80

// tcpで接続要求を待つソケットを作成して返却する関数
int tcp_listen_port(int portno)
{
    struct addrinfo hints; // 呼び出し元がサポートするソケットのタイプヒント
    struct addrinfo *ai; // address information

    char portno_str[PORTNO_BUFSIZE];
    int err, s;

    /* 自身のネットワーク情報を取得 */
    // hintsに値を入れる
    // port番号をportno_strに書き込んで，文字列として作成
    snprintf(portno_str, sizeof(portno_str), "%d", portno);

    memset(&hints, 0, sizeof(hints)); //getaddr関数の仕様に従いhintsのメモリ領域を0埋めする
    ai = NULL;
    hints.ai_family = PF_INET; // IPv4 のみを処理し、 IPv6を処理しないオプション
    hints.ai_flags = AI_PASSIVE; // bindが返却したsock_addr構造体を使用するオプション
    hints.ai_socktype = SOCK_STREAM; // TCP のみを処理し、UDP は処理しないオプション
    // getaddrinfoを実行
    if ((err = getaddrinfo(NULL, portno_str, &hints, &ai))) {
        fprintf(stderr, "bad portno %d? (%s)\n", portno,
        gai_strerror(err)); // gai_strerrorでエラーコードを読みやすい文字に変換
        return -1;
    }

    // socketを作成
    if ((s = socket(ai->ai_family, ai->ai_socktype,
    ai->ai_protocol)) < 0) {
        perror("socket");
        return -1;
    }

    // IPアドレスとポートをソケットにbindする
    if (bind(s, ai->ai_addr, ai->ai_addrlen) < 0) {
        perror("bind");
        fprintf(stderr, "Port number %d\n", portno);
        return -1;
    }

    // listenで接続要求の受付開始
    if (listen(s, 5) < 0) {
        perror("listen");
        return -1;
    }

    freeaddrinfo(ai); // addrinfo構造体を解放

    return (s); // 接続待ちしているソケットのファイルディスクリプタを返す
}

// FILEポインタをソケットのファイルディスクリプタに関連付ける関数
int fdopen_sock(int sock, FILE **in, FILE **out) {
    int sock_dup = dup(sock);  // dupを使用して、ソケットのコピーを作成
    if (sock_dup < 0) return -1;  // エラー処理

    *in = fdopen(sock, "r");  // 読み込み用に開く
    *out = fdopen(sock_dup, "w");  // 書き込み用に開く
    if (!*in || !*out) return -1;  // エラー処理

    setvbuf(*out, NULL, _IONBF, 0);  // バッファリングを無効化
    return 0;
}

// 簡易的なHTTPリクエスト受信処理
// 受け取ったリクエストを表示する
int http_receive_request(FILE *in) {
    char buf[1024];
    int is_get_request = 0; // GETリクエストかどうかを判定するフラグ
    int first_line = 1; // 最初の行を識別するためのフラグ

    printf("\nHTTPリクエストを受信しました．内容は以下のようなものです．\n\n");

    while (fgets(buf, sizeof(buf), in)) {
        // リクエストラインまたはヘッダー行の末尾の改行を除去
        buf[strcspn(buf, "\r\n")] = '\0';

        // 空行（ヘッダーの終了）を検出した場合、読み取りを終了
        if (strlen(buf) == 0) {
            break;
        }

        // 最初の行でGETリクエストかどうかをチェック
        if (first_line && strncmp(buf, "GET", 3) == 0) {
            is_get_request = 1; // GETリクエストである
            first_line = 0; // 最初の行の処理が終わった
        }

        // 受け取った行を表示
        printf("%s\n", buf);
    }

    printf("\n");

    return is_get_request; // GETリクエストかどうかを返す
}

// 簡易的なHTTPレスポンスを送信
void http_send_reply(FILE *out) {
    fprintf(out, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!\n");
}

// 不正なリクエストに対するHTTP 400 レスポンスを送信
void http_send_reply_bad_request(FILE *out) {
    fprintf(out, "HTTP/1.0 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBad request.\n");
}

void http_server_loop(int portno)
{
    int acc, com;

    acc = tcp_listen_port(portno);
    if (acc < 0) exit(-1);

    while (1) {
        FILE *in, *out;

        printf("Accepting incoming connections...\n");

        // クライアントから通信接続要求が来るまでプログラムを停止
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            exit(-1);
        }

        // socketのファイルディスクリプタから，FILEポインタを作成
        if (fdopen_sock(com, &in, &out) < 0) {
            perror("fdooen()");
            return;
        }
        
        if (http_receive_request(in))
        {
            http_send_reply(out);
        }else{
            http_send_reply_bad_request(out);
        }

        printf("Replied\n");

        fclose(in);
        fclose(out);
    }
}

int main(){
    http_server_loop(PORTNO);
}