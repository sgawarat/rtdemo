# メモ

## 不具合と解決策

### OpenGL

#### バッファ

##### `glBindBufferRange`でバインドしたUBOがシェーダから見えない。

- UBOの`glBindBufferRange`の`offset`は、`GL_*_BUFFER_OFFSET_ALIGNMENT`の値の倍数でなければならない。― [OpenGL Wiki](https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object#Limitations)
- 構造体のサイズを大きめの2の倍数(256や512)になるよう調整してみる。
