project('strg', 'c',
        version : '0.1',
        default_options : ['warning_level=2'])

add_project_arguments(
    '-DWLR_USE_UNSTABLE',
    '-D_POSIX_C_SOURCE=200809L',
    language : 'c'
)


wlroots = dependency('wlroots-0.19')
xkbcommon = dependency('xkbcommon')
wayland_dep = dependency('wayland-server')
wayland_scanner = find_program('wayland-scanner')
cairo = dependency('cairo')
pango = dependency('pango')
pangocairo = dependency('pangocairo')
lua = dependency('lua')

inc = include_directories('include')

srcs = files(
    'src/main.c',
    'src/core/input.c',
    'src/core/output.c',
    'src/core/protocol/xdg/xdg.c',
    'src/core/protocol/xdg/xdg_ops.c',
    'src/core/protocol/xwayland/xwl.c',
    'src/core/protocol/xwayland/xwl_ops.c',
    'src/style/decorations.c',
    'src/config/config.c',
    'src/config/lua_funcs.c',
    'src/util/sigutil.c',
    'src/util/window_util.c',
    'src/util/logutil.c',
    'src/util/dynarray.c',
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
        xkbcommon,
        cairo,
        pango,
        pangocairo,
        lua,
    ],
    install : true
)

