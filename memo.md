## メモ

* nextjsはwebpackに依存
* webpackは[paulmillr/chokidar](https://github.com/paulmillr/chokidar)を使ってる
* chokidarはmacOS向けには[fsevents/fsevents](https://github.com/fsevents/fsevents)を使ってる。その他OS向けはnodejsのfs.watch使ってる
* fs.watchFileのソース: https://github.com/nodejs/node/blob/74bd866d94aab61feb456bc68229278fbbc4be4b/lib/fs.js#L1565
* [File systemの注意事項](https://nodejs.org/docs/latest/api/fs.html#fs_caveats)
* fsパッケージは [libuv/libuv: Cross\-platform asynchronous I/O](https://github.com/libuv/libuv)を使用している
* libuv: [libuv/filesystem\.rst](https://github.com/libuv/libuv/blob/707dd7f17d9d84d972282666fee47dd3b216349b/docs/src/guide/filesystem.rst#file-change-events)
* [Node\.js の fs\.watch\(\) と fs\.watchFile\(\) の違い \- てっく煮ブログ](http://tech.nitoyon.com/ja/blog/2013/10/02/node-watch-impl/)

* Goだと[fsnotify/fsnotify](https://github.com/fsnotify/fsnotify)
  * [rjeczalik/notify](https://github.com/rjeczalik/notify) もある
* Pythonだと [gorakhargosh/watchdog](https://github.com/gorakhargosh/watchdog)
* Rustだと [notify\-rs/notify](https://github.com/notify-rs/notify)
  * メモ: https://github.com/notify-rs/notify/wiki/Leaks-in-the-abstraction

* facebook: [facebook/watchman](https://github.com/facebook/watchman)
  * [facebook/jest](https://github.com/facebook/jest) はwatchman使ってる
* PHP 公式でinotifyをサポートしている
  * [PHP: Inotify \- Manual](https://www.php.net/inotify)
* Ruby
  * [mat813/rb-kqueue](https://github.com/mat813/rb-kqueue)
* Docker
  * "However, attempts to run Intel-based containers on Apple silicon machines under emulation can crash as qemu sometimes fails to run the container. In addition, filesystem change notification APIs (inotify) do not work under qemu emulation. Even when the containers do run correctly under emulation, they will be slower and use more memory than the native equivalent."
  * [Known issues for Docker Desktop on Mac \| Docker Documentation](https://docs.docker.com/desktop/troubleshoot/known-issues/)

## API

* Linux systems with inotify
  * kernel version 2.6.13,released on August 29, 2005 からある
  * 以前はdnotifyという機能があった
  * [Filesystem notification series by Michael Kerrisk \[LWN\.net\]](https://lwn.net/Articles/605313/)
    * [linux/dnotify\.rst at master · torvalds/linux](https://github.com/torvalds/linux/blob/master/Documentation/filesystems/dnotify.rst)
  * `lsof | grep inotify | wc -l`
* macOS (uses FSEvents on 10.7+, kqueue(2) on earlier versions)
  * "Apple adopted kqueues in Mac OS X 10.3 and extended them in 10.6"
  * https://learning.oreilly.com/library/view/advanced-mac-os/9780321706560/ch16.html
    * https://learning.oreilly.com/library/view/advanced-mac-os/9780321706560/ch16s06.html も参考にする
  * FSEventsはdirectory base
* Windows
  * USNジャーナル
    * NTFS上のファイル/フォルダに対する変更処理を記録
    * バックアップ/アンチウイルス等の処理高速化を目的とした機能
    * update sequence number
  * [ReadDirectoryChangesW function \(winbase\.h\) \- Win32 apps \| Microsoft Docs](https://docs.microsoft.com/ja-jp/windows/win32/api/winbase/nf-winbase-readdirectorychangesw?redirectedfrom=MSDN)
* BSD: kqueue
  * "Kernel Queues"の略
  * FreeBSD 4.1 on July 2000 からある

## 挙動メモ

* listenは非同期APIのみ(変更を同期的に検知するAPIは無い)
* watchfiles(Python)は同期/非同期APIが両方提供されている
* fs.watch(node.js)は非同期APIのみ
* fsnotify(Golang)は非同期APIのみ

## Sample Code

### nodejs

```js
fs.watch(".", { recursive: true }, (event, filename) => {
  console.log(`event: ${event}, filename: ${filename}`);
});


fs.watchFile('memo.md', (curr, prev) => {
  console.log(`the current mtime is: ${curr.mtime}`);
  console.log(`the previous mtime was: ${prev.mtime}`);
});


const chokidar = require('chokidar');
chokidar.watch('.').on('all', (event, path) => {
  console.log(event, path);
});

const chokidar = require('chokidar');
chokidar.watch('memo.md').on('all', (event, path) => {
  console.log(event, path);
});
```
const watcher = fs.watch(".", { recursive: true });

[fs: watch signals for recursive incompatibility · nodejs/node@67e067e](https://github.com/nodejs/node/commit/67e067eb0658281b647ff68a5a9e64ea2cfdb706)

