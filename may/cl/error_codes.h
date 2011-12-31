#ifndef MCL_OPENCL_ERROR
#	error This file should be included from cl/error.c or cl/error.h
#endif

MCL_OPENCL_ERROR(success, SUCCESS);
MCL_OPENCL_ERROR(device_not_found, DEVICE_NOT_FOUND);
MCL_OPENCL_ERROR(device_not_available, DEVICE_NOT_AVAILABLE);
MCL_OPENCL_ERROR(compiler_not_available, COMPILER_NOT_AVAILABLE);
MCL_OPENCL_ERROR(mem_object_allocation_failure, MEM_OBJECT_ALLOCATION_FAILURE);
MCL_OPENCL_ERROR(out_of_resources, OUT_OF_RESOURCES);
MCL_OPENCL_ERROR(out_of_host_memory, OUT_OF_HOST_MEMORY);
MCL_OPENCL_ERROR(profiling_info_not_available, PROFILING_INFO_NOT_AVAILABLE);
MCL_OPENCL_ERROR(mem_copy_overlap, MEM_COPY_OVERLAP);
MCL_OPENCL_ERROR(image_format_mismatch, IMAGE_FORMAT_MISMATCH);
MCL_OPENCL_ERROR(image_format_not_supported, IMAGE_FORMAT_NOT_SUPPORTED);
MCL_OPENCL_ERROR(build_program_failure, BUILD_PROGRAM_FAILURE);
MCL_OPENCL_ERROR(map_failure, MAP_FAILURE);
MCL_OPENCL_ERROR(misaligned_sub_buffer_offset, MISALIGNED_SUB_BUFFER_OFFSET);
MCL_OPENCL_ERROR(exec_status_error_for_events_in_wait_list, EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);

MCL_OPENCL_ERROR(invalid_value, INVALID_VALUE);
MCL_OPENCL_ERROR(invalid_device_type, INVALID_DEVICE_TYPE);
MCL_OPENCL_ERROR(invalid_platform, INVALID_PLATFORM);
MCL_OPENCL_ERROR(invalid_device, INVALID_DEVICE);
MCL_OPENCL_ERROR(invalid_context, INVALID_CONTEXT);
MCL_OPENCL_ERROR(invalid_queue_properties, INVALID_QUEUE_PROPERTIES);
MCL_OPENCL_ERROR(invalid_command_queue, INVALID_COMMAND_QUEUE);
MCL_OPENCL_ERROR(invalid_host_ptr, INVALID_HOST_PTR);
MCL_OPENCL_ERROR(invalid_mem_object, INVALID_MEM_OBJECT);
MCL_OPENCL_ERROR(invalid_image_format_descriptor, INVALID_IMAGE_FORMAT_DESCRIPTOR);
MCL_OPENCL_ERROR(invalid_image_size, INVALID_IMAGE_SIZE);
MCL_OPENCL_ERROR(invalid_sampler, INVALID_SAMPLER);
MCL_OPENCL_ERROR(invalid_binary, INVALID_BINARY);
MCL_OPENCL_ERROR(invalid_build_options, INVALID_BUILD_OPTIONS);
MCL_OPENCL_ERROR(invalid_program, INVALID_PROGRAM);
MCL_OPENCL_ERROR(invalid_program_executable, INVALID_PROGRAM_EXECUTABLE);
MCL_OPENCL_ERROR(invalid_kernel_name, INVALID_KERNEL_NAME);
MCL_OPENCL_ERROR(invalid_kernel_definition, INVALID_KERNEL_DEFINITION);
MCL_OPENCL_ERROR(invalid_kernel, INVALID_KERNEL);
MCL_OPENCL_ERROR(invalid_arg_index, INVALID_ARG_INDEX);
MCL_OPENCL_ERROR(invalid_arg_value, INVALID_ARG_VALUE);
MCL_OPENCL_ERROR(invalid_arg_size, INVALID_ARG_SIZE);
MCL_OPENCL_ERROR(invalid_kernel_args, INVALID_KERNEL_ARGS);
MCL_OPENCL_ERROR(invalid_work_dimension, INVALID_WORK_DIMENSION);
MCL_OPENCL_ERROR(invalid_work_group_size, INVALID_WORK_GROUP_SIZE);
MCL_OPENCL_ERROR(invalid_work_item_size, INVALID_WORK_ITEM_SIZE);
MCL_OPENCL_ERROR(invalid_global_offset, INVALID_GLOBAL_OFFSET);
MCL_OPENCL_ERROR(invalid_event_wait_list, INVALID_EVENT_WAIT_LIST);
MCL_OPENCL_ERROR(invalid_event, INVALID_EVENT);
MCL_OPENCL_ERROR(invalid_operation, INVALID_OPERATION);
MCL_OPENCL_ERROR(invalid_gl_object, INVALID_GL_OBJECT);
MCL_OPENCL_ERROR(invalid_buffer_size, INVALID_BUFFER_SIZE);
MCL_OPENCL_ERROR(invalid_mip_level, INVALID_MIP_LEVEL);
MCL_OPENCL_ERROR(invalid_global_work_size, INVALID_GLOBAL_WORK_SIZE);
MCL_OPENCL_ERROR(invalid_property, INVALID_PROPERTY);
