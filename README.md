VeriCross: A Rapid Cross-Verification Platform for Soft Processors using Verilator
==================================================================================

For English description, please refer to <a href="README.en.md">README.en.md</a>.

概要
----

このリポジトリには，以下の論文で提案された，Verilator を用いたソフトプロセッサの
相互検証環境のソースコード一式を収録しています．

（論文は現在投稿中: 採録・書誌情報確定後に更新されます）

ベンチマークプログラム（MiBench）は RV32I 向けにコンパイルされる必要があるため，
RISC-V クロスコンパイラが必要です．詳しくは，`benchmark/README.md` を参照してください．

SystemVerilog（あるいは Verilog）で記述されたプロセッサの C++ コードへの変換のため，
Verilator が必要です．Ubuntu 24.04 から apt でインストールできるバージョン（5.020）
での動作を確認しています．

----------------------------------------------------------------------------------

リポジトリの構成
----------------

リポジトリのフォルダ構成は以下のとおりです．

- `benchmark`: MiBench の修正版
  - `binary`: クロスコンパイル後のバイナリ等一式がここに保存されます
  - `input`: 入力のデータファイル（ファイルサイズが大きいため，GitHub の管理外）
  - `source`: MiBench のソースコード一式
- `core_patch`: kronos および RVCoreP に対する，相互検証実現のためのパッチ
- `core_patched`: パッチを適用したファイルがここに保存されます
- `core_verilated`: Verilator の出力したコードがここに保存されます
- `core_wrapper`: kronos および RVCoreP を C++ から扱うためのラッパ回路およびクラス
- `eval`: 論文の評価用のスクリプト一式
- `kronos`: kronos の Verilator 5.x 対応版（サブモジュールで管理）
- `rvcorep`: RVCoreP の Verilator 5.x 対応版（サブモジュールで管理）
- `sim`: VeriCross の C++ ソースコード一式

----------------------------------------------------------------------------------

使い方
------

### 前提

RISC-V クロスコンパイラは `riscv32-unknown-elf-gcc` で利用できる準備が整っている
ものとします．コマンド名が異なる場合，`benchmark/common.mk` を修正してください．

Verilator の共通インクルードファイルは `/usr/share/verilator/include` にあるもの
とします．場所が異なる場合，`Makefile` の `VERILATOR_INCLUDE` を修正してください．

### ベンチマークの入力

ベンチマークの入力は，ファイルサイズが大きいため，GitHub の管理外としています．
初回のみ，`make -C benchmark input` でダウンロード・展開してください．

### システムのビルド

ベンチマークのコンパイル，kronos および RVCoreP の C++ コードへの変換とビルド，
これらを静的リンクした VeriCross のビルドは，すべて単に `make` するだけで
行えるようにしています．

### コマンドライン引数

VeriCross のバイナリは，必須のコマンドライン引数として，以下の3つを必要とします．
- バイナリと関連ファイルの存在するフォルダ名
- バイナリのファイル名（拡張子を除く）
- スタックの初期状態を記録したファイルの名前（末尾の `_sp.txt` を除く）

これらに加えて，以下のオプションの引数を受け付けます．
- `-p`: プロセッサコアのみを使用する（`-c` オプションも指定すること）
- `-ccore`, `--core core`: プロセッサコアモデルを指定する
- `--list-core`: 利用可能なプロセッサコアモデルを表示して終了する
- `-l`: 命令の実行トレースを標準出力に書き出す
- `--logfile file`: 命令の実行トレースをファイルに書き出す
- `-icount`, `--instruction count`: 実行する最大の命令数を指定する
- `-mN`, `--mismatch N`: 不一致を検出した場合，直近 N 命令のログを出力する

コマンドライン引数の例は，`eval` フォルダのシェルスクリプトを参照してください．


----------------------------------------------------------------------------------

著作権
------

本リポジトリの
- `sim` フォルダ中の C++ ソースコード，
- `core_wrapper` フォルダ中の C++, SystemVerilog ソースコード，
- `core_patch` フォルダ中の SystemVerilog パッチ

は，[藤枝 直輝](https://aitech.ac.jp/~dslab/nf/) により開発され，愛知工業大学
ディジタルシステム研究室が著作権を保有します．
これらのソースコードの一部には，松川 達哉，本橋 一馬，杉山 皓星，岡部 匠悟 による
貢献を含みます．ライセンスは New BSD です．

（`benchmark` フォルダにある）MiBench の各プログラムには，元のライセンスがそのまま
適用されます．
詳細は LICENSE.txt を確認してください．

Copyright (C) 2020-2026 Digital Systems Laboratory. All rights reserved.
