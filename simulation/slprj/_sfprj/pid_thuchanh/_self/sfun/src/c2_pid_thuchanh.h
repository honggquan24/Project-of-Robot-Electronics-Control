#ifndef __c2_pid_thuchanh_h__
#define __c2_pid_thuchanh_h__

/* Forward Declarations */
struct SFc2_pid_thuchanhInstanceStruct;

/* Type Definitions */
#ifndef typedef_c2_pid_thuchanhStackData
#define typedef_c2_pid_thuchanhStackData

typedef struct {
} c2_pid_thuchanhStackData;

#endif                                 /* typedef_c2_pid_thuchanhStackData */

#ifndef struct_SFc2_pid_thuchanhInstanceStruct
#define struct_SFc2_pid_thuchanhInstanceStruct

struct SFc2_pid_thuchanhInstanceStruct
{
  SimStruct *S;
  ChartInfoStruct chartInfo;
  uint8_T c2_is_active_c2_pid_thuchanh;
  uint8_T c2_JITStateAnimation[1];
  uint8_T c2_JITTransitionAnimation[1];
  void *c2_fEmlrtCtx;
  real_T *c2_vr;
  real_T *c2_v;
  real_T *c2_vl;
  real_T *c2_omega;
};

#endif                                 /* struct_SFc2_pid_thuchanhInstanceStruct */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
extern const mxArray *sf_c2_pid_thuchanh_get_eml_resolved_functions_info();

/* Function Definitions */
extern void sf_c2_pid_thuchanh_get_check_sum(mxArray *plhs[]);
extern void c2_pid_thuchanh_method_dispatcher(SimStruct *S, int_T method, void
  *data);

#endif
