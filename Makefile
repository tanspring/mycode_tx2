OUTNAME_RELEASE = ai_predict
OUTNAME_DEBUG   = ai_predict_debug
EXTRA_DIRECTORIES = ../common
MAKEFILE ?= ../Makefile.config
DO_CUDNN_CHECK = 1
include $(MAKEFILE)
