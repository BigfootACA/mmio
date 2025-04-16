project('mmio', 'c',
	version : '0.2',
	default_options : [
		'warning_level=3',
		'werror=true',
		'optimization=3',
		'b_lto=true',
	]
)

src = files([
	'cmd_bitmask.c',
	'cmd_fill.c',
	'cmd_loadstore.c',
	'cmd_readwrite.c',
	'cmd_show.c',
	'cmd_watch.c',
	'dump.c',
	'map.c',
	'parse.c',
	'main.c',
])

executable('mmio', src, install: true)
