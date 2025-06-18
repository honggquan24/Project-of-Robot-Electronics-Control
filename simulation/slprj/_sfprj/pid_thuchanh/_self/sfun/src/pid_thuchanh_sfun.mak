# ------------------- Required for MSVC nmake ---------------------------------
# This file should be included at the top of a MAKEFILE as follows:


CPU = AMD64

MODEL     = pid_thuchanh
TARGET      = sfun
MODULE_SRCS   = c1_pid_thuchanh.cu c2_pid_thuchanh.cu half_type.cpp
MODEL_SRC  = pid_thuchanh_sfun.cu
MODEL_REG = pid_thuchanh_sfun_registry.cu
MAKEFILE    = pid_thuchanh_sfun.mak
MATLAB_ROOT  = C:\Program Files\MATLAB\R2023b
BUILDARGS   =

#--------------------------- Tool Specifications ------------------------------
#
#
MSVC_ROOT1 = $(MSDEVDIR:SharedIDE=vc)
MSVC_ROOT2 = $(MSVC_ROOT1:SHAREDIDE=vc)
MSVC_ROOT  = $(MSVC_ROOT2:sharedide=vc)

# Compiler tool locations, CC, LD, LIBCMD:
CC     = cl.exe
LD     = nvcc
NVCC   = nvcc
.SUFFIXES: .cu
LIBCMD = lib.exe
#------------------------------ Include/Lib Path ------------------------------

USER_INCLUDES   =  /I "C:\Users\hongquan\documents\zalo received files\Do án\slprj\_sfprj\pid_thuchanh\_self\sfun\src" /I "C:\Users\hongquan\documents\zalo received files\Do án"
AUX_INCLUDES   = 
MLSLSF_INCLUDES = \
    /I "$(MATLAB_ROOT)\sys\cuda\win64\cuda\include" \
    /I "$(MATLAB_ROOT)/toolbox/shared/gpucoder/src/cuda/export/include/cuda" \
    /I "$(MATLAB_ROOT)/toolbox/gpucoder/gpucoder/src/cuda/thrust" \
    /I "C:\Program Files\MATLAB\R2023b\extern\include" \
    /I "C:\Program Files\MATLAB\R2023b\simulink\include" \
    /I "C:\Program Files\MATLAB\R2023b\simulink\include\sf_runtime" \
    /I "C:\Program Files\MATLAB\R2023b\stateflow\c\mex\include" \
    /I "C:\Program Files\MATLAB\R2023b\rtw\c\src" \
    /I "C:\Users\hongquan\Documents\Zalo Received Files\Do án\slprj\_sfprj\pid_thuchanh\_self\sfun\src" 

COMPILER_INCLUDES = /I "$(MSVC_ROOT)\include"

THIRD_PARTY_INCLUDES   = 
INCLUDE_PATH = $(USER_INCLUDES) $(AUX_INCLUDES) $(MLSLSF_INCLUDES)\
 $(THIRD_PARTY_INCLUDES)
LIB_PATH     = "$(MSVC_ROOT)\lib"

CFLAGS = -Xcompiler "/c /Zp8 /GR /W3 /EHs /D_CRT_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /DMX_COMPAT_64 /DMATLAB_MEXCMD_RELEASE=R2018a /DMATLAB_MEX_FILE /nologo /MD " 
LDFLAGS = -Xlinker "/nologo /dll /MANIFEST /OPT:NOREF /export:mexFunction /export:mexfilerequiredapiversion  "
NVCC_INCLUDE_PATH = $(INCLUDE_PATH:/I=-I)
CUFLAGS_NO_RDC = -c -arch sm_75 -DMW_CUDA_ARCH=750 -noprof -ldir "$(MATLAB_ROOT)/bin/win64"
CUFLAGS = $(CUFLAGS_NO_RDC) -rdc=true
GPUCODER_CUDA_UTIL_LIB = "$(MATLAB_ROOT)/bin/win64/gpucoder_cuda.lib"
CULDFLAGS = -arch sm_75 -DMW_CUDA_ARCH=750 -Xlinker /NODEFAULTLIB:libcmt.lib -I"$(MATLAB_ROOT)\sys\cuda\win64\cuda\include" -L"$(MATLAB_ROOT)\sys\cuda\win64\cuda\lib\x64" -L"$(MATLAB_ROOT)\bin\win64" cudart.lib cublas.lib cusolver.lib cufft.lib cusparse.lib curand.lib
#----------------------------- Source Files -----------------------------------

REQ_SRCS  =  $(MODEL_SRC) $(MODEL_REG) $(MODULE_SRCS)

USER_OBJS =

AUX_ABS_OBJS =

THIRD_PARTY_OBJS     = \
     "c_mexapi_version.obj" \

REQ_OBJS = $(REQ_SRCS:.cpp=.obj)
REQ_OBJS2 = $(REQ_OBJS:.cu=.obj)
REQ_OBJS3 = $(REQ_OBJS2:.c=.obj)
OBJS = $(REQ_OBJS3) $(USER_OBJS) $(AUX_ABS_OBJS) $(THIRD_PARTY_OBJS)
OBJLIST_FILE = pid_thuchanh_sfun.mol
SFCLIB = 
AUX_LNK_OBJS =     
USER_LIBS = 
#--------------------------------- Rules --------------------------------------

MEX_FILE_NAME_WO_EXT = $(MODEL)_$(TARGET)
MEX_FILE_NAME = $(MEX_FILE_NAME_WO_EXT).mexw64
MEX_FILE_CSF =
all : $(MEX_FILE_NAME) $(MEX_FILE_CSF)

SYS_LIBS =   "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\sf_runtime.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmx.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmex.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmat.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libfixedpoint.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libut.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwstringutil.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwmathutil.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libemlrt.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwsl_log_load_blocks.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwsimulink.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwsl_sfcn_cov_bridge.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libcovrt.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwsl_simtarget_core.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwsl_simtarget_instrumentation.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwslexec_simlog.lib" "C:\Program Files\MATLAB\R2023b\extern\lib\win64\microsoft\libmwslccrt.lib"
$(MEX_FILE_NAME) : $(MAKEFILE) $(OBJS) $(SFCLIB) $(AUX_LNK_OBJS) $(USER_LIBS) $(THIRD_PARTY_LIBS)
 @echo ### Linking ...
 $(LD) $(LDFLAGS) $(CULDFLAGS) $(GPUCODER_CUDA_UTIL_LIB) -o $(MEX_FILE_NAME) -Xlinker /map:"$(MEX_FILE_NAME_WO_EXT).map"\
  $(USER_LIBS) $(SFCLIB) $(AUX_LNK_OBJS)\
  $(DSP_LIBS) $(THIRD_PARTY_LIBS)\
  $(SYS_LIBS) $(OBJS)
     mt -outputresource:"$(MEX_FILE_NAME);2" -manifest "$(MEX_FILE_NAME).manifest"
	@echo ### Created $@

.c.obj :
	@echo ### Compiling "$<"
	$(NVCC) $(CUFLAGS) $(CFLAGS) $(NVCC_INCLUDE_PATH) "$<"

.cpp.obj :
	@echo ### Compiling "$<"
	$(NVCC) $(CUFLAGS) $(CFLAGS) $(NVCC_INCLUDE_PATH) "$<"

.cu.obj :
	@echo ### Compiling "$<" 
	$(NVCC) $(CUFLAGS) $(CFLAGS) $(NVCC_INCLUDE_PATH) "$<"


c_mexapi_version.obj :  "C:\Program Files\MATLAB\R2023b\extern\version\c_mexapi_version.c"
	@echo ### Compiling "C:\Program Files\MATLAB\R2023b\extern\version\c_mexapi_version.c"
 $(NVCC) $(CUFLAGS_NO_RDC) $(CFLAGS) $(NVCC_INCLUDE_PATH) $(CUINCLUDE) "C:\Program Files\MATLAB\R2023b\extern\version\c_mexapi_version.c"
