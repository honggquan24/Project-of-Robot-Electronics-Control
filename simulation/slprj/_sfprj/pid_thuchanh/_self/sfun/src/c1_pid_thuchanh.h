#ifndef __c1_pid_thuchanh_h__
#define __c1_pid_thuchanh_h__

/* Forward Declarations */
struct SFc1_pid_thuchanhInstanceStruct;

/* Type Definitions */
#ifndef typedef_c1_pid_thuchanhStackData
#define typedef_c1_pid_thuchanhStackData

typedef struct {
} c1_pid_thuchanhStackData;

#endif                                 /* typedef_c1_pid_thuchanhStackData */

#ifndef struct_SFc1_pid_thuchanhInstanceStruct
#define struct_SFc1_pid_thuchanhInstanceStruct

struct SFc1_pid_thuchanhInstanceStruct
{
  SimStruct *S;
  ChartInfoStruct chartInfo;
  uint8_T c1_is_active_c1_pid_thuchanh;
  uint8_T c1_JITStateAnimation[1];
  uint8_T c1_JITTransitionAnimation[1];
  void *c1_fEmlrtCtx;
  real_T *c1_vr;
  real_T *c1_v;
  real_T *c1_vl;
  real_T *c1_omega;
};

#endif                                 /* struct_SFc1_pid_thuchanhInstanceStruct */

/* Named Constants */

/* Variable Declarations */

/* Variable Definitions */

/* Function Declarations */
extern const mxArray *sf_c1_pid_thuchanh_get_eml_resolved_functions_info();

/* Function Definitions */
extern void sf_c1_pid_thuchanh_get_check_sum(mxArray *plhs[]);
extern void c1_pid_thuchanh_method_dispatcher(SimStruct *S, int_T method, void
  *data);

#endif
