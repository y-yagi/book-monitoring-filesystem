= 様々なライブラリたち

本章では、ファイル

== Node.js

Node.jsは言語本体でファイル変更監視の為のAPIが提供されています。提供されているのは@<code>{fs.watchFile}と@<code>{fs.watch}という2つのAPIです。

//list[fs.watchFile][fs.watchFile]{
fs.watchFile(filename[, options], listener)
//}

//list[fs.watch][fs.watch]{
fs.watch(filename[, options][, listener])
//}

引数はどちらも同じですね。引数だけ見ると違いがよくわからないのですが、@<code>{fs.watchFile}の方はポーリングによる実装になっており、指定出来るのはファイルだけになっています。対して、@<code>{fs.watch}はOSが提供しているAPIを使用した実装となっており、こちらはファイル・ディレクトリの両方が指定出来るようになっています。

当然OSが提供しているAPIを使用した方が情報が正確に取得出来る為、公式のドキュメントにも、可能な限り、@<code>{fs.watch}の方を使用する事が推奨されています@<fn>{fs.watch.doc}
//footnote[fs.watch.doc][https://nodejs.org/docs/latest/api/fs.html#fs_fs_watchfile_filename_options_listener]

前章で説明した通り、OSが提供しているAPIには、OS毎に差異があります。@<code>{fs.watch}はAPIの差異の吸収は行わず、OSが提供しているAPIをそのまま使用する、という方針を取っているようです@<fn>{fswatch_caution}。使用しているAPIは下記の通りです。
//footnote[fswatch_caution][@<code>{fs.watch}のドキュメントの注意事項にも、"The fs.watch API is not 100% consistent across platforms, and is unavailable in some situations."との記載があります。]

例えば、ファイル名の取得精度です。Linux（inotify）、Windows（ReadDirectoryChangesW）では変更されたファイル名が正確に取得出来ますが、macOS（FSEvents）はディレクトリベースの通知の為、変更されたファイル名が取得出来ない事があります。

なお、以前は、再帰的な監視処理がLinuxでは動作しない、などの問題がありましたが、現在はこの問題は解消されており@<fn>{recursive_linux}、影響が大きい差異については、APIでOSの差異の吸収を行うようになっているようです。
//footnote[recursive_linux][https://github.com/nodejs/node/pull/45098]

//table[API][使用している機能]{
OS	使用しているAPI
-------------------------------------------------------------
Linux	inotify
macOS	kqueue、FSEvents
Windows	ReadDirectoryChangesW
//}

=== chokidar

このようなOS間の差異やネイティブ@<code>{fs.watch}の制限を解決するため、多くのNode.jsプロジェクトでは@<href>{https://github.com/paulmillr/chokidar, chokidar}というライブラリが使用されています。chokidarは、@<code>{fs.watch}の上位互換として設計されており、クロスプラットフォームでの一貫した動作を提供します。

・OS間の差異を吸収し、すべてのプラットフォームで一貫したイベント通知とファイル名取得を提供。

・@<code>{fs.watch}で発生しうる問題（重複イベント、欠落、エラー）を内部で対処し、より安定した監視を実現。

・ネイティブ監視が利用できない環境（一部のネットワークファイルシステム、Docker環境など）では、自動的にポーリングにフォールバック。

などの機能を提供しています。

== Go

== Rust

[notify-rs/notify](https://github.com/notify-rs/notify)

== Ruby

== Python

== PHP

== プログラミング言語に依存しない実装

=== libuv

libuvは、Node.jsのために開発されたクロスプラットフォームの非同期I/Oライブラリです。ファイルシステムイベント監視用の機能も提供されており、Node.jsのAPIではこちらの機能が使われています。
当然クラスをプラットフォーム対応が行われており、下記APIが使用されるようになっています。

・Linux：inofity

・BSD系：kqueue

・macOS： FSEvents、kqueue(使える場合はFSEventsを優先)

・Windows：ReadDirectoryChangesW


Python(uvloop)、Rust(livub-rs)などのライブラリがあり、Node.js以外でも使用されています。


=== watchman

Watchman@<fn>{watchman}は、Facebookによって開発されたファイル監視ツールです。大規模なコードベースでの効率的なファイル変更検出を目的として設計されており、特にビルドシステムや開発ツールのバックエンドとして使用されています。
//footnote[watchman][https://github.com/facebook/watchman]

==== 特徴

Watchmanは以下のような特徴を持っています：

・ 高性能な監視：大規模なディレクトリツリーに対して効率的にファイル変更を監視

・ クロスプラットフォーム対応：Linux、macOS、Windowsで動作

・ ユニークなクエリ機能：ファイル変更に対する柔軟なフィルタリングと条件指定が可能

・ トリガー機能：特定のファイル変更パターンに対して自動的にコマンドを実行

・ JSON形式のレスポンス：プログラムからの連携が容易

==== 使用しているAPI

WatchmanはプラットフォームごとにOSの最適なファイル監視APIを使用します：

//table[watchman_API][Watchmanが使用しているAPI]{
OS	使用しているAPI
-------------------------------------------------------------
Linux	inotify
macOS	FSEvents
Windows	ReadDirectoryChangesW
//}

==== クエリシステム

Watchmanの大きな特徴の一つが、強力なクエリシステムです。単純なファイル変更通知だけでなく、以下のような条件でファイルをフィルタリングできます。

・ファイル名のパターンマッチング（glob、正規表現）

・ファイルサイズ、変更時刻による条件指定

・ファイルタイプ（ファイル、ディレクトリ、シンボリックリンク）による絞り込み

・変更されたファイルのメタデータ（権限、所有者など）取得

Watchmanは独立したデーモンプロセスとして動作し、複数のクライアントが同じwatchmanプロセスを共有することで、システムリソースの効率的な利用を実現しています。これにより、多数の開発ツールが同時にファイル監視を行う環境でも、パフォーマンスの劣化を最小限に抑えることができます。
