# Appendix

## stdsc : 標準サーバ・クライアントライブラリ

本ライブラリを実装するにあたり,サーバやクライアントの基本的なネットワーク実装は外部ライブラリである`stdsc`を用いて実現するものとする.

stdsc (STanDard Server Client library): https://github.com/yamanalab/stdsc

このライブラリは,サーバの状態遷移を実装するためのフレームワーク,特定のリクエストに対するコールバックを実装するためのフレームワーク,またクライアントのデータ送受信機能などを提供する. 詳細は上記URLのREADMEおよび同梱のexamplesを参照のこと.


## LUTファイルフォーマット

LUTファイルの入力数1の場合と2の場合とでそれぞれ用意する.
以下にフォーマットを示す.

```eval_rst
.. image:: images/fhetbl_design-LUT_fileformat.png
   :align: center
   :scale: 70%
```

```eval_rst
1行目は項目数を記述し, 2行目以降は入力値 :math:`x0` , :math:`x1` と出力値 :math:`f(x0,x1)` のペアを各行に記述する (入力数が1の場合は, :math:`x0` および :math:`f(x0)` とする).
```