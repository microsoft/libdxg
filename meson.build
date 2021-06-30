# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project('libdxg', 'cpp', version : '1.0.0',
  default_options: 'cpp_args=-std=c++17 -Wno-unknown-pragmas')

inc_dirs = [include_directories('include', is_system : true)]
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
  dependencies: [dep_dxheaders])

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
