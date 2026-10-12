# fk-astro

在 PixInsight 里运行你自己训练的 CoreML 模型(`.mlpackage` / `.mlmodel`)的
最小可用模块骨架。分块推理 + 重叠边距,加载你自己训练的 CoreML 模型
(明文 .mlpackage/.mlmodel 直读,无需任何中间层)。

## 目录结构

```
fk-astro/
├── Makefile              构建脚本(双架构通用二进制)
├── src/
│   ├── CoreMLEngine.h    CoreML 引擎接口(纯 C++)
│   ├── CoreMLEngine.mm   引擎实现:compileModelAtURL / MLModel / MLMultiArray
│   ├── FKAstroProcess.h  进程与参数元类定义
│   ├── FKAstroProcess.cpp  分块推理管线(FillTile/Predict/StoreTile)
│   └── FKAstroModule.cpp 模块入口(MetaModule + InstallPixInsightModule)
├── build/                编译中间产物(按架构分子目录)
└── dist/FKAstro-pxm.dylib  ← 最终模块
```

## 构建

前置条件:
- `/Applications/PixInsight` 完整安装(自带 PCL 2.11 头文件和源码)
- Xcode(MacOSX SDK)

```bash
cd ~/workplace/fk-astro
make -j8            # 编译第三方库 + PCL + 模块(首次约 10–20 分钟)
```

产出 `dist/FKAstro-pxm.dylib`(arm64 + x86_64 通用二进制,ad-hoc 签名)。

增量重建只编模块源码时:`make -j8` 会自动跳过未变化的部分。

## 安装到 PixInsight

PixInsight **强制要求模块签名**(`.xsgn`,Ed25519),未签名模块无法安装。
自用走 Local Signing Identity(绑定你的 PixInsight 许可证):

1. 打开 PixInsight,菜单 `Edit > Local Signing Identity...`,
   按提示创建本地签名身份(会生成密钥文件,记住密码)。
2. 用命令行给模块签名:

   ```bash
   /Applications/PixInsight/PixInsight.app/Contents/MacOS/PixInsight \
       --sign-module-file=$HOME/workplace/fk-astro/dist/FKAstro-pxm.dylib \
       --xssk-file=<你的密钥文件.xssk> \
       --xssk-password="<密码>"
   ```

   生成 `dist/FKAstro-pxm.xsgn`。
3. 复制到模块目录并重启 PixInsight:

   ```bash
   sudo cp dist/FKAstro-pxm.dylib dist/FKAstro-pxm.xsgn /Applications/PixInsight/bin/
   ```

   (也可用 PixInsight 的模块安装界面安装。)

发布给其他人用则需申请 CPD(Certified PixInsight Developer),
详见本机文档 `/Applications/PixInsight/doc/docs/CodeSigning/`。

## 使用

进程名 **FKAstroInference**(分类 Preprocessing / fk-astro)。
参数(默认界面自动生成):

| 参数 | 默认 | 说明 |
|---|---|---|
| modelPath | — | 模型文件路径(.mlpackage 或 .mlmodel) |
| inputScale / inputOffset | 1.0 / 0.0 | 输入变换 `t = v·scale + offset` |
| outputScale / outputOffset | 1.0 / 0.0 | 输出变换 `v' = t'·scale + offset`(结果截断到 [0,1]) |
| tileOverlap | 0.2 | 相邻块重叠像素(消除块间接缝;模型输入尺寸固定,块大小由模型决定) |
| device | Auto | Auto(ANE→GPU→CPU)/ GPU / CPU |
| verbose | true | 过程控制台输出进度 |

## 模型要求(当前骨架的限制)

- 输入输出都是 **固定形状 3-D float32 多数组**(CHW 或 HWC 自动识别)。
- **输出形状 == 输入形状**(去噪/锐化类模型)。超分类模型需要自行扩展
  `StoreTile` 的映射。
- 通道数:模型 C == 图像通道数,或 C == 1(单通道模型逐通道独立处理)。
- 图像小于模型输入时自动零填充单块处理。
- 像素约定:PixInsight 归一化浮点 [0,1] 直接喂给模型并直接写回
  (训练时请用相同的 [0,1] 归一化;其他值域需自行在源码中加变换)。

## 开发要点(PixInsight 模块开发的已验证实践)

- 模块命名 `<Name>-pxm.dylib`,install_name 为
  `@executable_path/FKAstro-pxm.dylib`
- 链接:`-lPCL-pxi -llz4-pxi -lzstd-pxi -lzlib-pxi -lRFC6234-pxi -llcms-pxi`
  + `-framework AppKit -framework Foundation -framework CoreML`
- 分块:步长 = 块尺寸 − 2×边距,窗口原点钳位到 `[0, W−TW]`,
  输出只写每块中心区域,重叠区由后续块覆盖 → 无缝
