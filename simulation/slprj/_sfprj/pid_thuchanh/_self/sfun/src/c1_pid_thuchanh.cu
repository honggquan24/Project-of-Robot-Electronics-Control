/* Include files */

#include "pid_thuchanh_sfun.h"
#include "c1_pid_thuchanh.h"
#ifdef utFree
#undef utFree
#endif

#ifdef utMalloc
#undef utMalloc
#endif

#ifdef __cplusplus

extern "C" void *utMalloc(size_t size);
extern "C" void utFree(void*);

#else

extern void *utMalloc(size_t size);
extern void utFree(void*);

#endif

/* Forward Declarations */

/* Type Definitions */

/* Named Constants */
const int32_T CALL_EVENT = -1;

/* Variable Declarations */

/* Variable Definitions */
static real_T _sfTime_;

/* Function Declarations */
static void initialize_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void initialize_params_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_start_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_terminate_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_setup_runtime_resources_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance);
static void mdl_cleanup_runtime_resources_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance);
static void enable_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void disable_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void sf_gateway_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void ext_mode_exec_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void c1_do_animation_call_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static const mxArray *get_sim_state_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance);
static void set_sim_state_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_st);
static void initSimStructsc1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);
static void initSubchartIOPointersc1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance);
static void c1_eML_blk_kernel(SFc1_pid_thuchanhInstanceStruct *chartInstance,
  real_T c1_b_vr, real_T c1_b_vl, real_T *c1_b_v, real_T *c1_b_omega);
static real_T c1_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct *chartInstance,
  const mxArray *c1_nullptr, const char_T *c1_identifier);
static real_T c1_b_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_u, const emlrtMsgIdentifier *c1_parentId);
static uint8_T c1_c_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_nullptr, const char_T *c1_identifier);
static uint8_T c1_d_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_u, const emlrtMsgIdentifier *c1_parentId);
static void init_dsm_address_info(SFc1_pid_thuchanhInstanceStruct *chartInstance);
static void init_simulink_io_address(SFc1_pid_thuchanhInstanceStruct
  *chartInstance);

/* Function Definitions */
static void initialize_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  emlrtLicenseCheckR2022a(chartInstance->c1_fEmlrtCtx,
    "EMLRT:runTime:MexFunctionNeedsLicense", "distrib_computing_toolbox", 2);
  sf_is_first_init_cond(chartInstance->S);
  sim_mode_is_external(chartInstance->S);
  _sfTime_ = sf_get_time(chartInstance->S);
  emlrtInitGPU(chartInstance->c1_fEmlrtCtx);
  cudaGetLastError();
}

static void initialize_params_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void mdl_start_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  sim_mode_is_external(chartInstance->S);
}

static void mdl_terminate_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  cudaError_t c1_errCode;
  c1_errCode = cudaGetLastError();
  if (c1_errCode != cudaSuccess) {
    emlrtThinCUDAError(static_cast<uint32_T>(c1_errCode), (char_T *)
                       cudaGetErrorName(c1_errCode), (char_T *)
                       cudaGetErrorString(c1_errCode), (char_T *)
                       "SimGPUErrorChecks", chartInstance->c1_fEmlrtCtx);
  }
}

static void mdl_setup_runtime_resources_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance)
{
  sfSetAnimationVectors(chartInstance->S, chartInstance->c1_JITStateAnimation,
                        chartInstance->c1_JITTransitionAnimation);
}

static void mdl_cleanup_runtime_resources_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance)
{
}

static void enable_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
}

static void disable_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
}

static void sf_gateway_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
  chartInstance->c1_JITTransitionAnimation[0] = 0U;
  c1_eML_blk_kernel(chartInstance, *chartInstance->c1_vr, *chartInstance->c1_vl,
                    chartInstance->c1_v, chartInstance->c1_omega);
  c1_do_animation_call_c1_pid_thuchanh(chartInstance);
}

static void ext_mode_exec_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void c1_do_animation_call_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  sfDoAnimationWrapper(chartInstance->S, false, true);
  sfDoAnimationWrapper(chartInstance->S, false, false);
}

static const mxArray *get_sim_state_c1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance)
{
  const mxArray *c1_b_y = NULL;
  const mxArray *c1_c_y = NULL;
  const mxArray *c1_d_y = NULL;
  const mxArray *c1_st;
  const mxArray *c1_y = NULL;
  c1_st = NULL;
  c1_st = NULL;
  c1_y = NULL;
  sf_mex_assign(&c1_y, sf_mex_createcellmatrix(3, 1), false);
  c1_b_y = NULL;
  sf_mex_assign(&c1_b_y, sf_mex_create("y", chartInstance->c1_omega, 0, 0U, 0U,
    0U, 0), false);
  sf_mex_setcell(c1_y, 0, c1_b_y);
  c1_c_y = NULL;
  sf_mex_assign(&c1_c_y, sf_mex_create("y", chartInstance->c1_v, 0, 0U, 0U, 0U,
    0), false);
  sf_mex_setcell(c1_y, 1, c1_c_y);
  c1_d_y = NULL;
  sf_mex_assign(&c1_d_y, sf_mex_create("y",
    &chartInstance->c1_is_active_c1_pid_thuchanh, 3, 0U, 0U, 0U, 0), false);
  sf_mex_setcell(c1_y, 2, c1_d_y);
  sf_mex_assign(&c1_st, c1_y, false);
  return c1_st;
}

static void set_sim_state_c1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_st)
{
  const mxArray *c1_u;
  c1_u = sf_mex_dup(c1_st);
  *chartInstance->c1_omega = c1_emlrt_marshallIn(chartInstance, sf_mex_dup
    (sf_mex_getcell(c1_u, 0)), "omega");
  *chartInstance->c1_v = c1_emlrt_marshallIn(chartInstance, sf_mex_dup
    (sf_mex_getcell(c1_u, 1)), "v");
  chartInstance->c1_is_active_c1_pid_thuchanh = c1_c_emlrt_marshallIn
    (chartInstance, sf_mex_dup(sf_mex_getcell(c1_u, 2)),
     "is_active_c1_pid_thuchanh");
  sf_mex_destroy(&c1_u);
  sf_mex_destroy(&c1_st);
}

static void initSimStructsc1_pid_thuchanh(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void initSubchartIOPointersc1_pid_thuchanh
  (SFc1_pid_thuchanhInstanceStruct *chartInstance)
{
}

const mxArray *sf_c1_pid_thuchanh_get_eml_resolved_functions_info()
{
  const mxArray *c1_nameCaptureInfo = NULL;
  c1_nameCaptureInfo = NULL;
  sf_mex_assign(&c1_nameCaptureInfo, sf_mex_create("nameCaptureInfo", NULL, 0,
    0U, 1U, 0U, 2, 0, 1), false);
  return c1_nameCaptureInfo;
}

static void c1_eML_blk_kernel(SFc1_pid_thuchanhInstanceStruct *chartInstance,
  real_T c1_b_vr, real_T c1_b_vl, real_T *c1_b_v, real_T *c1_b_omega)
{
  *c1_b_v = (c1_b_vr + c1_b_vl) / 2.0;
  *c1_b_omega = (c1_b_vr - c1_b_vl) / 0.2;
}

static real_T c1_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct *chartInstance,
  const mxArray *c1_nullptr, const char_T *c1_identifier)
{
  emlrtMsgIdentifier c1_thisId;
  real_T c1_y;
  c1_thisId.fIdentifier = const_cast<const char_T *>(c1_identifier);
  c1_thisId.fParent = NULL;
  c1_thisId.bParentIsCell = false;
  c1_y = c1_b_emlrt_marshallIn(chartInstance, sf_mex_dup(c1_nullptr), &c1_thisId);
  sf_mex_destroy(&c1_nullptr);
  return c1_y;
}

static real_T c1_b_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_u, const emlrtMsgIdentifier *c1_parentId)
{
  real_T c1_d;
  real_T c1_y;
  sf_mex_import(c1_parentId, sf_mex_dup(c1_u), &c1_d, 1, 0, 0U, 0, 0U, 0);
  c1_y = c1_d;
  sf_mex_destroy(&c1_u);
  return c1_y;
}

static uint8_T c1_c_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_nullptr, const char_T *c1_identifier)
{
  emlrtMsgIdentifier c1_thisId;
  uint8_T c1_y;
  c1_thisId.fIdentifier = const_cast<const char_T *>(c1_identifier);
  c1_thisId.fParent = NULL;
  c1_thisId.bParentIsCell = false;
  c1_y = c1_d_emlrt_marshallIn(chartInstance, sf_mex_dup(c1_nullptr), &c1_thisId);
  sf_mex_destroy(&c1_nullptr);
  return c1_y;
}

static uint8_T c1_d_emlrt_marshallIn(SFc1_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c1_u, const emlrtMsgIdentifier *c1_parentId)
{
  uint8_T c1_b_u;
  uint8_T c1_y;
  sf_mex_import(c1_parentId, sf_mex_dup(c1_u), &c1_b_u, 1, 3, 0U, 0, 0U, 0);
  c1_y = c1_b_u;
  sf_mex_destroy(&c1_u);
  return c1_y;
}

static void init_dsm_address_info(SFc1_pid_thuchanhInstanceStruct *chartInstance)
{
}

static void init_simulink_io_address(SFc1_pid_thuchanhInstanceStruct
  *chartInstance)
{
  chartInstance->c1_fEmlrtCtx = (void *)sfrtGetEmlrtCtx(chartInstance->S);
  chartInstance->c1_vr = (real_T *)ssGetInputPortSignal_wrapper(chartInstance->S,
    0);
  chartInstance->c1_v = (real_T *)ssGetOutputPortSignal_wrapper(chartInstance->S,
    1);
  chartInstance->c1_vl = (real_T *)ssGetInputPortSignal_wrapper(chartInstance->S,
    1);
  chartInstance->c1_omega = (real_T *)ssGetOutputPortSignal_wrapper
    (chartInstance->S, 2);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SFunction Glue Code */
void sf_c1_pid_thuchanh_get_check_sum(mxArray *plhs[])
{
  ((real_T *)mxGetPr((plhs[0])))[0] = (real_T)(234149045U);
  ((real_T *)mxGetPr((plhs[0])))[1] = (real_T)(2415487250U);
  ((real_T *)mxGetPr((plhs[0])))[2] = (real_T)(2130554710U);
  ((real_T *)mxGetPr((plhs[0])))[3] = (real_T)(2476181538U);
}

mxArray *sf_c1_pid_thuchanh_third_party_uses_info(void)
{
  mxArray * mxcell3p = mxCreateCellMatrix(1,0);
  return(mxcell3p);
}

mxArray *sf_c1_pid_thuchanh_jit_fallback_info(void)
{
  const char *infoFields[] = { "fallbackType", "fallbackReason",
    "hiddenFallbackType", "hiddenFallbackReason", "incompatibleSymbol" };

  mxArray *mxInfo = mxCreateStructMatrix(1, 1, 5, infoFields);
  mxArray *fallbackType = mxCreateString("pre");
  mxArray *fallbackReason = mxCreateString("GPUAcceleration");
  mxArray *hiddenFallbackType = mxCreateString("none");
  mxArray *hiddenFallbackReason = mxCreateString("");
  mxArray *incompatibleSymbol = mxCreateString("chartInfo");
  mxSetField(mxInfo, 0, infoFields[0], fallbackType);
  mxSetField(mxInfo, 0, infoFields[1], fallbackReason);
  mxSetField(mxInfo, 0, infoFields[2], hiddenFallbackType);
  mxSetField(mxInfo, 0, infoFields[3], hiddenFallbackReason);
  mxSetField(mxInfo, 0, infoFields[4], incompatibleSymbol);
  return mxInfo;
}

mxArray *sf_c1_pid_thuchanh_updateBuildInfo_args_info(void)
{
  mxArray *mxBIArgs = mxCreateCellMatrix(1,0);
  return mxBIArgs;
}

static const mxArray *sf_get_sim_state_info_c1_pid_thuchanh(void)
{
  const char *infoFields[] = { "chartChecksum", "varInfo" };

  mxArray *mxInfo = mxCreateStructMatrix(1, 1, 2, infoFields);
  mxArray *mxVarInfo = sf_mex_decode(
    "eNpjYPT0ZQACPiA+wMTAwAakOYCYiQECWKF8RiBmhtIQcRa4uAIQl1QWpILEi4uSPVOAdF5iLpi"
    "fWFrhmZeWDzbfggFhPhsW8xmRzOeEikPAB3vK9Ms4oOtnwaKfFUm/AJSfn5uanggNH1g4DZw/RM"
    "D+MCDgD0YUfzAylFHN/QoOlOmH2B9AwP2SaPEA4mcWxycml2SWpcYnG8YXZKbEl2SUJmck5mUgm"
    "QsCAGiJHpI="
    );
  mxArray *mxChecksum = mxCreateDoubleMatrix(1, 4, mxREAL);
  sf_c1_pid_thuchanh_get_check_sum(&mxChecksum);
  mxSetField(mxInfo, 0, infoFields[0], mxChecksum);
  mxSetField(mxInfo, 0, infoFields[1], mxVarInfo);
  return mxInfo;
}

static const char* sf_get_instance_specialization(void)
{
  return "sGWqU0b5zs97DgSjNrS7ko";
}

static void sf_opaque_initialize_c1_pid_thuchanh(void *chartInstanceVar)
{
  initialize_params_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*)
    chartInstanceVar);
  initialize_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_enable_c1_pid_thuchanh(void *chartInstanceVar)
{
  enable_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_disable_c1_pid_thuchanh(void *chartInstanceVar)
{
  disable_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_gateway_c1_pid_thuchanh(void *chartInstanceVar)
{
  sf_gateway_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static const mxArray* sf_opaque_get_sim_state_c1_pid_thuchanh(SimStruct* S)
{
  return get_sim_state_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct *)
    sf_get_chart_instance_ptr(S));     /* raw sim ctx */
}

static void sf_opaque_set_sim_state_c1_pid_thuchanh(SimStruct* S, const mxArray *
  st)
{
  set_sim_state_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*)
    sf_get_chart_instance_ptr(S), st);
}

static void sf_opaque_cleanup_runtime_resources_c1_pid_thuchanh(void
  *chartInstanceVar)
{
  if (chartInstanceVar!=NULL) {
    SimStruct *S = ((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar)->S;
    if (sim_mode_is_rtw_gen(S) || sim_mode_is_external(S)) {
      sf_clear_rtw_identifier(S);
      unload_pid_thuchanh_optimization_info();
    }

    mdl_cleanup_runtime_resources_c1_pid_thuchanh
      ((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
    ((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar)->
      ~SFc1_pid_thuchanhInstanceStruct();
    utFree(chartInstanceVar);
    if (ssGetUserData(S)!= NULL) {
      sf_free_ChartRunTimeInfo(S);
    }

    ssSetUserData(S,NULL);
  }
}

static void sf_opaque_mdl_start_c1_pid_thuchanh(void *chartInstanceVar)
{
  mdl_start_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*) chartInstanceVar);
  if (chartInstanceVar) {
    sf_reset_warnings_ChartRunTimeInfo(((SFc1_pid_thuchanhInstanceStruct*)
      chartInstanceVar)->S);
  }
}

static void sf_opaque_mdl_terminate_c1_pid_thuchanh(void *chartInstanceVar)
{
  mdl_terminate_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*)
    chartInstanceVar);
}

extern unsigned int sf_machine_global_initializer_called(void);
static void mdlProcessParameters_c1_pid_thuchanh(SimStruct *S)
{
  mdlProcessParamsCommon(S);
  if (sf_machine_global_initializer_called()) {
    initialize_params_c1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*)
      sf_get_chart_instance_ptr(S));
    initSubchartIOPointersc1_pid_thuchanh((SFc1_pid_thuchanhInstanceStruct*)
      sf_get_chart_instance_ptr(S));
  }
}

const char* sf_c1_pid_thuchanh_get_post_codegen_info(void)
{
  int i;
  const char* encStrCodegen [21] = {
    "eNrtWN2O20QUdtLtQlEpK4EKK1Wi3HGD1BZVpRKC3eanROx2I5xtJW6WiX1iDxnPeOcn2fQNeBD",
    "uEU/AJW8A4im4RFxxxnbS4ITEk6D+ILzyOmP7m++cM+dv7NU6xx4e1/D8/T3P28Xr63jWvfy4XI",
    "xrc2d+f8f7tBj//IbnBSKECLhvBgN64bkd3CRdIkmiPPeDkwS+AiWY0VTwDh+I6ljKByCBBzhBK",
    "qR24lU0MYzyYdvwwDKrJzENYj8WhoUPcEISnnA2+Sfe1OguMjaphEC3AUIdS2GiuM1ItNoKUo8b",
    "MQRDZRJnWynQvkmtqurYME1TBq0LCDpcaYJWUGv09TXR0NAXbka2+ip/ihZJyijh1W0dE+VDit6",
    "h4TQN8f+J0Wi9irzI16ecaCEpYa2ENWIiq2K7DOU8RrdmznbWiW5C30QR5ZG1rjQJcNQf/aSCrQ",
    "YNMQJJIjjhbryB1a51kS3wzC8rYjVN4DGRhwH6gYLQLQbRgZVPcGmhh9M4YYGTPoOO6kk6wjVyz",
    "RsdG0ob5Q2T5J6kNsJmvK0RrqrakLcd8AZhTLlheyI9ghGwjL9JNNkAm/M7gJWiYU+gd9jIdYx+",
    "w+m5gQLbEDyk1b1yVEJlReIRJvwKcJrYMIAQzTwTfTbRujgySoukgaHfPDqqyLeI7XANckACqJy",
    "vJaEKUODMrxx5Q6psICEaraQzLSvPkMfgRlBPDQxvjoUcoo1dC8MzW9lIcENDGEETNGRJroXe/Z",
    "gwU1HmRGFWtu5xqjDLuvEi1sbPRuCABDGEtgZSBseYZ3GCqkusbPk8RG1HVE+aoAJJ06qRZDChY",
    "/GzVupNUjjlQy7GvC1F4hddzAq/AsCsQSTHUvYAS6mctFH4alJLOO9l2d21YbB2JpqRvvWNh8Cx",
    "GlpdbfUmAUZVi2O7iQJtg/XpU2xHuKJKYws4aWUxEGZ98IH3rA/eWdIHvzXXB+8V4+D2WUrDMx0",
    "brMI8zua5NTfP1Qr99LQPX4XzFnDeDDe9fjiHry3h9eauZb4r9b+/Xy/x1fGvVvMW7PRmiWenhN",
    "stbHX/x/1f/vzt+g/fiW8++PL7195ex19b4K9lvy0uveS2X7lWjG9Me6VZZh4tJC/77hdr/OB6y",
    "Q/sWD18cn56q3/3qbp/rxn53z6S/r2hyOf7qb5a3ksleaf3b9r+DAM3ywMy6ITFvseOicn7cTv/",
    "J3Py7q6xx5U5f8Kd3+fb4W8clPHL7HW5ZC87FglEpOS/L06Pdw7K/rizBr+Hv0b/mvw3D7bD5/z",
    "dNfLvl9ZhP9t3nBFbXeCsnMkW88um8eqKe1X4/sdVX4cqdbD+nHHec8Ztq59rfX/V31+Vj73S+3",
    "svsR6r8qhLn/ey6fWr59aHvV+MP5t9W2jElIVLdjfFY9yADJY9/Y/49x+O9nu3GLes/YoPul9/f",
    "MgJm+B2Jt8uFre70n5LnD2SQNTyPeOLqCfT67r91tVSfNvxmPJQjNVHt+/cvbNNffoLgNQKtw==",
    ""
  };

  static char newstr [1501] = "";
  newstr[0] = '\0';
  for (i = 0; i < 21; i++) {
    strcat(newstr, encStrCodegen[i]);
  }

  return newstr;
}

static void mdlSetWorkWidths_c1_pid_thuchanh(SimStruct *S)
{
  const char* newstr = sf_c1_pid_thuchanh_get_post_codegen_info();
  sf_set_work_widths(S, newstr);
  ssSetChecksum0(S,(3608785465U));
  ssSetChecksum1(S,(2954287866U));
  ssSetChecksum2(S,(559968137U));
  ssSetChecksum3(S,(319268171U));
}

static void mdlRTW_c1_pid_thuchanh(SimStruct *S)
{
  if (sim_mode_is_rtw_gen(S)) {
    ssWriteRTWStrParam(S, "StateflowChartType", "Embedded MATLAB");
  }
}

static void mdlSetupRuntimeResources_c1_pid_thuchanh(SimStruct *S)
{
  SFc1_pid_thuchanhInstanceStruct *chartInstance;
  chartInstance = (SFc1_pid_thuchanhInstanceStruct *)utMalloc(sizeof
    (SFc1_pid_thuchanhInstanceStruct));
  if (chartInstance==NULL) {
    sf_mex_error_message("Could not allocate memory for chart instance.");
  }

  memset(chartInstance, 0, sizeof(SFc1_pid_thuchanhInstanceStruct));
  chartInstance = new (chartInstance) SFc1_pid_thuchanhInstanceStruct;
  chartInstance->chartInfo.chartInstance = chartInstance;
  chartInstance->chartInfo.isEMLChart = 1;
  chartInstance->chartInfo.chartInitialized = 0;
  chartInstance->chartInfo.sFunctionGateway = sf_opaque_gateway_c1_pid_thuchanh;
  chartInstance->chartInfo.initializeChart =
    sf_opaque_initialize_c1_pid_thuchanh;
  chartInstance->chartInfo.mdlStart = sf_opaque_mdl_start_c1_pid_thuchanh;
  chartInstance->chartInfo.mdlTerminate =
    sf_opaque_mdl_terminate_c1_pid_thuchanh;
  chartInstance->chartInfo.mdlCleanupRuntimeResources =
    sf_opaque_cleanup_runtime_resources_c1_pid_thuchanh;
  chartInstance->chartInfo.enableChart = sf_opaque_enable_c1_pid_thuchanh;
  chartInstance->chartInfo.disableChart = sf_opaque_disable_c1_pid_thuchanh;
  chartInstance->chartInfo.getSimState = sf_opaque_get_sim_state_c1_pid_thuchanh;
  chartInstance->chartInfo.setSimState = sf_opaque_set_sim_state_c1_pid_thuchanh;
  chartInstance->chartInfo.getSimStateInfo =
    sf_get_sim_state_info_c1_pid_thuchanh;
  chartInstance->chartInfo.zeroCrossings = NULL;
  chartInstance->chartInfo.outputs = NULL;
  chartInstance->chartInfo.derivatives = NULL;
  chartInstance->chartInfo.mdlRTW = mdlRTW_c1_pid_thuchanh;
  chartInstance->chartInfo.mdlSetWorkWidths = mdlSetWorkWidths_c1_pid_thuchanh;
  chartInstance->chartInfo.extModeExec = NULL;
  chartInstance->chartInfo.restoreLastMajorStepConfiguration = NULL;
  chartInstance->chartInfo.restoreBeforeLastMajorStepConfiguration = NULL;
  chartInstance->chartInfo.storeCurrentConfiguration = NULL;
  chartInstance->chartInfo.callAtomicSubchartUserFcn = NULL;
  chartInstance->chartInfo.callAtomicSubchartAutoFcn = NULL;
  chartInstance->chartInfo.callAtomicSubchartEventFcn = NULL;
  chartInstance->S = S;
  chartInstance->chartInfo.dispatchToExportedFcn = NULL;
  sf_init_ChartRunTimeInfo(S, &(chartInstance->chartInfo), false, 0);
  init_dsm_address_info(chartInstance);
  init_simulink_io_address(chartInstance);
  if (!sim_mode_is_rtw_gen(S)) {
  }

  mdl_setup_runtime_resources_c1_pid_thuchanh(chartInstance);
}

void c1_pid_thuchanh_method_dispatcher(SimStruct *S, int_T method, void *data)
{
  switch (method) {
   case SS_CALL_MDL_SETUP_RUNTIME_RESOURCES:
    mdlSetupRuntimeResources_c1_pid_thuchanh(S);
    break;

   case SS_CALL_MDL_SET_WORK_WIDTHS:
    mdlSetWorkWidths_c1_pid_thuchanh(S);
    break;

   case SS_CALL_MDL_PROCESS_PARAMETERS:
    mdlProcessParameters_c1_pid_thuchanh(S);
    break;

   default:
    /* Unhandled method */
    sf_mex_error_message("Stateflow Internal Error:\n"
                         "Error calling c1_pid_thuchanh_method_dispatcher.\n"
                         "Can't handle method %d.\n", method);
    break;
  }
}
