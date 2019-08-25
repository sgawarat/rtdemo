# メモ

## Tips

### HLSL

- ピクセルシェーダの入力としての`SV_Position`はスクリーン上のピクセル座標を返す

## 不具合と解決策

### 全般

#### 表示がおかしい

- 行列が転置状態になっているかも
  - DXCの`-Zpr`オプションを使ってみる
    - オプションなしだと、GLSLでは`layout(row_major)`が付与される

### コンピュートシェーダ

#### テクスチャをサンプリングできない

- `Sample`ではなく`SampleLevel`を使う
  - コンピュートシェーダはLODをサポートしていないらしい
  - 参考：https://devtalk.nvidia.com/default/topic/492974/texture-sampler-in-compute-shader

### OpenGL

#### 描画

##### 深度テストと深度書き込みを有効にすると、オブジェクトが描画されない。

- 毎フレームで深度バッファをクリアしてみる。
    - 深度バッファがクリアされないと、最も小さい深度値が蓄積してしまい、シーン全体が深度テストに失敗するようになる。
    - 深度バッファを正しくクリアするには`glDepthMask`を`GL_TRUE`にするのを忘れないこと。

#### ディスパッチ

##### コンピュートシェーダが動作しない。

- `glDispatchCompute`に0ではなく、1を渡してみる。
    - いずれかが0だとグループの総数が0になり、結果としてシェーダが動作しない。

#### フレームバッファ

##### glClearでフレームバッファを初期化できない。

- `gl[Color|Depth|Stencil]Mask`を`GL_TRUE`にしてみる。
    - `glClear`も書き込みマスクの影響を受ける。

##### バインドしても書き込み先が変化しない

- `glDrawBuffers`はFBOの一部なので、FBO生成時に設定する。
    - https://stackoverflow.com/questions/30980657/opengl-is-gldrawbuffers-modification-stored-in-a-fbo-no

#### バッファ

##### `glBindBufferRange`でバインドしたバッファがシェーダから見えない。

- 構造体のサイズを大きめの2の倍数(256や512)になるよう調整してみる。
    - `glBindBufferRange`の`offset`は、`GL_*_BUFFER_OFFSET_ALIGNMENT`の値の倍数でなければならない。― [OpenGL Wiki](https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object#Limitations)

##### 頂点バッファやインデックスバッファに対する操作が正常に行われない。

- 頂点バッファやインデックスバッファを操作するときにVAOをバインドしてみる。
    - CoreプロファイルではVAOが必須である。― [Modern OpenGL](http://github.prideout.net/modern-opengl-prezo/)
    - Compatibilityプロファイルでは0番のVAOがデフォルトで用意されるため、VAOを明示しなくても動作する。

#### バインディング

##### 前回起動時のバインディング番号が反映されてしまう。

- バインドするすべてのリソースが、シェーダの要求するサイズを満たしているかを確認してみる。
    - この不具合に遭遇したときは、不具合が出た箇所より前にバインドされていた、シェーダの要求サイズより小さかったUBOのサイズを大きくすると解消された。
    - ドライバのキャッシュ機構が悪さした？

### DXC

#### dxc failed : error code 0x80070459.

- ソースファイルをBOMありUTF8に変換する
  - 2019/02/27現在、BOMなしUTF8だと日本語を含む場合にエラーを吐く

#### インクルードするファイルが見つからない

- `-I`オプションを使う
- Windowsであれば、パスの区切り文字をバックスラッシュで置き換える
  - 2019/02/27現在、Windowsではパスの区切り文字としてスラッシュを受け付けてくれない

### spirv-cross

#### テクスチャにbindingが付与されない

- `--combined-samplers-inherit-bindings`オプションを使う

#### 構造体で構造体の変数を宣言するとコンパイルエラーになる

- 2019/03/08現在、出力されるGLSLは構造体がアルファベット順に定義されるので依存関係が解決できない

#### Push Constantのlocationが指定できない

- 2019/03/17現在、自動割り当てを調整することで実質的に固定できる
  - Intelのドライバでは、コード中に参照される順番によって順々に割り振るようなので、割り振りたい順に値を取り出すと良い

### Renderdoc

#### SSBOの番号の順序が入れ替わっている

- コード中に出現する順番に表示されている
- 実行中も？
  - 実行中は問題ない
