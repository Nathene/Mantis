project(
    'Mantis',
    'cpp',
    version: '0.1.2',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true',
        'buildtype=release',
        'b_ndebug=if-release',
    ],
)

add_project_arguments('-Wno-c2y-extensions', language: 'cpp')

catch2_dep = dependency('catch2-with-main', version: '>=3.0.0')

inc = include_directories('include')

mantis_lib = static_library(
    'mantis_core',
    sources: [
        'src/server.cpp',
        'src/parser.cpp',
    ],
    include_directories: inc,
)

mantis_dep = declare_dependency(
    include_directories: inc,
    link_with: mantis_lib,
)

executable(
    'mantis_node',
    sources: ['src/main.cpp'],
    dependencies: [mantis_dep],
    install: true
)

subdir('tests')
