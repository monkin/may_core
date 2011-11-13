
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
		return cast_to[0] = args[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_numeric(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	err_try {
		return cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
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
	cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	mcl_rule(mclt_is_vector(args[0]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[0])));
	mcl_rule(mclt_is_vector(args[1]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[1])));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compare(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(!type_compatible(args[0], args[1]) || mclt_is_image(args[0]) || (mclt_is_pointer(args[0]) && args[0]!=args[1]))
		err_throw(e_mcl_ex_invalid_operand);
	cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	if(mclt_is_vector(args[0]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[0]));
	if(mclt_is_vector(args[1]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[1]));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compose(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(type_is_arithmetic(args[0]) && type_is_arithmetic(args[1])) {
		mclt_t t = cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
		return mclt_is_vector(t) ? mclt_vector(MCLT_BOOL, mclt_vector_size(t)) : MCLT_BOOL;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_binary(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if((mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
			&& (mclt_is_integer(args[1]) || mclt_is_vector_of_integer(args[1]))) {
		return cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
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
	cast_to[0] = vector_size ? mclt_vector(MCLT_BOOL, vector_size) : MCLT_BOOL;
	return cast_to[1] = cast_to[2] = type_max(args[1], args[2]);
}

static mclt_t ret_type_op_index(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(mclt_is_pointer(args[0]) ? mclt_pointer_to(args[0])!=MCLT_VOID && mclt_is_integer(args[1]) : false)
		return mclt_pointer_to(args[0]);
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_work_item(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(type_compatible(MCLT_UINT, args[0])) {
		cast_to[0] = MCLT_UINT;
		return MCLT_ULONG;
	}
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
	if((mclt_is_integer(ret) || mclt_is_vector_of_integer(ret)) && !(ret & MCLT_UNSIGNED)) {
		cast_to[0] = cast_to[1] = ret;
		return ret | MCLT_UNSIGNED;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_integer_same(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc>=1);
	int i;
	mclt_t ret = args[0];
	for(i=1; i<argc; i++)
		ret = type_max(ret, args[i]);
	if(mclt_is_integer(ret) || mclt_is_vector_of_integer(ret)) {
		for(i=0; i<argc; i++)
			cast_to[i] = ret;
		return ret;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_float_same(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc>=1);
	int i;
	mclt_t ret = args[0];
	for(i=1; i<argc; i++)
		ret = type_max(ret, args[i]);
	if(mclt_is_float(ret) || mclt_is_vector_of_float(ret)) {
		for(i=0; i<argc; i++)
			cast_to[i] = ret;
		return ret;
	}
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
	
	{"clamp", 3, ret_type_float_same},
	{"degrees", 1, ret_type_float_same},
	{"mix", 3, ret_type_float_same},
	{"radians", 1, ret_type_float_same},
	{"step", 2, ret_type_float_same},
	{"smoothstep", 3, ret_type_float_same},
	{"sign", 1, ret_type_float_same},
	
	{"acos", 1, ret_type_float_same},
	{"acosh", 1, ret_type_float_same},
	{"acospi", 1, ret_type_float_same},
	{"asin", 1, ret_type_float_same},
	{"asinh", 1, ret_type_float_same},
	{"asinpi", 1, ret_type_float_same},
	{"atan", 1, ret_type_float_same},
	{"atan2", 2, ret_type_float_same},
	{"atanh", 1, ret_type_float_same},
	{"atanpi", 1, ret_type_float_same},
	{"atan2pi", 2, ret_type_float_same},
	{"cbrt", 1, ret_type_float_same},
	{"ceil", 1, ret_type_float_same},
	{"copysign", 2, ret_type_float_same},
	{"cos", 1, ret_type_float_same},
	{"cosh", 1, ret_type_float_same},
	{"cospi", 1, ret_type_float_same},
	{"erfc", 1, ret_type_float_same},
	{"erf", 1, ret_type_float_same},
	{"exp", 1, ret_type_float_same},
	{"exp2", 1, ret_type_float_same},
	{"exp10", 1, ret_type_float_same},
	
	{"expm1", 1, ret_type_float_same},
	{"fabs", 1, ret_type_float_same},
	{"fdim", 2, ret_type_float_same},
	{"floor", 1, ret_type_float_same},
	{"fma", 3, ret_type_float_same},
	{"fmax", 2, ret_type_float_same},
	{"fmin", 2, ret_type_float_same},
	{"fmod", 2, ret_type_float_same},
	{"fract", 2, 0},
	{"frexp", 2, 0},
	{"hypot", 2, ret_type_float_same},
	{"ilogb", 1, 0},
	{"ldexp", 2, 0},
	{"ldexp", 2, 0},
	{"lgamma", 1, ret_type_float_same},
	{"lgamma_r", 2, 0},
	{"log", 1, ret_type_float_same},
	{"log2", 1, ret_type_float_same},
	{"log10", 1, ret_type_float_same},
	{"log1p", 1, ret_type_float_same},
	{"logb", 1, ret_type_float_same},
	{"mad", 3, ret_type_float_same},
	{"maxmag", 2, ret_type_float_same},
	{"minmag", 2, ret_type_float_same},
	{"modf", 2, 0},
	{"nan", 1, 0},
	{"nextafter", 2, ret_type_float_same},
	{"pow", 2, ret_type_float_same},
	{"pown", 2, 0},
	{"powr", 2, ret_type_float_same},
	{"remainder", 2, ret_type_float_same},
	{"remquo", 3, 0},
	{"rint", 1, ret_type_float_same},
	{"rootn", 2, 0},
	{"round", 1, ret_type_float_same},
	{"rsqrt", 1, ret_type_float_same},
	{"sin", 1, ret_type_float_same},
	{"sincos", 2, 0},
	{"sinh", 1, ret_type_float_same},
	{"sinpi", 1, ret_type_float_same},
	{"sqrt", 1, ret_type_float_same},
	{"tan", 1, ret_type_float_same},
	{"tanh", 1, ret_type_float_same},
	{"tanpi", 1, ret_type_float_same},
	{"tgamma", 1, ret_type_float_same},
	{"trunc", 1, ret_type_float_same},
	
	{"dot", 2, ret_type_float_same},
	{"cross", 2, 0},
	{"distance", 2, 0},
	{"length", 1, 0},
	{"normalize", 1, ret_type_float_same},
	{"fast_distance", 2, 0},
	{"fast_length", 1, 0},
	{"fast_normalize", 1, ret_type_float_same},
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
	{"shufle2", 3, 0},
	
	{"read_imagef", 3, 0},
	{"read_imagei", 3, 0},
	{"read_imageui", 3, 0},
	{"write_imagef", 3, 0},
	{"write_imagei", 3, 0},
	{"write_imageui", 3, 0},
	{"get_image_width", 3, 0},
	{"get_image_height", 3, 0},
	
};

static mcl_ex_t mcl_call_internal(heap_t h, mcl_stdfn_t fn, mcl_ex_t *args) {

}

mcl_ex_t mcl_call(heap_t h, str_t nm, ...) {

}

mcl_ex_t mcl_call_cs(heap_t h, const char *nm, ...) {

}
