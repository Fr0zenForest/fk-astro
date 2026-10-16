# fk-astro

PixInsight 里的 CoreML 分块推理模块(`.mlpackage` / `.mlmodel` 明文直读),
内置 **FKv2 标定归一化**,可高精度复现 BlurXTerminator ML4 / ML5 的推理结果
(ML5 全图中位像素与官方逐位相同,见下文精度表)。

两用:

1. **通用骨架** — 在 PixInsight 里跑你自己训练的去噪/锐化类 CoreML 模型;
2. **BXT 复现验证** — 加载 BXT ML5 模型 + FKv2 归一化,与官方
   BlurXTerminator 2.6.9 输出做逐像素对比。

## 目录结构

```
fk-astro/
├── Makefile              构建脚本(双架构通用二进制)
├── install.sh            签名 + 拷贝 + 启动 PixInsight 一条龙
├── src/
│   ├── CoreMLEngine.h    CoreML 引擎接口(纯 C++)
│   ├── CoreMLEngine.mm   引擎实现:compileModelAtURL / MLModel / MLMultiArray
│   ├── FKAstroProcess.h  进程与参数元类定义
│   ├── FKAstroProcess.cpp  分块推理管线(PI 精确网格/FKv1/FKv2 归一化/羽化)
│   ├── FKCurves.h        FKv2 标定曲线库(5 组 × 498 节点,BXT ML5 运行时采集)
│   ├── FKAstroInterface.h   自定义界面定义(五区分组)
│   ├── FKAstroInterface.cpp 界面实现(Options 区含归一化下拉)
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
自用走 Local Signing Identity(绑定你的 PixInsight 许可证)。

一键流程(先在 PixInsight `Edit > Local Signing Identity...` 创建过本地签名身份):

```bash
XSSK=~/workplace/fk-astro/dev.xssk ./install.sh
# 提示输入 .xssk 密码;或 XSSK_PASSWORD=... ./install.sh
```

脚本会:签名 → `sudo cp` 到 `/Applications/PixInsight/bin/` → 以 `-m` 参数
启动 PixInsight 加载模块。**PixInsight 不能在运行中**(先正常退出再跑脚本),
首次加载后正常退出一次,让模块安装状态落盘。

手动等价流程:

```bash
/Applications/PixInsight/PixInsight.app/Contents/MacOS/PixInsight \
    --sign-module-file=$HOME/workplace/fk-astro/dist/FKAstro-pxm.dylib \
    --xssk-file=<你的密钥文件.xssk> --xssk-password="<密码>"
sudo cp dist/FKAstro-pxm.dylib dist/FKAstro-pxm.xsgn /Applications/PixInsight/bin/
```

发布给其他人用则需申请 CPD(Certified PixInsight Developer),
详见本机文档 `/Applications/PixInsight/doc/docs/CodeSigning/`。

## 使用

进程名 **FKAstroInference**(分类 Preprocessing / FKAstro),自定义界面分五区:
Model / Stellar Adjustments / Nonstellar Adjustments / Options /
Additional Options。

| 参数 | 默认 | 说明 |
|---|---|---|
| modelPath | — | 模型文件路径(.mlpackage 或 .mlmodel) |
| sharpenStars | 0.5 | 恒星区锐化强度 [0, 0.7](参数槽 0) |
| adjustStarHalos | 0.0 | 恒星光晕调整 [-0.5, 0.5](参数槽 1) |
| psfDiameter / autoPSF | 0.0 / true | 目标 PSF 直径 [0, 8] px;自动时模型自估(参数槽 2/3) |
| sharpenNonstellar | 0.5 | 非恒星区锐化强度 [0, 1](参数槽 4) |
| correctOnly | true | 中性校正模式:槽 0–4 清零(自动 PSF 仍生效) |
| normScheme | None | 归一化方案,见下 |
| reflectPad | true | 分块窗口越界时镜像填充(否则复制边缘像素) |
| tileOverlap | 0.2 | 相邻块重叠比例 [0, 0.5](消除块间接缝) |
| device | Auto | Auto / GPU / CPU |
| verbose | true | 过程控制台输出进度 |

### 归一化(Options 区)

- **None** — 像素直通模型,参数向量只含 5 个用户槽位。
  BXT **ML4(5 参数模型)用这个**:官方 ML4 的归一化接近恒等,
  直通即可到全图 mean 差 ~8e-5 的精度(见下表)。
- **FKv1** — 6 参数规范公式:每图测一次背景统计 `bg = P35 分位数`,
  第 6 入参 `slot5 = -log10(bg)` 自动计算(非用户参数),
  逐块做 `log → MTF(m=0.100) → affine(a=1.4685, b=-0.7095)` 正向归一化、
  推理后逆变换贴回,工作点 `wp = 0.505 × bg`。要求模型声明
  `[1, 6]` parameters 输入(5 参数模型选 FKv1 会直接报错)。
  暗区可用,亮区相对误差可达 30–130%,已被 FKv2 取代,仅留作对照。
- **FKv2**(推荐,ML5 用) — 标定曲线归一化。同 FKv1 一样按图测 `bg`
  并计算 `slot5`,但正/逆变换换成从 **BlurXTerminator ML5 运行时真实模型
  输入**采集的单调曲线(`src/FKCurves.h`):
  - 曲线按 `log(bg)` 最近邻从 5 组标定中选择
    (bg = 0.002 / **0.0102 深空典型** / 0.1025 / 0.19 / 0.35,
    每组 498 节点,log-x 均匀重采样加密尾部);
  - 插值为 **PCHIP(Fritsch-Carlson 单调三次)**,与 scipy
    `PchipInterpolator` 逐位一致;正逆两套节点斜率独立构建,
    逆变换在端点饱和钳位(模型输出超出标定域时取端点,不外推);
  - 归一化只作用于窗内有效(非零填充)区域。
  同样要求 `[1, 6]` parameters 输入,即只适用于 ML5。

### 分块网格

复刻 BXT 精确 tile 布局(逆向 0xf089c + 实测验证):
输出块固定 512²,`n = ceil(D / (512·(1−overlap)))`,`sx = D/n`,
内部块位置 `pos(k) = round(sx·k − (512−sx)/2)`(k = 1..n−2),
首块 0、末块 D−512。重叠区按羽化权重线性融合。
实测 M31(6248×4176, overlap 0.2)的列序列
0, 330, 720, 1111, …, 5406, 5736 与官方完全吻合。

## BXT 复现配方与精度

配方(以 ML5 为例):

| 设置 | 值 |
|---|---|
| modelPath | 解密后的明文 `BlurXTerminator.5.mlpackage`(见 `~/py-astro`) |
| normScheme | **FKv2** |
| sharpenStars / adjustHalos / psf / autoPSF / sharpenNonstellar | 与官方 BXT 界面同值 |
| correctOnly | 按需(false = 锐化,true = 仅校正) |
| tileOverlap | 0.2(与官方默认一致) |

M31 单帧(6248×4176, 16bit, ML5, sharpen 0.56/0.37, halo 0.10, autoPSF)
对官方 BlurXTerminator 2.6.9 的全图像素差([0,1] 域):

| 配置 | P50 | P99 | P99.9 | 亮区(a>0.03)相对 | d>0.3 占比 |
|---|---|---|---|---|---|
| **ML5 + FKv2** | **0**(逐位相同) | 2.9e-4 | 5.7e-3 | 3.4% | 0.0000% |
| ML5 + FKv1(对照) | 0 | — | — | 37.5–129% | — |
| ML4 + None | 1.5e-5 | 7.0e-4 | — | 6.1% | — |

ML4 结论:官方 ML4 归一化近恒等(5 参数模型,无背景槽),
None 直通即达全图 mean 差 7.8e-5;若要 ML4 也到逐位级,
需另外采集 ML4 标定曲线(当前曲线库只覆盖 ML5)。

## 模型要求(当前骨架的限制)

- 输入输出都是 **固定形状 3-D float32 多数组**(CHW 或 HWC 自动识别)。
- **输出形状 == 输入形状**(去噪/锐化类模型)。超分类模型需要自行扩展
  `StoreTile` 的映射。
- 通道数:模型 C == 图像通道数,或 C == 1(单通道模型逐通道独立处理)。
- 图像小于模型输入时自动零填充单块处理。
- 像素约定:PixInsight 归一化浮点 [0,1];整型图像在喂给模型前按
  最大采样值归一到 [0,1],模型输出写回时再按
  `v·HighestSampleValue + 0.5` 量化(float 图直接写回 float)。

## 开发要点(PixInsight 模块开发的已验证实践)

- 模块命名 `<Name>-pxm.dylib`,install_name 为
  `@executable_path/FKAstro-pxm.dylib`
- 链接:`-lPCL-pxi -llz4-pxi -lzstd-pxi -lzlib-pxi -lRFC6234-pxi -llcms-pxi`
  + `-framework AppKit -framework Foundation -framework CoreML`
- 分块:输出块 512²,网格公式见上节;窗口原点钳位到 `[0, W−TW]`,
  输出只写每块中心区域,重叠区羽化融合
- 归一化曲线插值(PCHIP)在 double 域做,喂模型前转 float32;
  逆变换后写回整型图像时必须过 `HighestSampleValue` 量化,
  不能把 [0,1] 值直接标量转换成整型(会全黑)
