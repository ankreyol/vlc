# qtwayland

WTWAYLAND_VERSION := 5.12.0-beta1
WTWAYLAND_URL := http://download.qt.io/development_releases/qt/5.12/5.12.0-beta1/submodules/qtwayland-everywhere-src-5.12.0-beta1.tar.xz

DEPS_qtwayland += qt $(DEPS_qt)

$(TARBALLS)/qtwayland-$(WTWAYLAND_VERSION).tar.xz:
	$(call download,$(WTWAYLAND_URL))

.sum-qtwayland: qtwayland-$(WTWAYLAND_VERSION).tar.xz

qtwayland: qtwayland-$(WTWAYLAND_VERSION).tar.xz .sum-qtwayland
	$(UNPACK)
	mv qtwayland-everywhere-src-$(WTWAYLAND_VERSION) qtwayland-$(WTWAYLAND_VERSION)
	$(MOVE)

.qtwayland: qtwayland
	cd $< && $(PREFIX)/bin/qmake
	# Make && Install libraries
	cd $< && $(MAKE)
	cd $< && $(MAKE) -C src sub-plugins-install_subtargets
	cp $(PREFIX)/plugins/platforms/libqwayland-egl.a $(PREFIX)/lib/
	cp $(PREFIX)/plugins/wayland-shell-integration/libwl-shell.a $(PREFIX)/lib/
	cd $(PREFIX)/lib/pkgconfig; sed -i.orig -e 's/ -lQt5WaylandClient/ -lqwayland-egl -lwl-shell -lQt5WaylandClient -lQt5EglSupport -lEGL -lwayland-egl/' Qt5WaylandClient.pc
	$(call pkg_static,"$(PREFIX)/lib/pkgconfig/Qt5WaylandClient.pc")
	touch $@
