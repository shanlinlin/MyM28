include ../../make.mk

targs := doc
doc-subdirs :=
doc-depends :=
doc-objects :=
doc-outputs := 模组鉴别说明.pdf 模组鉴别说明.aux 模组鉴别说明.log 模组鉴别说明.out 模组鉴别说明.toc
doc-outputs += database/database.xlsx ../app/include/.database.h
################################################################################
.PHONY: all clean $(targs:%=all-%) $(targs:%=clean-%) $(TARG:%=_all-%) $(TARG:%=_clean-%) $(TARG:%=%-_objects) $($(TARG)-subdirs)
all: $(targs:%=all-%)
clean: $(targs:%=clean-%)
-include $($($(TARG)-_depends))
$(targs:%=all-%):
	$(MAKE) -f$(word 1,$(MAKEFILE_LIST)) $(@:%=_%) TARG=$(@:all-%=%)
$(targs:%=clean-%):
	$(MAKE) -f$(word 1,$(MAKEFILE_LIST)) $(@:%=_%) TARG=$(@:clean-%=%)
$(TARG:%=_all-%): $($(TARG)-subdirs)
	$(MAKE) -f$(word 1,$(MAKEFILE_LIST)) $(THREAD:%=-j%) $(TARG)-_objects $(TARG)-_depends=$(TARG)-depends
$(TARG:%=_clean-%): $($(TARG)-subdirs)
	$(RM) $($(TARG)-objects) $($(TARG)-depends) $($(TARG)-outputs)
$(TARG:%=%-_objects): $($(TARG)-objects) $($(TARG)-outputs)
$($(TARG)-subdirs):
	$(MAKE) -f$(word 1,$(MAKEFILE_LIST)) $(MAKECMDGOALS) -C $(@) DIR=$(DIR)$(@)
################################################################################
模组鉴别说明.pdf: 模组鉴别说明.tex techmation.cls $(word 1,$(MAKEFILE_LIST))
	xelatex $(<:doc/%=%)
模组鉴别说明.aux 模组鉴别说明.log 模组鉴别说明.out: 模组鉴别说明.pdf

database/database.xlsx: database/database.json database/db_data.py $(word 1,$(MAKEFILE_LIST))
	@echo $(TARG): $(DIR)$(@)
	python3 database xlsx
../app/include/.database.h: database/database.json database/db_data.py $(word 1,$(MAKEFILE_LIST))
	@echo $(TARG): $(DIR)$(@)
	python3 database format head
	mv database/database.h $(@)
