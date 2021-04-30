//#include "gimple-walk.h"
//#include "intl.h"
//#include "tree-ssa-alias.h"
//#include "basic-block.h"
//#include "gimple-expr.h"
//#include "function.h"


#include <iostream>
#include <string>

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "context.h"
#include "tree.h"
#include "tree-pass.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"

#define RESET   "\033[0m"
#define RED		"\033[31m"
#define GREEN   "\033[32m" 


int plugin_is_GPL_compatible;


static inline tree build_const_char_string(int len, const char *str)
{
	tree cstr, elem, index, type;

	cstr = build_string(len, str);
	elem = build_type_variant(char_type_node, 1, 0);
	index = build_index_type(size_int(len - 1));
	type = build_array_type(elem, index);
	TREE_TYPE(cstr) = type;
	TREE_CONSTANT(cstr) = 1;
	TREE_READONLY(cstr) = 1;
	TREE_STATIC(cstr) = 1;
	return cstr;
}


static vec<tree, va_gc> * create_clobbers(std::string clobbers_str){
	std::string delimiter = ",";
	std::string token;
	int pos = 0;
	tree clob;
	vec<tree, va_gc> *clobbers = NULL;


	while ((pos = clobbers_str.find(delimiter)) != std::string::npos) {
		token = clobbers_str.substr(0, pos);
		//std::cout << token.length() << std::endl;
		clob = build_tree_list(NULL_TREE, build_const_char_string(token.length()+1, token.c_str()));
		vec_safe_push(clobbers, clob);
		clobbers_str.erase(0, pos + delimiter.length());
	}

	clob = build_tree_list(NULL_TREE, build_const_char_string(clobbers_str.length()+1, clobbers_str.c_str()));
	vec_safe_push(clobbers, clob);

	return clobbers;
}

static tree create_pointer(const char *str){
	tree var;
	var = create_tmp_var(ptr_type_node, str);
	mark_addressable(var);
	SET_SSA_NAME_VAR_OR_IDENTIFIER(ptr_type_node, var);

	return var;
}

static void insert_asm_instr_before(const char *str, vec<tree, va_gc> *inputs, vec<tree, va_gc> *outputs, 
											vec<tree, va_gc> *clobbers, vec<tree, va_gc> *labels, bool _volatile, gimple_stmt_iterator * gsi){

	gimple *asm_instr = gimple_build_asm_vec(str, inputs, outputs, clobbers, labels);
	gimple_asm_set_volatile((gasm *)asm_instr, _volatile);
	//print_gimple_stmt(stderr, asm_instr, TDF_NONE);
	gsi_insert_before(gsi, asm_instr, GSI_NEW_STMT);

}

static void insert_asm_instr_after(const char *str, vec<tree, va_gc> *inputs, vec<tree, va_gc> *outputs, 
											vec<tree, va_gc> *clobbers, vec<tree, va_gc> *labels, bool _volatile, gimple_stmt_iterator * gsi){

	gimple *asm_instr = gimple_build_asm_vec(str, inputs, outputs, clobbers, labels);
	gimple_asm_set_volatile((gasm *)asm_instr, _volatile);
	//print_gimple_stmt(stderr, asm_instr, TDF_NONE);
	gsi_insert_after(gsi, asm_instr, GSI_NEW_STMT);

}

static void instrument_entry(function *fun, tree var){

	basic_block bb; 
	gimple_stmt_iterator gsi_entry_bb;
	tree output;
	vec<tree, va_gc> *outputs = NULL;
	vec<tree, va_gc> *clobbers = NULL;

	bb = single_succ(ENTRY_BLOCK_PTR_FOR_FN(fun));
	gsi_entry_bb = gsi_start_bb(bb);
	
	clobbers = create_clobbers("r8,r9,ecx,edx,esi,edi,rax");

	const char *asm_instr = "xor %%r9, %%r9\n\t"
							"mov $0xffffffff, %%r8d\n\t"
							"xor %%ecx, %%ecx\n\t"
							"mov $0x22, %%cl\n\t"
							"xor %%edx, %%edx\n\t"
							"mov $0x3, %%dl\n\t"
							"mov $0x8, %%esi\n\t"
							"xor %%edi, %%edi\n\t"
							"call mmap@plt\n\t";
	
	insert_asm_instr_before(asm_instr, NULL, NULL, clobbers, NULL, true, &gsi_entry_bb);

	output = build_tree_list(NULL_TREE, build_const_char_string(3, "=r")); 
	output = chainon(NULL_TREE, build_tree_list(output, var));
	vec_safe_push(outputs, output);

	clobbers = create_clobbers("memory");

	asm_instr = "mov %%rax, %0";
	insert_asm_instr_after(asm_instr,NULL,outputs,clobbers,NULL,true,&gsi_entry_bb);


	clobbers = create_clobbers("rcx,memory");
	asm_instr = "mov 0x8(%%rbp), %%rcx\n\t"
                "mov %%rcx, (%%rax)\n\t";
	insert_asm_instr_after(asm_instr,NULL,NULL,clobbers,NULL,true,&gsi_entry_bb);

	clobbers = create_clobbers("edx,rax,rdi");
	asm_instr = "mov $0x1, %%dl\n\t"
			    "mov %%rax, %%rdi\n\t"
			    "call mprotect@plt\n\t";
	insert_asm_instr_after(asm_instr,NULL,NULL,clobbers,NULL,true,&gsi_entry_bb);


}

static void instrument_exit(function *fun, tree var){
	basic_block bb; 
	gimple_stmt_iterator gsi_exit_bb;
	tree input;
	vec<tree, va_gc> *inputs = NULL;
	vec<tree, va_gc> *clobbers = NULL;

	bb = single_pred(EXIT_BLOCK_PTR_FOR_FN(fun));
	gsi_exit_bb = gsi_last_bb(bb);

	clobbers = create_clobbers("rax,rdi,esi,memory");
	input = build_tree_list(NULL_TREE, build_const_char_string(2, "r"));
	input = chainon(NULL_TREE, build_tree_list(input, var));
	vec_safe_push(inputs, input);

	const char *asm_instr = "mov (%0), %%rax\n\t"
                            "xor 0x8(%%rbp), %%rax\n\t"
                            "jne __stack_chk_fail@plt\n\t"
							"mov %%rdx, %%rdi\n\t"
							"mov $0x8, %%esi\n\t"
							"call munmap@plt\n\t";
	
	insert_asm_instr_before(asm_instr,inputs,NULL,clobbers,NULL,true,&gsi_exit_bb);


}

static bool to_instrument(function *fun){
	bool to_instr = false;
	std::string func_name = function_name(fun);
	std::cout << "[*] Checking function: '" << func_name << "'" << std::endl;

	vec<tree, va_gc> *local_declarations = fun->local_decls;
	if(local_declarations == NULL)
		return false;

	for(tree *decl = local_declarations->begin(); decl != local_declarations->end(); decl++){
		if(TREE_CODE (TREE_TYPE (*decl)) == ARRAY_TYPE){
			std::cout << RED << "[-] Found Buffer: " << get_name(*decl) << std::endl << RESET;
			to_instr = true;
		}
	}

	return to_instr;
}

static unsigned int instrument_functions(function *fun){

	if(!to_instrument(fun))
		return 0;
	

	std::string func_name = function_name(fun);
  	std::cout << "[!] Instrumenting function: '" << func_name << "' at: " 
	  << LOCATION_FILE(fun->function_start_locus) << ":" << LOCATION_LINE(fun->function_start_locus) << std::endl;

	
	tree m_addr = create_pointer("m_addr");

	instrument_entry(fun, m_addr);

	instrument_exit(fun, m_addr);

	std::cout << GREEN << "[+] Instrumented funtion: '" << func_name << "'" << std::endl << RESET;


	return 0;
}




struct plugin_info instr_plugin_info = {
	"1.0", 
	"Instrumentation code plugin"
};


namespace {
    const pass_data instrumentation_pass_data = {
        GIMPLE_PASS,
        "instr_pass2",           /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        (PROP_ssa | PROP_cfg | PROP_gimple_leh),    /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct instrumentation_pass : gimple_opt_pass {
        instrumentation_pass(gcc::context *ctx) : gimple_opt_pass(instrumentation_pass_data, ctx){}

			unsigned int execute(function *fun) {
				return instrument_functions(fun);
			}
    };	


}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version){
  	if(!plugin_default_version_check(version, &gcc_version)){
		std::cerr << "Error, version mismatching!" << std::endl;
		return 1;
	}
	
	std::cout << std::endl;
	
	struct register_pass_info instr_pass_info;

	instr_pass_info.pass = new instrumentation_pass(g);
	instr_pass_info.reference_pass_name = "cfg";
	instr_pass_info.ref_pass_instance_number = 1;
  	instr_pass_info.pos_op = PASS_POS_INSERT_AFTER;


	register_callback(plugin_info->base_name, PLUGIN_INFO, NULL, &instr_plugin_info);
	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &instr_pass_info);
	

	

	return 0;
}

