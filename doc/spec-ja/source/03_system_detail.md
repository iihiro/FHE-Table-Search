# システム詳細

[システム概要](02_system_overview)で示した各ライブラリの設計を以下に示す.

## Userライブラリ

Userライブラリは,Decryptor向けのクライアント機能とComputationServer向けのクライアント機能を提供する.

### インターフェース

#### Decryptor向けクライアント

```c++
class DECClient
{
public:
    /**
     * コンストラクタ
     * @param[in] host Decryptorのホスト名
     * @param[in] port Decryptorのポート番号
     */
    DECClient(const char* host, const char* port);
    virtual ~DECClient(void) = default;

    /**
     * 接続
     * @param[in] retry_interval_usec リトライ間隔(usec)
     * @param[in] timeout_sec タイムアウト時間(sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * 切断
     */
    void disconnect();

    /**
     * 新規鍵ペア生成
     * @param[out] pubkey Public key
     * @param[out] pseckey Secret key
     * @return keyID
     */
    int32_t new_keys(fts_share::PubKey& pubkey, fts_share::SecKey& seckey);

    /**
     * 鍵ペア削除
     * @param[in] key_id keyID
     * @return 処理に成功したか否か
     */
    bool delete_keys(const int32_t key_id) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

コンストラクタで受け取った接続先に対して,`connect()`で接続し,`disconnect()`で切断する. `new_keys()`では,Decryptorに対して新規鍵ペアの生成を要求することができ,生成された鍵ペアは引数で受け取ることができる. また,戻り値でその鍵ペアに対応したkeyIDを取得することができる. `delete_keys()`では,keyIDで指定した鍵ペアの削除をDecryptorへ要求することができる.

#### ComputationServer向けクライアント

```c++
class CSClient
{
public:
    /**
     * コンストラクタ
     * @param[in] host ComputationServerのホスト名
     * @param[in] port ComputationServerのポート番号
     */
    CSClient(const char* host, const char* port);
    virtual ~CSClient(void) = default;

    /**
     * 接続
     * @param[in] retry_interval_usec リトライ間隔(usec)
     * @param[in] timeout_sec タイムアウト時間(sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * 切断
     */
    void disconnect();
    
    /**
     * クエリ送信
     * @param[in] key_id keyID
     * @param[in] func_no 関数番号
     * @param[in] enc_input 暗号化された入力値(1つ or 2つ)
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no, const std::vector<fts_share::EncData>& enc_input) const;
    
    /**
     * 結果受信
     * @param[in] query_id queryID
     * @param[out] enc_result 暗号化された結果
     * @return 結果が受信できたか否か
     */
    bool recv_result(const int32_t query_id, fts_share::EncData& enc_result) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

コンストラクタで受け取った接続先に対して,`connect()`で接続し,`disconnect()`で切断する. `send_query()`では,ComputationServerに対してクエリを送信し,そのクエリに対応したqueryIDを引数で返す. この時,ComputationServerは引数で渡されたkeyID,関数番号および暗号化された入力値に従って計算を非同期で開始する. `recv_result()`では,queryIDで指定したクエリの計算結果があればそれを引数で返す. 結果がない(まだ計算中の)場合は, 戻り値としてfalseを返す.

### シーケンス

前述の「インターフェース/Decryptor向けクライアント」および「インターフェース/ComputationServer向けクライアント」のインターフェースを用いて,[システム概要/User](02_system_overview)の処理フローに従って処理を行う.


## Decryptorライブラリ

Decryptorライブラリは,Decryptorパーティとしてのサーバ機能を提供する.

### インターフェース

```c++
class DecServer
{
public:
    /**
     * コンストラクタ
     * @param[in] port ポート番号
     * @param[in] callback コールバック関数定義
     * @param[in] state 状態遷移定義
     */
    DecServer(const char* port,
              stdsc::CallbackFunctionContainer& callback,
              stdsc::StateContext& state);
    ~DecServer(void) = default;

    /**
     * サーバ始動
     */
    void start(void);
    /**
     * サーバ停止指示
     */
    void stop(void);
    /**
     * サーバ停止待ち
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

Decryptorの実装であるDecServerクラスは,インターフェースとしてサーバの始動/停止のメソッドのみ持つ. リクエスト受信時の処理や状態遷移はstdscの`stdsc::CallbackFunctionContainer`および`stdsc::StateContext`を用いて実装する. stdscについては[Appendix/stdsc:標準サーバ・クライアントライブラリ](04_appendix)を参照のこと.

### 状態遷移

```eval_rst
.. image:: images/fhetbl_design-state-dec.png
   :align: center
   :scale: 70%
```

Decryptorは1つの状態しか持たない.

### シーケンス

#### 新規鍵生成リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-1.png
   :align: center
   :scale: 70%
```

DecServerは新規鍵生成リクエストを受信( **(1)** )すると,識別子となるkeyIDを生成した上で( **(2)** ),新規の鍵ペアを生成する( **(3)** ).
生成した鍵ペアはkeyIDと対応付けてkey tableへ保存した上で,それをレスポンスとして返す( **(4)** ).

#### 公開鍵リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-2.png
   :align: center
   :scale: 70%
```

公開鍵リクエストを受信( **(1)** )すると,key tableからkeyIDに対応した公開鍵を取得し( **(2)** ),レスポンスとして返す( **(3)** ).

#### 計算リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-3.png
   :align: center
   :scale: 70%
```

計算リクエストとして暗号化された中間結果を受信( **(1)** )すると,それを復号化し( **(2)** ),PIRクエリを生成する( **(3)** ).
生成したPIRクエリを暗号化して,それをレスポンスとして返す( **(4)(5)** ).

#### 鍵破棄リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-4.png
   :align: center
   :scale: 70%
```

DecServerは鍵破棄リクエストを受信( **(1)** )すると,keyIDに対応した鍵ペアをkey tableから削除( **(2)** )した上で,処理に成功したか否かをレスポンスとして返す( **(3)** ).


## Computation Serverライブラリ

ComputationServerライブラリは,ComputationServerパーティとしてのサーバ機能を提供する.

### インターフェース

```c++
class CSServer
{
public:
    /**
     * コンストラクタ
     * @param[in] port ポート番号
     * @param[in] LUT_dirpath LUTファイルのディレクトリパス
     * @param[in] callback コールバック関数定義
     * @param[in] state 状態遷移定義
     * @param[in] max_concurrent_queries 最大同時クエリー数
     * @param[in] max_results 最大結果保持数
     * @param[in] result_lifetime_sec 結果を保持する時間(秒)
     */
    CSServer(const char* port,
             const std::string& LUT_dirpath,
             stdsc::CallbackFunctionContainer& callback,
             stdsc::StateContext& state,
             const uint32_t max_concurrent_queries = DEFAULT_MAX_CONCURRENT_QUERIES,
             const uint32_t max_results = DEFAULT_MAX_RESULTS,
             const uint32_t result_lifetime_sec = DEFAULT_MAX_RESULT_LIFETIME_SEC);
    ~CSServer(void) = default;

    /**
     * サーバ始動
     */
    void start(void);
    /**
     * サーバ停止指示
     */
    void stop(void);
    /**
     * サーバ停止待ち
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

ComputationServerの実装であるCSServerクラスは,インターフェースとしてサーバの始動/停止のメソッドのみ持つ. リクエスト受信時の処理や状態遷移はstdscの`stdsc::CallbackFunctionContainer`および`stdsc::StateContext`を用いて実装する. stdscについては[Appendix/stdsc:標準サーバ・クライアントライブラリ](04_appendix)を参照のこと.
また,CSServerクラスのコンストラクタでは以下の処理を行う.
* 引数`LUT_dirpath`のディレクトリから,`*.txt`のファイルをLUTファイルとして全て読み込む(LUTファイルの詳細は[Appendix/LUTファイルフォーマット](04_appendix)を参照のこと).
* 引数`max_concurrent_queries`を最大同時クエリ数として,後述の「シーケンス/クエリ受信時」の処理において同時に受け付けられる最大クエリ数を制限する
* 引数`max_results`を最大結果保持数として,後述の「シーケンス/クエリ受信時」の処理において保持する最大結果数を制限する
* 引数`result_lifetime_sec`を結果の寿命(結果が保持される時間)として,後述の「シーケンス/クエリ受信時」の処理において時間超過した結果は削除される

### 状態遷移

```eval_rst
.. image:: images/fhetbl_design-state-cs.png
   :align: center
   :scale: 70%
```

ComputationServerは1つの状態しか持たない.

### シーケンス

#### クエリ受信時

```eval_rst
.. image:: images/fhetbl_design-seq-cs-01-1.png
   :align: center
   :scale: 70%
```

CSServerはクエリを受信( **(1)** )すると,まず入力値数と関数番号の整合性が取れているかを検証する( **(2)** ). 検証の結果,不正と判断された場合は,その旨をレスポンスとして返し,以降の処理は行わない. 次に計算結果を保持するResultQueueに対して,保持された結果の数が上限値(`max_results`)に達しているかを確認する. 上限値に達している場合は,寿命(`result_lifetime_sec`)を超えた結果をResultQueueから削除する( **(3)** ). その上で,クエリを保持するQeuryQueueの登録数とReusultQueueの登録数の上限値超過のチェックを行い,超過している場合は,その旨をレスポンスとして返し,以降の処理は行わない. 超過していない場合は,受信したクエリの識別子となるqueryIDを生成( **(4)** )した上で,QueryQueueへそれをプッシュし( **(5)** ),queryIDをレスポンスとして返す( **(6)** ).

QeuryQueueとResultQueueはアトミックキューとして,他スレッドから排他的にアクセスできる作りとする. QueryQueueにプッシュされたクエリはCSThreadがポップして処理する. CSThreadは,CSServerとは別スレッドで動作するComputationServerのメイン計算処理を行うスレッドである. CSThreadは計算結果をResultQueueへプッシュする.

```eval_rst
.. image:: images/fhetbl_design-seq-cs-02.png
   :align: center
   :scale: 70%
```

CSThreadは,CSServerとは異なるスレッドで動作し,QueryQueueにクエリがプッシュされるのを非同期に監視する. QueryQueueにクエリがプッシュされると,それをポップし( **(1)** ),計算処理を開始する.

CSThreadは,CSClient(DecServerとのやりとりを仲介するクラス)を通じてkeyIDに対応するPublicKeyを取得する( **(2)** ). 次にクエリで受け取った暗号化された入力値を元に,LUTから中間結果を探索し( **(3)** ), CSClientを通じてPIRクエリを取得する( **(4)** ). PIRクエリからクエリを再構築し, LUTから出力値を取得する( **(5)** ). 取得した出力値は,ResultQueueへプッシュする( **(7)** ).

#### 結果リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-cs-01-2.png
   :align: center
   :scale: 70%
```

CSServerは結果リクエストを受信( **(1)** )すると,ResultQueueからqueryIDに対応した結果(CSThreadで求められる出力値)をポップし( **(2)** ),レスポンスとして返す( **(3)** ). queryIDに対応した結果が存在しない場合は,その旨をレスポンスとして返す.

