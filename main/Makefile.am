ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = gbc gbx . lib share man
EXTRA_DIST = TODO reconf tools spec README mime gb.*.h
BUILT_SOURCES = trunk_version.h
CLEANFILES = trunk_version.h

.PHONY: trunk_version.h
trunk_version.h:
	@if test -d ../.svn ; then \
		if test ../.svn/wc.db -nt trunk_version.h || test ../.svn/entries -nt trunk_version.h; then \
			echo '#define TRUNK_VERSION "'`LC_ALL=C svn info 2>/dev/null | grep Revision | egrep -wo [0-9]+`'"' > trunk_version.h; \
		fi \
	elif test -d ../.git ; then \
		GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`; \
		if test \! -e ../.git/refs/heads/$$GIT_BRANCH -o ../.git/refs/heads/$$GIT_BRANCH -nt trunk_version.h; then \
			if GIT_TAG=$$(git describe --exact-match $$GIT_BRANCH 2>/dev/null) ; then \
				GIT_BRANCH="$$GIT_TAG"; \
			fi; \
			GIT_HASH=`git rev-parse --short HEAD`; \
			echo '#define TRUNK_VERSION "'$$GIT_HASH' ('$$GIT_BRANCH')"' > trunk_version.h; \
			echo '#define TRUNK_VERSION_GIT 1' >> trunk_version.h; \
		fi; \
	else \
		touch trunk_version.h; \
	fi

install-exec-local:
	@rm -f $(srcdir)/../warnings.log
	
	@if test "x$(ROOT)" != "x"; then \
	 echo "[Installing with ROOT=$(ROOT)]"; \
	fi
	@if test "x$(DESTDIR)" != "x"; then \
	  echo "[Installing with DESTDIR=$(DESTDIR)]"; \
	  ROOT=$DESTDIR; \
	fi
	
	@echo "Installing runtime symbolic link"
	@$(LN_S) -f gbx$(GAMBAS_VERSION) $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION) || true
	
	@if test x"$(XDG_UTILS)" != x; then \
	  echo "Registering Gambas executable mimetype"; \
	  xdg-icon-resource install --novendor --context mimetypes --size 256 $(srcdir)/mime/application-x-gambas3.png application-x-gambas3; \
	  xdg-mime install $(srcdir)/mime/application-x-gambas3.xml; \
	fi

	@echo "Installing 'gb' component..."
	@$(INSTALL) -d $(DESTDIR)$(gbdatadir)/info
	@$(INSTALL) lib/gb.component $(DESTDIR)$(gblibdir)
	@$(DESTDIR)$(bindir)/gbi$(GAMBAS_VERSION) -r $(DESTDIR)$(prefix) gb
	@rm -f $(DESTDIR)$(gblibdir)/gb.la
	@rm -f $(DESTDIR)$(gblibdir)/gb.so*

	@echo "Installing compiler tools..."
	@(cd $(srcdir)/tools; d=`pwd`; \
	  for p in gb*; do \
	    echo "Compiling $$p..."; cd $$d/$$p; \
	    $(DESTDIR)$(bindir)/gbc$(GAMBAS_VERSION) -ag -r $(DESTDIR)$(prefix); \
	    if test $$? -eq 0; then \
	      $(DESTDIR)$(bindir)/gba$(GAMBAS_VERSION); \
	      rm -rf .gambas; \
	      echo "Installing $$p..."; \
	      $(INSTALL) $$p.gambas $(DESTDIR)$(bindir); \
	      $(LN_S) -f $$p.gambas $(DESTDIR)$(bindir)/$$p || true; \
	    else \
	      echo "|| Unable to compile $$p" >> ../../../warnings.log; \
	    fi \
	  done)

uninstall-local:
	@echo "Uninstalling compiler tools..."
	@(cd $(srcdir)/tools; for p in gb*; do rm -f $(DESTDIR)$(bindir)/$$p.gambas $(DESTDIR)$(bindir)/$$p; done)

	@echo "Uninstalling 'gb' component..."
	@rm -f $(DESTDIR)$(gblibdir)/gb.component
	@rm -rf $(DESTDIR)$(gbdatadir)/info/gb.info
	@rm -rf $(DESTDIR)$(gbdatadir)/info/gb.list

	@if test x"$(XDG_UTILS)" != x; then \
	  echo "Unregistering Gambas executable mimetype"; \
	  xdg-mime uninstall $(srcdir)/mime/application-x-gambas3.xml; \
	  xdg-icon-resource uninstall --context mimetypes --size 256 application-x-gambas3; \
	fi
	
	@echo "Uninstalling runtime symbolic link"
	@rm -f $(DESTDIR)$(bindir)/gbr$(GAMBAS_VERSION)

	@rmdir --ignore-fail-on-non-empty $(DESTDIR)$(gbdatadir)/info $(DESTDIR)$(gbdatadir)/control $(DESTDIR)$(gbdatadir)

dist-hook:
	@rm -f $(distdir)/trunk_version.h
	@(cd $(distdir)/tools; \
	rm -rf `find . -name ".gambas" -o -name ".action" -o -name ".lock" -o -name ".xvpics" -o -name "*~" -o -name "*.out"  -o -name "*.pot" -o -name "*.gambas" -o -name "core*" -o -name ".kdbg*" -o -name ".svn"`;)


