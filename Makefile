SUBDIRS := injection testLib_one testLib_two tests sample

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ clean
	$(MAKE) -C $@ 

.PHONY: all $(SUBDIRS)
