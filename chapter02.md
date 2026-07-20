# 様々なライブラリたち

本章では、どのようなファイル変更監視ライブラリがあるかを、プログラミング言語毎に見ていきたいと思います。後半では、特定のプログラミング言語に依存しないライブラリについても見ていきます。

## Node.js

Node.jsは言語本体でファイル変更監視の為のAPIが提供されています。提供されているのは`fs.watchFile`と`fs.watch`という2つのAPIです。

//list[fs.watchFile][fs.watchFile]{
fs.watchFile(filename[, options], listener)
//}

//list[fs.watch][fs.watch]{
fs.watch(filename[, options][, listener])
//}

引数はどちらも同じですね。引数だけ見ると違いがよくわからないのですが、`fs.watchFile`の方はポーリングによる実装になっており、指定出来るのはファイルだけになっています。対して、`fs.watch`はOSが提供しているAPIを使用した実装となっており、こちらはファイル・ディレクトリの両方が指定出来るようになっています。

当然OSが提供しているAPIを使用した方が情報が正確に取得出来る為、公式のドキュメントにも、可能な限り、`fs.watch`の方を使用する事が推奨されています[^fs.watch.doc]
[^fs.watch.doc]: https://nodejs.org/docs/latest/api/fs.html#fs_fs_watchfile_filename_options_listener

前章で説明した通り、OSが提供しているAPIには、OS毎に差異があります。`fs.watch`はAPIの差異の吸収は行わず、OSが提供しているAPIをそのまま使用する、という方針を取っているようです[^fswatch_caution]。使用しているAPIは下記の通りです。
[^fswatch_caution]: `fs.watch`のドキュメントの注意事項にも、"The fs.watch API is not 100% consistent across platforms, and is unavailable in some situations."との記載があります。

例えば、ファイル名の取得精度です。Linux（inotify）、Windows（ReadDirectoryChangesW）では変更されたファイル名が正確に取得出来ますが、macOS（FSEvents）はディレクトリベースの通知の為、変更されたファイル名が取得出来ない事があります。

なお、上記のような説明があるものの、少しづつ方針が変わっているようです。例えば、以前は再帰的な監視処理がLinuxでは動作しない、という問題がありましたが、現在はこの問題は解消されており[^recursive_linux]、影響が大きい差異についてはAPIでOSの差異の吸収を行うようになっているようです。
[^recursive_linux]: https://github.com/nodejs/node/pull/45098


| OS | 使用しているAPI |
| --- | --- |
| Linux | inotify |
| macOS | kqueue、FSEvents |
| Windows | ReadDirectoryChangesW |

### chokidar

先に述べたように、Node.jsは言語本体でファイル変更監視の為のAPIが提供されています。しかし、OS間で差異がある為、特に古いバージョンではそれだけを使用するのは中々困難だったようです。
そのためか、多くのNode.jsプロジェクトでは[chokidar](https://github.com/paulmillr/chokidar)というライブラリが使用されています。chokidarは、`fs.watch`の上位互換として設計されており、クロスプラットフォームでの一貫した動作を提供します。2012年に、Brunchというビルドツールの一部として切り出される形で誕生しました。

chokidarでは、OS間の差異を吸収し、すべてのプラットフォームで一貫したイベント通知とファイル名の取得を提供し、重複イベントやエラーなどを内部で対象するようになっています。監視対象の`FSWatcher`は、`add`（ファイル追加）、`addDir`（ディレクトリ追加）、`change`（変更）、`unlink`（ファイル削除）、`unlinkDir`（ディレクトリ削除）、`ready`（初回スキャン完了）といったイベントを発行し、アプリケーション側はこれらを個別に、あるいは`all`イベントとしてまとめて購読出来ます。

代表的なオプションに`atomic`と`awaitWriteFinish`があります。`atomic`は、多くのエディタが行う「一時ファイルへ書き込んでから元のファイル名にリネームする」という保存方法によって発生する削除・作成イベントの組を、1つの`change`イベントにまとめてくれる機能です。`awaitWriteFinish`は、ファイルサイズの変化が一定時間止まるまでイベントの発行を保留する機能で、大きなファイルが分割して書き込まれるようなケースに対応する為のものです。

なお、v4では、macOS向けにバンドルされていたネイティブの`fsevents`依存とglobサポートが削除され、ネイティブの監視処理は完全にNode.jsの`fs.watch`任せになっています。

## Go

Goでは、[fsnotify/fsnotify](https://github.com/fsnotify/fsnotify)が事実上の標準ライブラリとして使用されています。`fsnotify.NewWatcher()`が`Events`と`Errors`という2つのチャネルを持つ`Watcher`を返し、`watcher.Add(path)`で監視対象のパスを追加する、というシンプルなAPIになっています。

対応状況はOSによって差があります。Linux（inotify）、BSD系及びmacOS（kqueue）、Windows（ReadDirectoryChangesW）、illumos（FEN）は対応済みですが、fanotifyやmacOSネイティブのFSEvents、USNジャーナル、汎用的なポーリングフォールバックは、本書執筆時点ではまだ実装されていません。また、chokidarのようなライブラリ側での再帰監視のサポートは無く、サブディレクトリを監視したい場合は、利用する側でディレクトリツリーを辿って個別に`Add()`を呼び出す必要があります。

macOSでは、ネイティブのFSEventsではなく前章で説明したkqueueが使われています。kqueueは監視対象のファイル毎にファイルディスクリプタを1つ消費するため、多数のファイルを含むディレクトリを監視すると、OSのファイルディスクリプタ数上限に達しやすいという制約を、Goで書かれたアプリケーションもそのまま引き継ぐ事になります。

## Rust

Rustでは、[notify-rs/notify](https://github.com/notify-rs/notify)が広く使われています。rust-analyzerやzed、watchexec、cargo-watch、mdBookといった著名なプロジェクトで採用されています。

対応しているバックエンドは、Linuxはinotify、WindowsはReadDirectoryChangesW、その他のBSD系向けkqueueで、全プラットフォーム向けにポーリングのフォールバックも用意されています。特徴的なのは、macOSではFSEventsとkqueueのどちらを使うかをCargoのフィーチャーフラグでビルド時に選択出来るようになっている点です。本書で紹介している他のライブラリはOS毎にどちらか一方を内部で決め打ちしていますが、notify-rsは利用者側に選択肢を委ねています。

また、デバウンス（短時間に連続する変更をまとめる）機能はコアの`notify`クレートには含まれておらず、`notify-debouncer-mini`や`notify-debouncer-full`という別クレートとして提供されています。chokidarの`awaitWriteFinish`のように標準機能として組み込むのではなく、必要な場合だけ別途取り込む、という設計方針を取っています。

## Ruby

Rubyでは、[guard/listen](https://github.com/guard/listen)が広く使われています。

APIは`Listen.to(*dirs) { |modified, added, removed| ... }`という形で、採用しているアダプタに関わらず、常に変更・追加・削除された絶対パスの配列3つをコールバックに渡すようになっています。監視対象のOSに応じたアダプタは自動的に選択され、実際の処理は別のgemに委譲されます。具体的には、Linuxでは`rb-inotify`、macOSでは`rb-fsevent`、*BSD系では`rb-kqueue`、Windowsでは`wdm`が使われ、いずれの環境にも当てはまらない場合はポーリングにフォールバックします。

`:latency`（変更の確認頻度）と`:wait_for_delay`（変更検知後にコールバックを呼ぶまでの遅延時間）を独立して調整出来る他、`:ignore`/`:only`で監視対象パスを正規表現によりフィルタする事も出来ます。
制限事項として、forkしたプロセス間ではリスナーが引き継がれない為プロセス毎に`Listen.to`を呼び出す必要がある事や、NFSやSamba、VMの共有フォルダのようにネイティブアダプタが機能しない環境ではポーリングが必須になる事、監視対象が多いプロジェクトではLinux環境のinotify watch数上限（`max_user_watches`、カーネルのデフォルト値は環境によって異なる）に達してしまう事などが挙げられます。

## Python

[gorakhargosh/watchdog](https://github.com/gorakhargosh/watchdog)

## プログラミング言語に依存しない実装

### libuv

libuvは、Node.jsのために開発されたクロスプラットフォームの非同期I/Oライブラリです。ファイルシステムイベント監視用の機能も提供されており、Node.jsのAPIではこちらの機能が使われています。
当然クラスをプラットフォーム対応が行われており、下記APIが使用されるようになっています。

* Linux：inotify
* BSD系：kqueue
* macOS： FSEvents、kqueue(使える場合はFSEventsを優先)
* Windows：ReadDirectoryChangesW

APIは、監視ハンドルを初期化する`uv_fs_event_init()`、監視を開始する`uv_fs_event_start(handle, cb, path, flags)`、停止する`uv_fs_event_stop()`という構成になっています。再帰監視を有効にする`UV_FS_EVENT_RECURSIVE`フラグは、ネイティブに再帰監視をサポートしているmacOSとWindowsでのみ実装されており、Linuxのinotify自体には再帰監視の仕組みが無く、libuv側でもユーザー空間での代替実装は行っていない為、Linux上では利用出来ません。

Python(uvloop)、Rust(livub-rs)などのライブラリがあり、Node.js以外でも使用されています。

### watchman

Watchman[^watchman]は、Facebookによって開発されたファイル監視ツールです。大規模なコードベースでの効率的なファイル変更検出を目的として設計されており、特にビルドシステムや開発ツールのバックエンドとして使用されています。
[^watchman]: https://github.com/facebook/watchman

#### 特徴

Watchmanは以下のような特徴を持っています：

* 高性能な監視：大規模なディレクトリツリーに対して効率的にファイル変更を監視
* クロスプラットフォーム対応：Linux、macOS、Windowsで動作
* ユニークなクエリ機能：ファイル変更に対する柔軟なフィルタリングと条件指定が可能
* トリガー機能：特定のファイル変更パターンに対して自動的にコマンドを実行
* JSON形式のレスポンス：プログラムからの連携が容易

#### 使用しているAPI

WatchmanはプラットフォームごとにOSの最適なファイル監視APIを使用します：

| OS | 使用しているAPI |
| --- | --- |
| Linux | inotify |
| macOS | FSEvents |
| Windows | ReadDirectoryChangesW |

#### クエリシステム

Watchmanの大きな特徴の一つが、強力なクエリシステムです。単純なファイル変更通知だけでなく、以下のような条件でファイルをフィルタリングできます。

* ファイル名のパターンマッチング（glob、正規表現）
* ファイルサイズ、変更時刻による条件指定
* ファイルタイプ（ファイル、ディレクトリ、シンボリックリンク）による絞り込み
* 変更されたファイルのメタデータ（権限、所有者など）取得

Watchmanは、ライブラリとしてアプリケーションに組み込む形ではなく、独立したデーモンプロセスとして動作します。複数のクライアントはソケット経由で同じwatchmanプロセスに接続し、監視状態を共有します。例えば`watchman watch <dir>`でディレクトリの監視を開始し、`watchman -- trigger <dir> <name> <glob> -- <cmd>`のようにファイル変更のパターンに対してコマンドを自動実行するトリガーを登録出来ます。これにより、多数の開発ツールが同時にファイル監視を行う環境でも、それぞれが個別にOSのAPIを呼び出す事なく、システムリソースを効率的に利用出来ます。

#### Cookie機構

Watchmanが正確性を担保する為に持っている仕組みが、Cookieと呼ばれる機構です。クエリを実行する際、監視ルート内に一意な名前の一時ファイルを作成し、その作成イベントが通知ストリームに戻ってくるまでクエリの応答を待ちます。これにより、「このCookieファイルの作成が見えた時点で、それより前に発生した変更は全て観測済みである」という事を保証出来ます。この仕組みは、大規模な並列テストの実行中にWatchmanの処理に遅延が生じ、まだ反映されていない古い状態を返してしまう、という実運用上の問題への対処として導入されたそうです。

ただし、この保証も万能ではありません。前章で触れた通り、FSEventsには変更の配信完了を保証する仕組みが無く、`git checkout`のような負荷の高い操作の際には、Cookie機構と組み合わせても、それ以前の変更に対応するイベントが遅れて届いてしまう事があるようです。
