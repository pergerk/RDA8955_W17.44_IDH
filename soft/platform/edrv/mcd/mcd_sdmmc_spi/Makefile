## --------------Don't change the line below------------------ ##
# For more information on the different variables, and others you
# may define, see ${SOFT_DIR}/Makefile.skel and/or read the doc
include ${SOFT_WORKDIR}/env/compilation/compilevars.mk

LOCAL_NAME := platform/edrv/mcd/mcd_sdmmc_spi

LOCAL_API_DEPENDS := \
                     platform \
                     platform/chip/hal \
                     platform/chip/defs \
                     platform/edrv/pmd \
                     platform/edrv/mcd \
                     platform/edrv \
                     target \
                     platform/base/sx


C_SRC := mcd_spi_dual.c mcd_sdmmc.c mcd_api.c
S_SRC :=


## --------------Don't change the line below------------------ ##
include ${SOFT_WORKDIR}/env/compilation/compilerules.mk
