/* Include files */

#include "pid_thuchanh_sfun.h"
#include "c2_pid_thuchanh.h"
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
static void initialize_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void initialize_params_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_start_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_terminate_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void mdl_setup_runtime_resources_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance);
static void mdl_cleanup_runtime_resources_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance);
static void enable_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void disable_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void sf_gateway_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void ext_mode_exec_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void c2_do_animation_call_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static const mxArray *get_sim_state_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance);
static void set_sim_state_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_st);
static void initSimStructsc2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);
static void initSubchartIOPointersc2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance);
static void c2_eML_blk_kernel(SFc2_pid_thuchanhInstanceStruct *chartInstance,
  real_T c2_b_vr, real_T c2_b_vl, real_T *c2_b_v, real_T *c2_b_omega);
static real_T c2_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct *chartInstance,
  const mxArray *c2_nullptr, const char_T *c2_identifier);
static real_T c2_b_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_u, const emlrtMsgIdentifier *c2_parentId);
static uint8_T c2_c_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_nullptr, const char_T *c2_identifier);
static uint8_T c2_d_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_u, const emlrtMsgIdentifier *c2_parentId);
static void init_dsm_address_info(SFc2_pid_thuchanhInstanceStruct *chartInstance);
static void init_simulink_io_address(SFc2_pid_thuchanhInstanceStruct
  *chartInstance);

/* Function Definitions */
static void initialize_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  emlrtLicenseCheckR2022a(chartInstance->c2_fEmlrtCtx,
    "EMLRT:runTime:MexFunctionNeedsLicense", "distrib_computing_toolbox", 2);
  sf_is_first_init_cond(chartInstance->S);
  sim_mode_is_external(chartInstance->S);
  _sfTime_ = sf_get_time(chartInstance->S);
  emlrtInitGPU(chartInstance->c2_fEmlrtCtx);
  cudaGetLastError();
}

static void initialize_params_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void mdl_start_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  sim_mode_is_external(chartInstance->S);
}

static void mdl_terminate_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  cudaError_t c2_errCode;
  c2_errCode = cudaGetLastError();
  if (c2_errCode != cudaSuccess) {
    emlrtThinCUDAError(static_cast<uint32_T>(c2_errCode), (char_T *)
                       cudaGetErrorName(c2_errCode), (char_T *)
                       cudaGetErrorString(c2_errCode), (char_T *)
                       "SimGPUErrorChecks", chartInstance->c2_fEmlrtCtx);
  }
}

static void mdl_setup_runtime_resources_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance)
{
  sfSetAnimationVectors(chartInstance->S, chartInstance->c2_JITStateAnimation,
                        chartInstance->c2_JITTransitionAnimation);
}

static void mdl_cleanup_runtime_resources_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance)
{
}

static void enable_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
}

static void disable_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
}

static void sf_gateway_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  _sfTime_ = sf_get_time(chartInstance->S);
  chartInstance->c2_JITTransitionAnimation[0] = 0U;
  c2_eML_blk_kernel(chartInstance, *chartInstance->c2_vr, *chartInstance->c2_vl,
                    chartInstance->c2_v, chartInstance->c2_omega);
  c2_do_animation_call_c2_pid_thuchanh(chartInstance);
}

static void ext_mode_exec_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void c2_do_animation_call_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  sfDoAnimationWrapper(chartInstance->S, false, true);
  sfDoAnimationWrapper(chartInstance->S, false, false);
}

static const mxArray *get_sim_state_c2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance)
{
  const mxArray *c2_b_y = NULL;
  const mxArray *c2_c_y = NULL;
  const mxArray *c2_d_y = NULL;
  const mxArray *c2_st;
  const mxArray *c2_y = NULL;
  c2_st = NULL;
  c2_st = NULL;
  c2_y = NULL;
  sf_mex_assign(&c2_y, sf_mex_createcellmatrix(3, 1), false);
  c2_b_y = NULL;
  sf_mex_assign(&c2_b_y, sf_mex_create("y", chartInstance->c2_omega, 0, 0U, 0U,
    0U, 0), false);
  sf_mex_setcell(c2_y, 0, c2_b_y);
  c2_c_y = NULL;
  sf_mex_assign(&c2_c_y, sf_mex_create("y", chartInstance->c2_v, 0, 0U, 0U, 0U,
    0), false);
  sf_mex_setcell(c2_y, 1, c2_c_y);
  c2_d_y = NULL;
  sf_mex_assign(&c2_d_y, sf_mex_create("y",
    &chartInstance->c2_is_active_c2_pid_thuchanh, 3, 0U, 0U, 0U, 0), false);
  sf_mex_setcell(c2_y, 2, c2_d_y);
  sf_mex_assign(&c2_st, c2_y, false);
  return c2_st;
}

static void set_sim_state_c2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_st)
{
  const mxArray *c2_u;
  c2_u = sf_mex_dup(c2_st);
  *chartInstance->c2_omega = c2_emlrt_marshallIn(chartInstance, sf_mex_dup
    (sf_mex_getcell(c2_u, 0)), "omega");
  *chartInstance->c2_v = c2_emlrt_marshallIn(chartInstance, sf_mex_dup
    (sf_mex_getcell(c2_u, 1)), "v");
  chartInstance->c2_is_active_c2_pid_thuchanh = c2_c_emlrt_marshallIn
    (chartInstance, sf_mex_dup(sf_mex_getcell(c2_u, 2)),
     "is_active_c2_pid_thuchanh");
  sf_mex_destroy(&c2_u);
  sf_mex_destroy(&c2_st);
}

static void initSimStructsc2_pid_thuchanh(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
}

static void initSubchartIOPointersc2_pid_thuchanh
  (SFc2_pid_thuchanhInstanceStruct *chartInstance)
{
}

const mxArray *sf_c2_pid_thuchanh_get_eml_resolved_functions_info()
{
  const mxArray *c2_nameCaptureInfo = NULL;
  c2_nameCaptureInfo = NULL;
  sf_mex_assign(&c2_nameCaptureInfo, sf_mex_create("nameCaptureInfo", NULL, 0,
    0U, 1U, 0U, 2, 0, 1), false);
  return c2_nameCaptureInfo;
}

static void c2_eML_blk_kernel(SFc2_pid_thuchanhInstanceStruct *chartInstance,
  real_T c2_b_vr, real_T c2_b_vl, real_T *c2_b_v, real_T *c2_b_omega)
{
  *c2_b_v = (c2_b_vr + c2_b_vl) / 2.0;
  *c2_b_omega = (c2_b_vr - c2_b_vl) / 0.2;
}

static real_T c2_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct *chartInstance,
  const mxArray *c2_nullptr, const char_T *c2_identifier)
{
  emlrtMsgIdentifier c2_thisId;
  real_T c2_y;
  c2_thisId.fIdentifier = const_cast<const char_T *>(c2_identifier);
  c2_thisId.fParent = NULL;
  c2_thisId.bParentIsCell = false;
  c2_y = c2_b_emlrt_marshallIn(chartInstance, sf_mex_dup(c2_nullptr), &c2_thisId);
  sf_mex_destroy(&c2_nullptr);
  return c2_y;
}

static real_T c2_b_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_u, const emlrtMsgIdentifier *c2_parentId)
{
  real_T c2_d;
  real_T c2_y;
  sf_mex_import(c2_parentId, sf_mex_dup(c2_u), &c2_d, 1, 0, 0U, 0, 0U, 0);
  c2_y = c2_d;
  sf_mex_destroy(&c2_u);
  return c2_y;
}

static uint8_T c2_c_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_nullptr, const char_T *c2_identifier)
{
  emlrtMsgIdentifier c2_thisId;
  uint8_T c2_y;
  c2_thisId.fIdentifier = const_cast<const char_T *>(c2_identifier);
  c2_thisId.fParent = NULL;
  c2_thisId.bParentIsCell = false;
  c2_y = c2_d_emlrt_marshallIn(chartInstance, sf_mex_dup(c2_nullptr), &c2_thisId);
  sf_mex_destroy(&c2_nullptr);
  return c2_y;
}

static uint8_T c2_d_emlrt_marshallIn(SFc2_pid_thuchanhInstanceStruct
  *chartInstance, const mxArray *c2_u, const emlrtMsgIdentifier *c2_parentId)
{
  uint8_T c2_b_u;
  uint8_T c2_y;
  sf_mex_import(c2_parentId, sf_mex_dup(c2_u), &c2_b_u, 1, 3, 0U, 0, 0U, 0);
  c2_y = c2_b_u;
  sf_mex_destroy(&c2_u);
  return c2_y;
}

static void init_dsm_address_info(SFc2_pid_thuchanhInstanceStruct *chartInstance)
{
}

static void init_simulink_io_address(SFc2_pid_thuchanhInstanceStruct
  *chartInstance)
{
  chartInstance->c2_fEmlrtCtx = (void *)sfrtGetEmlrtCtx(chartInstance->S);
  chartInstance->c2_vr = (real_T *)ssGetInputPortSignal_wrapper(chartInstance->S,
    0);
  chartInstance->c2_v = (real_T *)ssGetOutputPortSignal_wrapper(chartInstance->S,
    1);
  chartInstance->c2_vl = (real_T *)ssGetInputPortSignal_wrapper(chartInstance->S,
    1);
  chartInstance->c2_omega = (real_T *)ssGetOutputPortSignal_wrapper
    (chartInstance->S, 2);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SFunction Glue Code */
void sf_c2_pid_thuchanh_get_check_sum(mxArray *plhs[])
{
  ((real_T *)mxGetPr((plhs[0])))[0] = (real_T)(234149045U);
  ((real_T *)mxGetPr((plhs[0])))[1] = (real_T)(2415487250U);
  ((real_T *)mxGetPr((plhs[0])))[2] = (real_T)(2130554710U);
  ((real_T *)mxGetPr((plhs[0])))[3] = (real_T)(2476181538U);
}

mxArray *sf_c2_pid_thuchanh_third_party_uses_info(void)
{
  mxArray * mxcell3p = mxCreateCellMatrix(1,0);
  return(mxcell3p);
}

mxArray *sf_c2_pid_thuchanh_jit_fallback_info(void)
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

mxArray *sf_c2_pid_thuchanh_updateBuildInfo_args_info(void)
{
  mxArray *mxBIArgs = mxCreateCellMatrix(1,0);
  return mxBIArgs;
}

static const mxArray *sf_get_sim_state_info_c2_pid_thuchanh(void)
{
  const char *infoFields[] = { "chartChecksum", "varInfo" };

  mxArray *mxInfo = mxCreateStructMatrix(1, 1, 2, infoFields);
  mxArray *mxVarInfo = sf_mex_decode(
    "eNpjYPT0ZQACPiA+wMTAwAakOYCYiQECWKF8RiBmhtIQcRa4uAIQl1QWpILEi4uSPVOAdF5iLpi"
    "fWFrhmZeWDzbfggFhPhsW8xmRzOeEikPAB3vK9Ms4oOtnwaKfFUm/AJSfn5uanggNH1g4DZw/RM"
    "D+MCDgD0YUfzAylFHN/QoOlOmH2B9AwP2SaPEA4mcWxycml2SWpcYnG8UXZKbEl2SUJmck5mUgm"
    "QsCAGimHpM="
    );
  mxArray *mxChecksum = mxCreateDoubleMatrix(1, 4, mxREAL);
  sf_c2_pid_thuchanh_get_check_sum(&mxChecksum);
  mxSetField(mxInfo, 0, infoFields[0], mxChecksum);
  mxSetField(mxInfo, 0, infoFields[1], mxVarInfo);
  return mxInfo;
}

static const char* sf_get_instance_specialization(void)
{
  return "sGWqU0b5zs97DgSjNrS7ko";
}

static void sf_opaque_initialize_c2_pid_thuchanh(void *chartInstanceVar)
{
  initialize_params_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*)
    chartInstanceVar);
  initialize_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_enable_c2_pid_thuchanh(void *chartInstanceVar)
{
  enable_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_disable_c2_pid_thuchanh(void *chartInstanceVar)
{
  disable_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static void sf_opaque_gateway_c2_pid_thuchanh(void *chartInstanceVar)
{
  sf_gateway_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
}

static const mxArray* sf_opaque_get_sim_state_c2_pid_thuchanh(SimStruct* S)
{
  return get_sim_state_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct *)
    sf_get_chart_instance_ptr(S));     /* raw sim ctx */
}

static void sf_opaque_set_sim_state_c2_pid_thuchanh(SimStruct* S, const mxArray *
  st)
{
  set_sim_state_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*)
    sf_get_chart_instance_ptr(S), st);
}

static void sf_opaque_cleanup_runtime_resources_c2_pid_thuchanh(void
  *chartInstanceVar)
{
  if (chartInstanceVar!=NULL) {
    SimStruct *S = ((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar)->S;
    if (sim_mode_is_rtw_gen(S) || sim_mode_is_external(S)) {
      sf_clear_rtw_identifier(S);
      unload_pid_thuchanh_optimization_info();
    }

    mdl_cleanup_runtime_resources_c2_pid_thuchanh
      ((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
    ((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar)->
      ~SFc2_pid_thuchanhInstanceStruct();
    utFree(chartInstanceVar);
    if (ssGetUserData(S)!= NULL) {
      sf_free_ChartRunTimeInfo(S);
    }

    ssSetUserData(S,NULL);
  }
}

static void sf_opaque_mdl_start_c2_pid_thuchanh(void *chartInstanceVar)
{
  mdl_start_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*) chartInstanceVar);
  if (chartInstanceVar) {
    sf_reset_warnings_ChartRunTimeInfo(((SFc2_pid_thuchanhInstanceStruct*)
      chartInstanceVar)->S);
  }
}

static void sf_opaque_mdl_terminate_c2_pid_thuchanh(void *chartInstanceVar)
{
  mdl_terminate_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*)
    chartInstanceVar);
}

extern unsigned int sf_machine_global_initializer_called(void);
static void mdlProcessParameters_c2_pid_thuchanh(SimStruct *S)
{
  mdlProcessParamsCommon(S);
  if (sf_machine_global_initializer_called()) {
    initialize_params_c2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*)
      sf_get_chart_instance_ptr(S));
    initSubchartIOPointersc2_pid_thuchanh((SFc2_pid_thuchanhInstanceStruct*)
      sf_get_chart_instance_ptr(S));
  }
}

const char* sf_c2_pid_thuchanh_get_post_codegen_info(void)
{
  int i;
  const char* encStrCodegen [21] = {
    "eNrtWN2O20QUdtLttkWlrFTUslIlyh03SGVR1VZCsNv8lIjdboSzrcTNdmKf2NOMZ7zzk2z6Bjw",
    "I94gn4JI3APEUXCKuOOM4aXBC4klQfxBeeZ2x/c13zpnzN/YqrSMPj2t4/v6B523j9TKeVW98XM",
    "zHlZlzfH/L+zwf//yO5wUihAi4b3o9eu65HdwkbSJJojz3g5MEvgElmNFU8BbvifJYynsggQc4Q",
    "SqkduJVNDGM8n7T8MAyq6cxDWI/FoaFD3FCEh5zNvon3tToNjLWqYRANwFCHUthorjJSLTcClIP",
    "azEEfWUSZ1sp0L5JrarqyDBNUwaNcwhaXGmCVlAr9PU10VDT525Gtvoqf4IWScoo4eVtHRPlQ4r",
    "eoeEkDfH/sdFovZK8yNelnGghKWGNhNViIsti2wzlPEK3Zs521omuQ9dEEeWRta40CXDUH/2khK",
    "16NTEASSI45m68gdWucZ4t8NQvS2I1TeAJkQcB+oGC0C0G0YGVT3BpoYPTOGGBky6DlupIOsA1c",
    "s0bLRtKa+UNk4w9Sa2FzXgbA1xVtSZvM+A1wphyw3ZEeggDYBl/nWiyBnbM7wBWioYdgd5hI9cx",
    "+g2nZwZybE3wkJb3ykEBlRWJx5jwS8BpYsMAQjTzVPTpRKviyCgtkhqGfv3wsCTfPLbFNcgeCaB",
    "0vpaEKkCBM79y5A2psoGEaLSSzrQsPcM4BteCeqpneH0oZB9t7FoYXtrKRoIbGsII6qAhS3IN9O",
    "4nhJmSMicKs7J1jxOFWdaNF7E2ftYCBySIIbQ1kDI4wjyLE5RdYmXL5wFqO6B6VAcVSJqWjSSDC",
    "R2Ln7VSZ5TCCe9zMeRNKRI/72KW+BUAZg0iOZayh1hK5aiJwpeTWsJZJ8vurg2DtTPRjHStbzwC",
    "jtXQ6mqrNwkwqhoc200UaBOsT19gO8IVVRpbwFEji4Ew64P3vZd98NaCPvi9mT54Jx8He6cpDU9",
    "1bLAK8zib587MPFdL9NOTPnwZzpvDeVPc5PrxDL6ygNebuRb5rlT//n61wFfFv0rFm7PTuwWerQ",
    "JuO7fVgx93f/nztxs/fCeeffT195eur+KvzPFXst8Wl15w269cy8e3Jr3SNDMP5pKXfferFX5wo",
    "+AHdqwePT07udO9+0I9uFeP/OePpX+vL8bz/VRdLu+FgryT+7dtf4aBm+UBGbTCfN9jx8SM+3E7",
    "//0ZebdX2OPKjD/hzu/LzfC39ov4Rfa6WLCXHYsEIlLw39enx/v7RX/cWoHfwV+Df03+2/ub4cf",
    "87RXy7xbWYTfbd5wSW13gtJjJ5vPLuvHqintb+P7HlV+HMnWw+opx3ivGbaqfa31/299flo+9wv",
    "s7b7Aey/KoS5/3pun1q+fWh32Yj7+YfluoxZSFC3Y3+WPcgPQWPf2P+Pcfjva7mY8b1n75B91vP",
    "zvghI1wOzPeLua329J+S5w+kkDU4j3j66gnk+uq/dbVQnzb8ZDyUAzVJ5/u3d3bpD79BZd4Crk=",
    ""
  };

  static char newstr [1501] = "";
  newstr[0] = '\0';
  for (i = 0; i < 21; i++) {
    strcat(newstr, encStrCodegen[i]);
  }

  return newstr;
}

static void mdlSetWorkWidths_c2_pid_thuchanh(SimStruct *S)
{
  const char* newstr = sf_c2_pid_thuchanh_get_post_codegen_info();
  sf_set_work_widths(S, newstr);
  ssSetChecksum0(S,(3608785465U));
  ssSetChecksum1(S,(2954287866U));
  ssSetChecksum2(S,(559968137U));
  ssSetChecksum3(S,(319268171U));
}

static void mdlRTW_c2_pid_thuchanh(SimStruct *S)
{
  if (sim_mode_is_rtw_gen(S)) {
    ssWriteRTWStrParam(S, "StateflowChartType", "Embedded MATLAB");
  }
}

static void mdlSetupRuntimeResources_c2_pid_thuchanh(SimStruct *S)
{
  SFc2_pid_thuchanhInstanceStruct *chartInstance;
  chartInstance = (SFc2_pid_thuchanhInstanceStruct *)utMalloc(sizeof
    (SFc2_pid_thuchanhInstanceStruct));
  if (chartInstance==NULL) {
    sf_mex_error_message("Could not allocate memory for chart instance.");
  }

  memset(chartInstance, 0, sizeof(SFc2_pid_thuchanhInstanceStruct));
  chartInstance = new (chartInstance) SFc2_pid_thuchanhInstanceStruct;
  chartInstance->chartInfo.chartInstance = chartInstance;
  chartInstance->chartInfo.isEMLChart = 1;
  chartInstance->chartInfo.chartInitialized = 0;
  chartInstance->chartInfo.sFunctionGateway = sf_opaque_gateway_c2_pid_thuchanh;
  chartInstance->chartInfo.initializeChart =
    sf_opaque_initialize_c2_pid_thuchanh;
  chartInstance->chartInfo.mdlStart = sf_opaque_mdl_start_c2_pid_thuchanh;
  chartInstance->chartInfo.mdlTerminate =
    sf_opaque_mdl_terminate_c2_pid_thuchanh;
  chartInstance->chartInfo.mdlCleanupRuntimeResources =
    sf_opaque_cleanup_runtime_resources_c2_pid_thuchanh;
  chartInstance->chartInfo.enableChart = sf_opaque_enable_c2_pid_thuchanh;
  chartInstance->chartInfo.disableChart = sf_opaque_disable_c2_pid_thuchanh;
  chartInstance->chartInfo.getSimState = sf_opaque_get_sim_state_c2_pid_thuchanh;
  chartInstance->chartInfo.setSimState = sf_opaque_set_sim_state_c2_pid_thuchanh;
  chartInstance->chartInfo.getSimStateInfo =
    sf_get_sim_state_info_c2_pid_thuchanh;
  chartInstance->chartInfo.zeroCrossings = NULL;
  chartInstance->chartInfo.outputs = NULL;
  chartInstance->chartInfo.derivatives = NULL;
  chartInstance->chartInfo.mdlRTW = mdlRTW_c2_pid_thuchanh;
  chartInstance->chartInfo.mdlSetWorkWidths = mdlSetWorkWidths_c2_pid_thuchanh;
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

  mdl_setup_runtime_resources_c2_pid_thuchanh(chartInstance);
}

void c2_pid_thuchanh_method_dispatcher(SimStruct *S, int_T method, void *data)
{
  switch (method) {
   case SS_CALL_MDL_SETUP_RUNTIME_RESOURCES:
    mdlSetupRuntimeResources_c2_pid_thuchanh(S);
    break;

   case SS_CALL_MDL_SET_WORK_WIDTHS:
    mdlSetWorkWidths_c2_pid_thuchanh(S);
    break;

   case SS_CALL_MDL_PROCESS_PARAMETERS:
    mdlProcessParameters_c2_pid_thuchanh(S);
    break;

   default:
    /* Unhandled method */
    sf_mex_error_message("Stateflow Internal Error:\n"
                         "Error calling c2_pid_thuchanh_method_dispatcher.\n"
                         "Can't handle method %d.\n", method);
    break;
  }
}
