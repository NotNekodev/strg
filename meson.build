project('strg', 'c',
        version : '0.1',
        default_options : ['warning_level=2'])

add_project_arguments(
    '-DWLR_USE_UNSTABLE',
    '-D_POSIX_C_SOURCE=200112L',
    language : 'c'
)


wlroots = dependency('wlroots-0.19')
xkbcommon = dependency('xkbcommon')
wayland_dep = dependency('wayland-server')
wayland_scanner = find_program('wayland-scanner')


inc = include_directories('include')

srcs = files(
    'src/main.c',
    'src/core/input.c',
    'src/core/output.c',
    'src/core/xdg_shell.c',
    'src/style/decorations.c'
)

xdg_shell_xml = '/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml'

xdg_shell_header = custom_target(
    'xdg-shell-protocol',
    input : xdg_shell_xml,
    output : 'xdg-shell-protocol.h',
    command : [
        wayland_scanner,
        'server-header',
        '@INPUT@',
        '@OUTPUT@'
    ]
)

executable(
    'strg',
    srcs,
    xdg_shell_header,
    include_directories : inc,
    dependencies : [
        wlroots,
        wayland_dep,
        xkbcommon
    ],
    install : true
)

