
#include <maylib/json.h>
#include <maylib/stream.h>

int main() {
	jbuilder_t jb = jbuilder_create_s(ios_std_out(), JSON_FORMAT_SPACE_4);
	jbuilder_object(jb);
		jbuilder_key_cs(jb, "12");
		jbuilder_string_cs(jb, "test\n\ttest2");
		jbuilder_key_cs(jb, "tre");
		jbuilder_string_cs(jb, "test435");
		jbuilder_key_cs(jb, "qwe");
		jbuilder_number(jb, 10000000898998888.4);
		jbuilder_key_cs(jb, "iut");
		jbuilder_number_i(jb, 100000009993492L);
	jbuilder_object_end(jb);
	jbuilder_delete(jb);	
	return 0;
}

