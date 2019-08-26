# rtdemo

[![Build status](https://ci.appveyor.com/api/projects/status/o9v2ysnjq76wr005?svg=true)](https://ci.appveyor.com/project/sgawarat/rtdemo)

## 概要

OpenGLによるレンダリング技法のデモ

## ビルド方法

1. vcpkgを使って依存するライブラリをインストールする
2. DXCをビルドする
   - SPIR-V CodeGenを有効にしてビルドする
3. CMakeを使ってビルドする

## 依存性

### ライブラリ

- [gsl](https://github.com/Microsoft/GSL)
- [spdlog](https://github.com/gabime/spdlog)
- [glfw](https://github.com/glfw/glfw)
- [glew](http://glew.sourceforge.net)
- [imgui](https://github.com/ocornut/imgui)
- [glm](https://github.com/g-truc/glm)
- [assimp](https://github.com/assimp/assimp)

### ツール

- [vcpkg](https://github.com/Microsoft/vcpkg)
- [glslang](https://github.com/KhronosGroup/glslang)
- [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)

### リソース

#### フォント

- [Migu](http://mix-mplus-ipa.osdn.jp/migu/)
