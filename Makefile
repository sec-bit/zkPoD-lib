MAKE ?= $(shell which make)

ifndef V
  V := @
else
  V :=
  MAKE := V=1 $(MAKE)
endif

all: pod_setup pod_publish pod_core

pod_setup pod_publish pod_core:
	$(V) cd $@; $(MAKE)

.PHONY: pod_setup pod_publish pod_core
