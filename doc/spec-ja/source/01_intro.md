# はじめに

## 目的
本書は,準同型表探索(FunctionEvaluationwithFullyHomomorphicEncryptionusingTableLookup)による関数計算ライブラリの基本設計についてまとめたものである.
準同型表探索による関数計算ライブラリを実装する開発者が,ソフトウェアの構成および処理シーケンスを理解できることを目的とする.

## 範囲
準同型表探索による関数計算ライブラリの外部仕様および内部仕様を範囲とする.

## 準同型表探索による関数計算プロトコル
準同型表探索による関数計算プロトコルは,三つのパーティ(User(複数),Decryptor,ComputationServer)から構成される.

```eval_rst
.. image:: images/overview.png
   :align: center
   :scale: 60%
```

```eval_rst
User(s)から送信されるデータをもとに予め決められた計算を行い,その結果を送信元であるUser(s)に返すものである.

Decryptorが暗号鍵を生成し，鍵の内PublicKeyとSecuretKeyをUser(s)に、PublicKeyをComputationServerに配布した後,ComputationServerでは関数入出力のオリジナル表(LUT matries : :math:`f(x0,x1)` を計算するための表)を生成する.(この表は平文状態でComputationServerに保持される).この後,本システムはサービスを開始する.

このサービスは, :math:`f(x0,x1)` の結果を返すものである. User(s)は, :math:`x0` と :math:`x0` のペアを暗号化した上( :math:`Enc(x0), Enc(x1)` )でComputationServerへ送る. ComputationServerでは受け取った :math:`Enc(x0), Enc(x1)` に基づき :math:`f(Enc(x0), Enc(x1))` を計算(表探索により実現)する. 計算過程において,Decryptorとの間でデータのやりとりを行う. 最終的に得られた :math:`f(Enc(x0), Enc(x1))` をComputationServerは,User(s)に返す.

なお,このサービスには入力数が1のものと入力数が2のものがある.

```

## ソフトウェアの入手
準同型表探索による関数計算ライブラリの最新版は以下のGithubリポジトリで管理されている.

https://github.com/yamanalab/FHE-Table-Search

## 用語

```eval_rst
.. csv-table::
   :header-rows: 1

   名称, 説明
```

## 参照
* RuixiaoLI and Yu ISHIMARl and Hayato YAMANA, "Fully Homomorphic Enctyption with Table Loolmp for Privacy-Prese^ing Smart Grid"
* RuixiaoLI and Yu ISHIMARl and Hayato YAMANA, "Privacy Preserving Calculation in Cloud using Fully Homomorphic Encryption with Table Lookup"