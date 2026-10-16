"""Compile and run the production ML5 math and extracted image reader offline.

Requires a C++17 compiler (MSVC Build Tools on Windows, clang++/g++ elsewhere).
No PCL installation, CoreML, model package, weights or Python packages needed.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build' / 'ml5-tests'


def definition(source, marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth = 0
    for token in re.finditer(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', source[opening:]):
        if token.group() == '{':
            depth += 1
        elif token.group() == '}':
            depth -= 1
            if depth == 0:
                end = opening + token.end()
                return source[start:end] + (';' if source[end:end+1] == ';' else '')
    raise ValueError('Unclosed definition: ' + marker)


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'src/FKAstroProcess.cpp').read_text(encoding='utf-8-sig')
    adapter = r'''
namespace pcl {
template<class P> struct GenericImage {
    using sample = typename P::sample;
    int w,h,n; std::vector<sample> pixels;
    GenericImage(int W,int H,int N):w(W),h(H),n(N),pixels(size_t(W)*H*N) {}
    int Width() const { return w; } int Height() const { return h; }
    int NumberOfChannels() const { return n; }
    const sample* PixelData(int c) const { return pixels.data()+size_t(c)*w*h; }
    sample* PixelData(int c) { return pixels.data()+size_t(c)*w*h; }
};
}
struct TestFloat { using sample=float; };
struct TestDouble { using sample=double; };
struct TestUInt16 { using sample=uint16_t; static uint16_t HighestSampleValue() { return 65535; } };
'''
    markers = ['template <typename S>', 'struct FKNormalizer',
               'template <class P>\nstatic fka::ml5::ArcsinhNormalizer FitML5T',
               'template <class P>\nstatic void WriteML5T',
               'struct FKImageReader\n', 'template <class P>\nstruct FKImageReaderT']
    generated = '#include "FKCurves.h"\n' + adapter + '\n\n'.join(definition(source, m) for m in markers)
    engine = (ROOT / 'src/CoreMLEngine.mm').read_text(encoding='utf-8-sig')
    generated += '\nnamespace fka {\nenum MLMultiArrayDataType { MLMultiArrayDataTypeFloat16, MLMultiArrayDataTypeFloat32, MLMultiArrayDataTypeFloat64 };\n'
    for marker in ['static float HalfToFloat(', 'static uint16_t FloatToHalf(', 'static void WriteArrayValue(', 'static float ReadArrayValue(']:
        generated += definition(engine, marker) + '\n'
    generated += '}\n'
    (BUILD / 'module_reader.inc').write_text(generated, encoding='utf-8')
    test = ROOT / 'tests/ml5_contract_test.cpp'
    executable = BUILD / ('ml5-tests.exe' if os.name == 'nt' else 'ml5-tests')
    if os.name == 'nt':
        vswhere = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        install = subprocess.check_output([str(vswhere), '-latest', '-products', '*', '-requires',
            'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
        vcvars = Path(install) / 'VC/Auxiliary/Build/vcvars64.bat'
        batch = BUILD / 'build.cmd'
        batch.write_text(f'@echo off\ncall "{vcvars}" >nul\nif errorlevel 1 exit /b %errorlevel%\n'
                         f'cl /nologo /std:c++17 /O2 /EHsc /fp:strict /W4 /utf-8 /I"{ROOT / "src"}" /I"{BUILD}" "{test}" /Fe:"{executable}"\n'
                         'exit /b %errorlevel%\n', encoding='utf-8')
        subprocess.run(['cmd.exe', '/d', '/c', str(batch)], cwd=BUILD, check=True)
    else:
        compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('g++')
        if not compiler:
            raise RuntimeError('Install a C++17 compiler or set CXX')
        subprocess.run([compiler, '-std=c++17', '-O2', '-ffp-contract=off', '-Wall', '-Wextra',
                        '-I' + str(ROOT/'src'), '-I' + str(BUILD), str(test), '-o', str(executable)], cwd=BUILD, check=True)
    subprocess.run([str(executable)], cwd=BUILD, check=True)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
