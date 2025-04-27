project(
    'sfind',
    ['c'],
    default_options: ['c_std=c99'],
    license: 'GPL-2.0',
    version: '1.0',
)

c_args = [
    '-Wall',
    '-Wextra',
    '-O2',
    '-D_GNU_SOURCE',
]

app_deps = [
    dependency('tinyc'),
]

app_sources = [
    'dirparser.c',
    'entry.c',
    'main.c',
]

executable(
    meson.project_name(),
    c_args: c_args,
    dependencies: app_deps,
    sources: app_sources,
    install: true,
)


