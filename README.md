# VSCode上のPlatformIO環境でM5ATOMの開発をするための雛形

M5Stack社のM5ATOMシリーズ（[Matrix](https://docs.m5stack.com/en/core/atom_matrix)／[Lite](https://docs.m5stack.com/en/core/atom_lite)）のプログラムをVSCode上のPlatformIO環境で書くための雛形です。

## ビルドの設定

`platformio.ino`の`[env]`セクションにある`build_flags`で`ENABLE_SERIAL_MONITOR`を定義すると，`common.h`にあるシリアル周りのマクロ定義が有効になり，シリアル表示のコードを挿入します。また，MACアドレスを表示するためのマクロ定義も追加されます。このマクロ定義を使ってデバッグ用の表示コードを書いておくと，不要なときは表示コードを挿入しないようにすることができます。

## ビルド環境の切り替え

VSCodeのステータスバー（画面下）にある「Switch ProjectIO Project Environment」を押すと，環境の切り替えメニューが画面上に表示され，`platformio.ino`の`default_envs`を書き換えずに環境を切り替えることができます。

用意している環境は以下の3つです。

* `m5atom`  
   M5ATOMの最小限の雛形
* `m5atom-wifi`  
   M5ATOMをWiFiに接続する雛形
* `m5atom-ble-keyboard`  
   M5ATOMをBLEキーボードにする雛形  
   M5ATOMのボタンを押すと`SEND_KEY`に指定したキーを押したのと同じ
* `m5atom-espnow`  
   ESPNOWでデータを送受信する雛形
* `m5atom-mqtt`  
   M5ATOMをMQTTクライアントにする雛形。`m5atom-wifi`も必要

## WiFi設定

`m5atom-wifi`でビルドする場合，`data/settings.json`に以下のようにWiFiの設定を書いてSPIFFSに転送することで，M5ATOMのNVS（不揮発記憶装置）にWiFiのSSIDとパスワードを記録し，再起動時にNVSから設定を読み込んでWiFiに接続できるようになります。

```data/settings.json
{
    "ssid": "SSID",
    "password": "Password"
}
```

`settings.json`をM5ATOMのSPIFFSに転送するには，VSCodeのPlatformIOのプロジェクトタスクから「m5atom/Platform/Upload Filesystem」を選びます。

![PlatformIOプロジェクトタスク](https://i.gyazo.com/41b42c0a6a6377485767a8dca70b8d9a.png)

コマンドラインから実行することもできます。以下のコマンドを実行します。

```
$ pio run --target uploadfs
```

M5ATOMのSPIFFSにファイルが転送されると，M5ATOMが再起動します。
M5ATOMはSPIFFSの`/settings.json`を見つけると，JSONに書かれているSSIDとパスワードをNVSに記録し，`settings.json`を削除します。
これ以降は再起動されるたびにNVSに記録された接続情報を利用してWiFiに接続します。

WiFiの設定を変更するには，`settings.json`の内容を変更し，再度M5ATOMにファイルを転送してください。
