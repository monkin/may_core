
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
	mclt_t (*return_type)(size_t, const mclt_t *, mclt_t *);
} mcl_stdfn_s;

typedef mcl_stdfn_s *mcl_stdfn_t;

#define mcl_rule(cnd, result) if(cnd) return (result)
#define mcl_rulex(precnd, cnd, result, exception) if(precnd) { if(cnd) { return (result); } else { err_throw(exception); }; }
#define mcl_else(result) return result

/**
 * t2 can be lossless converted to t1
 */
static bool type_greater(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rule(mclt_is_vector(t1) && mclt_is_vector(t2), mclt_vector_size(t1)==mclt_vector_size(t2) && type_greater(mclt_vector_of(t1), mclt_vector_of(t2)));
	mcl_rule(mclt_is_float(t1), mclt_is_numeric(t2));
	mcl_rule(mclt_is_integer(t1), mclt_integer_size(t1)>=mclt_integer_size(t2));
	mcl_rule(mclt_is_vector(t1), type_greater(mclt_vector_of(t1), t2));
	mcl_rule(mclt_is_pointer(t1) && mclt_is_pointer(t2), (mclt_pointer_to(t1)==mclt_pointer_to(t2) || mclt_is_void(mclt_pointer_to(t1))) && mclt_pointer_type(t1)==mclt_pointer_type(t1));
	mcl_rule(mclt_is_bool(t1), mclt_is_numeric(t2) || mclt_is_pointer(t2));
	mcl_else(false);
}

/**
 * t1 can contains some values from t2
 * t2 can be converted to t1
 */
static bool type_compatible(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rule(mclt_is_vector(t1) && mclt_is_vector(t2), mclt_vector_size(t1)==mclt_vector_size(t2));
	mcl_rule(mclt_is_pointer(t1) && mclt_is_pointer(t2), mclt_pointer_type(t1)==mclt_pointer_type(t1));
	mcl_rule(mclt_is_numeric(t1) && mclt_is_numeric(t2), true);
	mcl_rule(mclt_is_bool(t1) && mclt_is_pointer(t2), true);
	mcl_else(false);
}

/**
 * For numerics and vectors only. Returns the greater of both types.
 */
static mclt_t type_max(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rulex(mclt_is_vector(t1) && mclt_is_vector(t2),
		mclt_vector_size(t1)==mclt_vector_size(t2),
		mclt_vector(type_max(mclt_vector_of(t1), mclt_vector_of(t2)), mclt_vector_size(t1)),
		e_mcl_ex_invalid_operand);
	mcl_rule(mclt_is_vector(t1) && mclt_is_numeric(t2),
		mclt_vector(type_max(mclt_vector_of(t1), t2), mclt_vector_size(t1)));
	mcl_rule(mclt_is_numeric(t1) && mclt_is_vector(t2), type_max(t2, t1));
	mcl_rule(mclt_is_float(t1) || mclt_is_float(t2), MCLT_FLOAT);
	mcl_rule((mclt_is_integer(t1) && mclt_is_integer(t2)) ? mclt_integer_size(t1)==mclt_integer_size(t2) : false, t1 | MCLT_UNSIGNED);
	mcl_rule(mclt_is_integer(t1) && mclt_is_integer(t2), mclt_integer_size(t1)>mclt_integer_size(t2) ? t1 : t2);
	err_throw(e_mcl_ex_invalid_operand);
}

#define type_is_arithmetic(t) (mclt_is_numeric(t) || mclt_is_vector(t))

static mclt_t ret_type_op_set(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(type_compatible(args[0], args[1]))
		return args[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_numeric(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	err_try {
		return type_max(args[0], args[1]);
	} err_catch {
		if(err_is(e_mclt_error))
			err_replace(e_mcl_ex_invalid_operand);
		err_throw_down();
	}
}

static mclt_t ret_type_op_plus(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if((mcl_is_pointer(args[0]) && mcl_is_integer(args[1])) ? !mclt_is_void(mclt_pointer_to(args[0])) : false)
		return args[0];
	if((mcl_is_pointer(args[1]) && mcl_is_integer(args[0])) ? !mclt_is_void(mclt_pointer_to(args[1])) : false)
		return args[1];
	return ret_type_op_numeric(argc, args, cast_to);
}

static mclt_t ret_type_op_minus(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(mcl_is_pointer(args[0])) {
		if(mclt_is_void(mclt_pointer_to(args[0])))
			err_throw(e_mcl_ex_invalid_operand);
		if(args[0]==args[1])
			return MCLT_LONG;
		if((mcl_is_pointer(args[0]) && mcl_is_integer(args[1])) ? !mclt_is_void(mclt_pointer_to(args[0])) : false)
			return args[0];
		err_throw(e_mcl_ex_invalid_operand);
	}
	return ret_type_op_numeric(argc, args, cast_to);
}

static mclt_t ret_type_op_not(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	mcl_rule(mclt_is_vector(args[0]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[0])));
	mcl_rule(mclt_is_pointer(args[0]) || mclt_is_numeric(args[0]), MCLT_BOOL);
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_equal(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(!type_compatible(args[0], args[1]))
		err_throw(e_mcl_ex_invalid_operand);
	mcl_rule(mclt_is_vector(args[0]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[0])));
	mcl_rule(mclt_is_vector(args[1]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[1])));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compare(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(!type_compatible(args[0], args[1]) || mclt_is_image(args[0]) || (mclt_is_pointer(args[0]) && args[0]!=args[1]))
		err_throw(e_mcl_ex_invalid_operand);
	if(mclt_is_vector(args[0]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[0]));
	if(mclt_is_vector(args[1]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[1]));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compose(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(type_is_arithmetic(args[0]) && type_is_arithmetic(args[1])) {
		mclt_t t = type_max(args[0], args[1]);
		return mclt_is_vector(t) ? mclt_vector(MCLT_BOOL, mclt_vector_size(t)) : MCLT_BOOL;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_binary(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if((mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
			&& (mclt_is_integer(args[1]) || mclt_is_vector_of_integer(args[1]))) {
		return type_max(args[0], args[1]);
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_invert(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
		return args[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_ternary(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==3);
	int i;
	int vector_size = 0;
	for(i=0; i<3; i++) {
		if(mclt_is_vector(args[i])) {
			if(!vector_size)
				vector_size = mclt_vector_size(args[i]);
			else {
				if(vector_size!=mclt_vector_size(args[i]))
					err_throw(e_mcl_ex_invalid_operand);
			}
		} else if(!mclt_is_numeric(args[0]))
			err_throw(e_mcl_ex_invalid_operand);
	}
	if(vector_size && mclt_is_vector_of_float(args[0]))
		err_throw(e_mcl_ex_invalid_operand);
	return type_max(args[1], args[2]);
}

static mclt_t ret_type_op_index(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(mclt_is_pointer(args[0]) ? mclt_pointer_to(args[0])!=MCLT_VOID && mclt_is_integer(args[1]) : false)
		return mclt_pointer_to(args[0]);
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_work_item(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(type_compatible(MCLT_UINT, args[0]))
		return MCLT_ULONG;
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_work_dim(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==0);
	return MCLT_ULONG;
}

static mclt_t ret_type_abs(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
		return args[0] | MCLT_UNSIGNED;
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_abs_diff(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	mclt_t ret = type_max(args[0], args[1]);
	if(mclt_is_integer(ret) || mclt_is_vector_of_integer(ret))
		return ret | MCLT_UNSIGNED;
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_integer_same(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc>=1);
	int i;
	mclt_t ret = args[0];
	for(i=1; i<argc; i++)
		ret = type_max(ret, args[i]);
	if(mclt_is_integer(ret) || mclt_is_vector_of_integer(ret))
		return ret;
	err_throw(e_mcl_ex_invalid_operand);
}


static mcl_stdfn_s stdfn_list[] = {
	{"=", 2, ret_type_op_set},
	{"+", 2, ret_type_op_plus},
	{"-", 2, ret_type_op_minus},
	{"*", 2, ret_type_op_numeric},
	{"/", 2, ret_type_op_numeric},
	{"%", 2, ret_type_op_numeric},
	
	{"==", 2, ret_type_op_equal},
	{"!=", 2, ret_type_op_equal},
	{"<", 2, ret_type_op_compare},
	{">", 2, ret_type_op_compare},
	{"<=", 2, ret_type_op_compare},
	{">=", 2, ret_type_op_compare},
	{"!", 1, ret_type_op_not},
	{"&&", 2, ret_type_op_compose},
	{"||", 2, ret_type_op_compose},
	
	{"&", 2, ret_type_op_binary},
	{"|", 2, ret_type_op_binary},
	{"^", 2, ret_type_op_binary},
	{"<<", 2, ret_type_op_binary},
	{">>", 2, ret_type_op_binary},
	{">>", 2, ret_type_op_binary},
	{"~", 1, ret_type_op_invert},
	
	{"?", 3, ret_type_op_ternary},
	{"[]", 2, ret_type_op_index},
	
	{"get_global_id", 1, ret_type_work_item},
	{"get_global_offset", 1, ret_type_work_item},
	{"get_global_size", 1, ret_type_work_item},
	{"get_group_id", 1, ret_type_work_item},
	{"get_local_id", 1, ret_type_work_item},
	{"get_local_size", 1, ret_type_work_item},
	{"get_num_groups", 1, ret_type_work_item},
	{"get_work_dim", 0, ret_type_work_dim},
	
	{"abs", 1, ret_type_abs},
	{"abs_diff", 2, ret_type_abs_diff},
	{"add_sat", 2, ret_type_integer_same},
	{"hadd", 2, ret_type_integer_same},
	{"rhadd", 2, ret_type_integer_same},
	{"clz", 1, ret_type_integer_same},
	{"clamp", 3, ret_type_integer_same},
	{"mad_hi", 3, ret_type_integer_same},
	{"mad_sat", 3, ret_type_integer_same},
	{"max", 2, ret_type_integer_same},
	{"min", 2, ret_type_integer_same},
	{"mul_hi", 2, ret_type_integer_same},
	{"rotate", 2, ret_type_integer_same},
	{"sub_sat", 2, ret_type_integer_same},
	{"upsample", 2, 0},
	
	{"mad24", 3, ret_type_integer_same},
	{"mul24", 2, ret_type_integer_same},
	
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
