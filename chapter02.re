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

さて、そんな@<code>{fs.watch}ですが、OS毎のAPIの差異についてはどのように対応しているのでしょうか? @<code>{fs.watch}はAPIの差異の吸収は行わず、OSが提供しているAPIをそのまま使用する、という方針を取っているようです。@<code>{fs.watch}のドキュメントの注意事項にも、"The fs.watch API is not 100% consistent across platforms, and is unavailable in some situations."との記載があります。

具体的な例を見ていきましょう。@<code>{fs.watch}

//table[API][使用している機能]{
OS	使用しているAPI
-------------------------------------------------------------
Linux	inotify
macOS	kqueue、FSEvents
Windows	ReadDirectoryChangesW
//}

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
