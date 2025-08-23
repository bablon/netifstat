project('netifstat', 'c',
  version: '1.0.0',
  license: 'GPL-3.0-or-later')
adw_dep = dependency('libadwaita-1')
gio_unix_dep = dependency('gio-unix-2.0')
libnl_genl_dep = dependency('libnl-genl-3.0')

gnome = import('gnome')
resources = gnome.compile_resources('netifstat.resources',
  'netifstat.gresource.xml',
  c_name: 'netifstat')

executable('netifstat',
  ['netifstat.c',
   'netif-widget.c',
   'kgx-theme-switcher.c'] + resources,
  install: true,
  dependencies: [adw_dep, gio_unix_dep, libnl_genl_dep])
