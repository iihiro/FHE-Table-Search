# Appendix

## 標準サーバ・クライアントライブラリ

本ライブラリを実装するにあたり,サーバやクライアントの標準的機能は外部ライブラリであるstdscを用いて実現するものとする.

stdsc (Standard Server Client library): https://github.com/yamanalab/stdsc

このライブラリは,サーバの状態遷移を実装するためのフレームワーク,特定のリクエストに対するコールバックを実装するためのフレームワーク,またクライアントのデータ送受信機能などを提供する. 詳細は上記URLのREADMEおよび同梱のexamplesを参照のこと.
