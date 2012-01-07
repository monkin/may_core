
#include "image.h"
#include "error.h"
#include "mcl.h"
#include <IL/il.h>
#include <CL/cl.h>
#include <stdbool.h>

ERR_DEFINE(e_mcl_image_loading, "Image loading error.", e_mcl_error);

cl_mem mcl_image_create(cl_context context, str_t data) {
	mcl_image_create_bin(context, str_begin(data), str_len(data));
}
cl_mem mcl_image_create_bin(cl_context context, const void *data, size_t len) {
	cl_int code;
	ILuint ilim = 0;
	cl_mem clim = 0;
	err_try {
		ilGenImages(1, &ilim);
		ilBindImage(ilim);
		if(!ilLoadL(IL_TYPE_UNKNOWN, data, len))
			err_throw(e_mcl_image_loading);
		ilEnable(IL_ORIGIN_SET);
		ilEnable(IL_FORMAT_SET);
		bool need_convertion = false;
		cl_image_format clformat;
		ILint ilformat = ilGetInteger(IL_IMAGE_FORMAT);
		ILint iltype = ilGetInteger(IL_IMAGE_TYPE);
		cl_channel_order *chanel_order = &clformat.image_channel_order;
		cl_channel_type *chanel_type = &clformat.image_channel_data_type;
		switch(ilformat) {
			case IL_ALPHA:
				*chanel_order = CL_A;
				break;
			case IL_RGBA:
				*chanel_order = CL_RGBA;
				break;
			case IL_BGRA:
				*chanel_order = CL_BGRA;
				break;
			case IL_LUMINANCE:
				*chanel_order = CL_LUMINANCE;
				break;
			default:
				need_convertion = true;
				*chanel_order = CL_RGBA;
				ilformat = IL_RGBA;
				break;
		}
		switch(iltype) {
			case IL_BYTE:
				*chanel_type = CL_SNORM_INT8;
				break;
			case IL_UNSIGNED_BYTE:
				*chanel_type = CL_UNORM_INT8;
				break;
			case IL_SHORT:
				*chanel_type = CL_SNORM_INT16;
				break;
			case IL_UNSIGNED_SHORT:
				*chanel_type = CL_UNORM_INT16;
				break;
			case IL_FLOAT:
				*chanel_type = CL_FLOAT;
				break;
			default:
				need_convertion = true;
				*chanel_type = CL_FLOAT;
				iltype = IL_FLOAT;
				break;
		}
		if(need_convertion) {
			if(!ilConvertImage(ilformat, iltype))
				err_throw(e_mcl_image_loading);
		}
		clim = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &clformat,
				ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetData(), &code);
		mcl_throw_if_error(code);
		ilDeleteImages(1, &ilim);
		return clim;
	} err_catch {
		if(ilim)
			ilDeleteImages(1, &ilim);
		if(clim)
			clReleaseMemObject(clim);
		err_throw_down();
	}
}
cl_mem mcl_image_delete(cl_mem img) {
	if(img)
		clReleaseMemObject(img);
	return 0;
}