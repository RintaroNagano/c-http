# c-http
c言語で作成したhttpサーバとクライアントです．

サーバはシングルスレッドで動作する非常に簡素なもの．
リクエストを一つずつ処理していくので，複数リクエストは待たされます．

# 学んだこと
- tcp/ip通信に使われるシステムコール
- httpのパケット内容
- ネットワークプログラミングの概念
  - アドレスファミリ
  - ソケットタイプ
  - ネットワークバイトオーダーとホストバイトオーダー
