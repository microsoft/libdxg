# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project(
  'libdxg',
  'cpp',
  version : '1.0.0'
  )

ndk = get_option('ndk')
message('NDK:' + ndk)

cppargs = ['-std=c++17', '-Wno-unknown-pragmas']
if (ndk != '')
    cppargs += ['--target=x86_64-linux-android']
endif

inc_dirs = [include_directories('include', is_system : true)]

if (ndk != '')
   inc_dirs += ndk + '/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1'
   inc_dirs += ndk + '/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/x86_64-linux-android'
   inc_dirs += ndk + '/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include'
endif

install_inc_subdirs = ['dxg']

dep_dxheaders = dependency(
  'DirectX-Headers',
  fallback : ['DirectX-Headers', 'dep_dxheaders'])

install_headers(
  'include/d3dkmthk.h',
  'include/d3dkmdt.h',
  'include/d3dukmdt.h',
  subdir : 'dxg')

libdxg = library(
  'dxg',
  'src/d3dkmt-wsl.cpp',
  version: '1.0.0',
  include_directories : inc_dirs,
  cpp_args: cppargs,
  dependencies: [dep_dxheaders],
  install: true)

dep_libdxg = declare_dependency(
    link_with : libdxg,
    dependencies: [dep_dxheaders],
    include_directories : inc_dirs)

if meson.version().version_compare('>=0.54.0')
    meson.override_dependency('libdxg', dep_libdxg)
endif

pkg = import('pkgconfig')
pkg.generate(name : 'libdxg',
             description : 'Headers and library for libdxg',
             libraries : [libdxg],
             requires: 'DirectX-Headers',
             version : meson.project_version(),
             subdirs : install_inc_subdirs)
