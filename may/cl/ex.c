
#include "ex.h"
#include <assert.h>

ERR_DEFINE(e_mcl_ex_invalid_operand, "Invalid operand type", e_mcl_error);

bool mcl_insert_ptr(map_t m, void *p) {
	char phash[sizeof(void *)];
	int i;
	for(i=0; i<sizeof(void *); i++)
		phash[i] = ((char *)(&p))[sizeof(void *) - i];
	if(map_get_bin(m, phash, sizeof(phash)))
		return false;
	else {
		map_set_bin(m, phash, sizeof(phash), p);
		return true;
	}
}

typedef struct {
	char *name;
	int args_count;
	mclt_t (*return_type)(size_t, mclt_t *);
} mcl_stdfn_s;

typedef mcl_stdfn_s *mcl_stdfn_t;

static mclt_t ret_type_op_set(size_t argc, mclt_t *argt) {
	assert(argc==2);
	if(mclt_is_compatible(argt[0], argt[1]))
		return argt[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_binary(size_t argc, mclt_t *argt) {
	assert(argc==2);
	err_try {
		return mclt_op_result(argt[0], argt[1]);
	} err_catch {
		if(err_is(e_mclt_error))
			err_replace(e_mcl_ex_invalid_operand);
		err_throw_down();
	}
}

static mcl_stdfn_s stdfn_list[] = {
	{"=", 2, ret_type_op_set},
	{"+", 2, ret_type_op_binary},
	{"-", 2, ret_type_op_binary},
	{"*", 2, ret_type_op_binary},
	{"/", 2, ret_type_op_binary},
	{"%", 2, ret_type_op_binary},
	
	{"!", 1, 0},
	{"==", 2, 0},
	{"!=", 2, 0},
	{"<", 2, 0},
	{">", 2, 0},
	{"<=", 2, 0},
	{">=", 2, 0},
	{"&&", 2, 0},
	{"||", 2, 0},
	
	{"&", 2, 0},
	{"|", 2, 0},
	{"^", 2, 0},
	{"~", 2, 0},
	{"<<", 2, 0},
	{">>", 2, 0},
	
	{"?", 3, 0},
	{"[]", 2, 0},
	
	{"get_global_id", 1, 0},
	{"get_global_offset", 1, 0},
	{"get_global_size", 1, 0},
	{"get_group_id", 1, 0},
	{"get_local_id", 1, 0},
	{"get_local_size", 1, 0},
	{"get_num_groups", 1, 0},
	{"get_work_dim", 0, 0},
	
	{"abs", 1, 0},
	{"abs_diff", 2, 0},
	{"add_sat", 2, 0},
	{"hadd", 2, 0},
	{"rhadd", 2, 0},
	{"clz", 1, 0},
	{"clamp", 3, 0},
	{"mad_hi", 3, 0},
	{"mad_sat", 3, 0},
	{"max", 2, 0},
	{"min", 2, 0},
	{"mul_hi", 2, 0},
	{"rotate", 2, 0},
	{"sub_sat", 2, 0},
	{"upsample", 2, 0},
	
	{"mad24", 3, 0},
	{"mul24", 2, 0},
	
	{"degrees", 1, 0},
	{"mix", 3, 0},
	{"radians", 1, 0},
	{"step", 2, 0},
	{"smoothstep", 3, 0},
	{"sign", 1, 0},
	
	{"acos", 1, 0},
	{"acosh", 1, 0},
	{"acospi", 1, 0},
	{"asin", 1, 0},
	{"asinh", 1, 0},
	{"asinpi", 1, 0},
	{"atan", 1, 0},
	{"atan2", 2, 0},
	{"atanh", 1, 0},
	{"atanpi", 1, 0},
	{"atan2pi", 2, 0},
	{"cbrt", 1, 0},
	{"ceil", 1, 0},
	{"copysign", 2, 0},
	{"cos", 1, 0},
	{"cosh", 1, 0},
	{"cospi", 1, 0},
	{"erfc", 1, 0},
	{"erf", 1, 0},
	{"exp", 1, 0},
	{"exp2", 1, 0},
	{"exp10", 1, 0},
	
	{"expm1", 1, 0},
	{"fabs", 1, 0},
	{"fdim", 2, 0},
	{"floor", 1, 0},
	{"fma", 3, 0},
	{"fmax", 2, 0},
	{"fmin", 2, 0},
	{"fmod", 2, 0},
	{"fract", 2, 0},
	{"frexp", 2, 0},
	{"hypot", 2, 0},
	{"ilogb", 1, 0},
	{"ldexp", 2, 0},
	{"ldexp", 2, 0},
	{"lgamma", 1, 0},
	{"lgamma_r", 2, 0},
	{"log", 1, 0},
	{"log2", 1, 0},
	{"log10", 1, 0},
	{"log1p", 1, 0},
	{"logb", 1, 0},
	{"mad", 3, 0},
	{"maxmag", 2, 0},
	{"minmag", 2, 0},
	{"modf", 2, 0},
	{"nan", 1, 0},
	{"nextafter", 2, 0},
	{"pow", 2, 0},
	{"pown", 2, 0},
	{"powr", 2, 0},
	{"remainder", 2, 0},
	{"remquo", 3, 0},
	{"rint", 1, 0},
	{"rootn", 2, 0},
	{"round", 1, 0},
	{"rsqrt", 1, 0},
	{"sin", 1, 0},
	{"sincos", 2, 0},
	{"sinh", 1, 0},
	{"sinpi", 1, 0},
	{"sqrt", 1, 0},
	{"tan", 1, 0},
	{"tanh", 1, 0},
	{"tanpi", 1, 0},
	{"tgamma", 1, 0},
	{"trunc", 1, 0},
	
	{"dot", 2, 0},
	{"cross", 2, 0},
	{"distance", 2, 0},
	{"length", 1, 0},
	{"normalize", 1, 0},
	{"fast_distance", 2, 0},
	{"fast_length", 1, 0},
	{"fast_normalize", 1, 0},
	{"isequal", 2, 0},
	{"isnotequal", 2, 0},
	{"isgreater", 2, 0},
	{"isgreaterequal", 2, 0},
	{"isless", 2, 0},
	{"islessequal", 2, 0},
	{"islessgreater", 2, 0},
	{"isfinite", 1, 0},
	{"isinf", 1, 0},
	{"isnan", 1, 0},
	{"isnormal", 1, 0},
	{"isordered", 2, 0},
	{"isunordered", 2, 0},
	{"signbit", 1, 0},
	{"any", 1, 0},
	{"all", 1, 0},
	{"bitselect", 3, 0},
	{"select", 3, 0},
	{"shufle", 2, 0},
	{"shufle2", 3, 0}
};

static mcl_ex_t mcl_call_internal(heap_t h, mcl_stdfn_t fn, mcl_ex_t *args) {

}

mcl_ex_t mcl_call(heap_t h, str_t nm, ...) {

}

mcl_ex_t mcl_call_cs(heap_t h, const char *nm, ...) {

}
