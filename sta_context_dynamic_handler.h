
#ifdef STA_PUBLIC_API

/*
#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta-main/sta_context_dynamic_handler.h"
*/


/* --- Exception Type Constants --- */
#define CDH_OOM -2 /**< Out of Memory */
#define CDH_ERROR     -1  /**< Unkown error */

#define CDH_NONE      0  /**< No error */
#define CDH_ERROR_UNKOWN   1  /**< Unkown error */
#define CDH_STR       2  /**< Error payload is a string */
#define CDH_CODE      3  /**< Error payload is an integer code */
#define CDH_DATA      4  /**< Error payload is a custom data pointer */
#define CDH_DATA_USER 1001 /**< Base value for user-defined error types */

typedef struct {
    int type;  // -1 = erorr, 0=none,1 = error, 2=str,3 = code, 4..N = data
 
        int code; // for CODE type
		       char * data; // for STRING and DATA type 
	gt_term_free_func free_fn; // for both str and ptr (optional)
} cdh_exception_t;


#define a_throw(type,code,data,free_fn) do{ context_dynamic_handler_set_exception(cdh,type,code,data,free_fn); a_return(0) }while(0);
#define a_throw_i(code) a_throw(CDH_CODE,code,a_null,a_null)
#define a_throw_s(str)  a_throw(CDH_STR,CDH_ERROR,(void*)(str),a_null)
#define a_throw_s_copy(str) a_throw(CDH_STR,CDH_ERROR,cdh_str_copy_malloc(str), (gt_term_free_func)free)


 // delete old if needed
#define a_try a_sure(context_dynamic_handler_start_try(cdh)) do {
#define a_catch }while(0); if(a_is_error){
#define a_final } context_dynamic_handler_clear_exception_and_error(cdh);
#define a_fail } if(a_is_error) a_return(0)
#define Try a_try 
#define Catch a_catch
#define Final a_final
#define Fail a_fail




typedef struct context_dynamic_handler_s context_dynamic_handler_t;

struct context_dynamic_handler_s
{
	context_dynamic_handler_t * sub;
	context_dynamic_handler_t * super;
	//int sub_is_error;
	
	int is_error;
	void * r;//return value
	gt_list_t * dh;
	
		// sub_exception is owns and exception is not owned 
	cdh_exception_t * exception;// set set by caller, throw use this, its free by caller 
	cdh_exception_t * sub_exception;// for called function , sub->exception = cdh->sub_exception;
};


//note cdh parameter can null 
context_dynamic_handler_t * context_dynamic_handler_new(void);
context_dynamic_handler_t * context_dynamic_handler_sub(context_dynamic_handler_t * cdh);
void context_dynamic_handler_delete(context_dynamic_handler_t * cdh);

void * context_dynamic_handler_add(context_dynamic_handler_t * cdh,void * ptr,void * func);


//return pointer of var 
void * context_dynamic_handler_add_var(context_dynamic_handler_t * cdh,void ** var_ptr,void * func);

void * context_dynamic_handler_set_error(context_dynamic_handler_t * cdh,int is_error);




#define a_is_error context_dynamic_handler_get_error(cdh)
int context_dynamic_handler_get_error(context_dynamic_handler_t * cdh);
a_bool context_dynamic_handler_set_exception(context_dynamic_handler_t * cdh,int type,int code,void * data,gt_term_free_func free_fn);

int context_dynamic_handler_clear_exception_and_error(context_dynamic_handler_t * cdh);

a_bool context_dynamic_handler_start_try(context_dynamic_handler_t * cdh);








//var cdh
#define a_begin(expr) context_dynamic_handler_t * cdh = context_dynamic_handler_new();\
if(!cdh) return 0;

#define a_handle(ptr,func) if(ptr)\
{if(!context_dynamic_handler_add(cdh,ptr,func)) {(*func)(ptr); a_return(0)}}\
else a_return(0);
	
#define a_handle_var(var,func) if(!context_dynamic_handler_add_var(cdh,(void **)&var,func)){if(var)(*func)(var);a_return(0)}

#define a_handle_r(ptr,func) if(cdh){cdh->r = ptr; a_handle_var(cdh->r,func)}else{if(ptr)(*func)(ptr); a_return(0)}

#define a_return(expr) {context_dynamic_handler_delete(cdh);return expr;}
#define a_sure(expr) {if(!(expr)) a_return(0)}
//#define a_ok_cdh(expr) {if(cdh){ if(cdh->is_error) a_return(0) }}
#define a_ok(ptr) {if(ptr){if(ptr->is_error) a_return(0) } else a_return(0)}

#define a_return_ok(expr) {if(cdh) cdh->r = 0;context_dynamic_handler_delete(cdh);return expr;}

#define a_sub context_dynamic_handler_sub(cdh)
#define a_cdh context_dynamic_handler_t * cdh
/* 
//example a_cdh and a_sub( both can null)
int cdh_foo(a_cdh)
{
	//use a_cdh
}
#define foo(expr) cdh_foo(a_sub) 

//example a_begin and a_return
int fun(void)
{
	a_begin()
	
	cdh_foo(a_sub);
	foo()
	
	a_return(0)
}


*/

#define a_create_pod(name_of_type,name_of_variable) name_of_type * name_of_variable = (name_of_type *) malloc(sizeof(name_of_type)); a_handle(name_of_variable,free) \
memset(name_of_variable,0,sizeof(name_of_type));

// example: a_new_context_dynamic_handler_t a_delete_context_dynamic_handler_t
// format : {a_new_}name_of_type {a_delete_}name_of_type
#define a_create(name_of_type,name_of_variable) a_create_(name_of_type,(),name_of_variable)
//a_create_(a,(),v)
#define a_create_(name_of_type,parameters,name_of_variable) name_of_type * name_of_variable = a_new_##name_of_type parameters;\
a_handle(name_of_variable,a_delete_##name_of_type)


#define a_new(name_of_type,name_of_variable) a_new_(name_of_type,(),name_of_variable)

#define a_new_(name_of_type,parameters,name_of_variable)name_of_variable = a_new_##name_of_type parameters;\
a_handle(name_of_variable,a_delete_##name_of_type)



// A Project By Arshad Latti

#endif

#ifdef STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#define STA_GT_LIST_IMPLEMENTATION
#endif

#ifdef STA_PUBLIC_API


//#define STA_GT_LIST_IMPLEMENTATION
//#include "sta-main/sta_gt_list.h"


struct s_gt_list__node
{
	struct s_gt_list__node * next;
	struct s_gt_list__node * pre;
	gt_term_free_func data_term_func;
	char data[sizeof(struct s_gt_list__node *)];
};
typedef struct s_gt_list__node  gt_list__node_t;

struct s_gt_list
{
	gt_list__node_t * node_first;
	gt_list__node_t * node_last;
	
	size_t item_size;
	size_t items_count;
	
	gt_init_with_func item_init_with_func;
	gt_term_free_func item_term_free_func;
};
typedef struct s_gt_list gt_list_t;



gt_list_t * gt_list_new_ex(size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);
gt_list_t * gt_list_new(size_t item_size);
void gt_list_delete(gt_list_t * gtl);

a_bool gt_list_add_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);
a_bool gt_list_add(gt_list_t * gtl,void * item);

a_bool gt_list_add_first(gt_list_t * gtl,void * item);
a_bool gt_list_add_first_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);


gt_list__node_t * gt_list_add_at(gt_list_t * gtl,void * item,int index);
gt_list__node_t * gt_list_add_at_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func,int index);


void * gt_list_get_at(gt_list_t * gtl,int index);
//a_bool gt_list_add_first_ex



gt_list__node_t * gt_list__node_add(gt_list_t * gtl,size_t item_size,a_bool is_at_first);
gt_list__node_t * gt_list__node_remove(gt_list_t * gtl,a_bool is_at_first);

gt_list__node_t * gt_list__node_add_at(gt_list_t * gtl,size_t item_size,int index);
gt_list__node_t * gt_list__node_get_at(gt_list_t * gtl,int index);


// A Project By Arshad Latti

#endif

#ifndef STA_DEF_H_
#define STA_DEF_H_

#ifndef STA_NO_STDINT_H
#include <stdint.h>
#endif

/* 
//#define STA_NO_STDINT_H
//#define STA_COMPILER_VC_6

#ifdef STA_COMPILER_VC_6
#include "stdint-vc6.h"
#endif 
*/
 
#define a_bool int 
#define a_true 1
#define a_false 0
#define a_null 0


// PACKED 
/*
A_PACKED_BEGIN

struct disk_data
{
char x;
int y;
}A_PACKED;

A_PACKED_END
*/

#ifndef STA_PACKED_OPTIONS
//auto detect compiler
#define STA_PACKED_OPTIONS
#define STA_PACKED_COMPILER_GCC
//#define STA_PACKED_COMPILER_MSVC
//#define STA_PACKED_COMPILER_TCC

#endif

#ifdef STA_PACKED_COMPILER_GCC
#define A_PACKED_BEGIN
#define A_PACKED_END
#define A_PACKED __attribute__((packed))
#endif

#ifdef STA_PACKED_COMPILER_MSVC
#define A_PACKED_BEGIN #pragma pack( push,1 )
#define A_PACKED_END #pragma pack( pop )
#define A_PACKED 
#endif

// Framebuffer Image
#define framebuffer_pixel_t uint32_t

struct s_framebuffer_image
{
	framebuffer_pixel_t * pixels;
	int32_t h,w,pitch;//all in pixels
};
typedef struct s_framebuffer_image framebuffer_image_t;


//AV_PIX_FMT_BGRA
struct s_framebuffer_color
{
uint8_t b;
uint8_t g;
uint8_t r;
uint8_t a;
};
typedef struct s_framebuffer_color framebuffer_color_t;

//
struct s_clip_rect
{
	int32_t x,y,h,w;
};
typedef struct s_clip_rect clip_rect_t;

struct s_a_rect
{
	int32_t x,y,x2,y2;
};
typedef struct s_a_rect a_rect_t;


struct s_a_rect_size
{
	int h,w;
};
typedef struct s_a_rect_size a_rect_size_t;



struct s_a_point
{
	int x,y;
};
typedef struct s_a_point a_point_t;



typedef struct s_a_date_time a_date_time_t;
struct s_a_date_time
{
	uint16_t year;
	uint8_t month;
	uint8_t day;//date
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t flags;// bit 1.0=utc 2.0=24hr 3.0=date valid 4.0=time valid 5.0valid 6-8=resvered FF=not valid
};

//

#define A_MIN(a,b) ((a) < (b) ? (a) : (b))
#define A_MAX(a,b) ((a) < (b) ? (b) : (a))
#define A_KB(k) ((k) * 1024)
#define A_MB(mb) (A_KB((mb) * 1024))
#define A_GB(gb) (A_MB((gb) * 1024))
#define A_TB(tb) (A_GB((tb) * 1024))


//if.not if.index
#define if_not(expr) if(!(expr))
#define if_index(expr) if((expr) >= 0)



#define for_i(i_max) int i; for(i=0;i<i_max;i++)
#define for_i_(i_max) for(i=0;i<i_max;i++)



#define for_var(i,i_max) int i; for(i=0;i<i_max;i++)
#define for_var_(i,i_max) for(i=0;i<i_max;i++)
// for_j
#define for_j(x_max) for_var(j,x_max)
#define for_j_(x_max) for_var_(j,x_max)
		
// for_k
#define for_k(x_max) for_var(k,x_max)
#define for_k_(x_max) for_var_(k,x_max)
	
	
// for_x for_y
#define for_x(x_max) int x; for(x=0;x<x_max;x++)
#define for_x_(x_max) for(x=0;x<x_max;x++)
	
#define for_y(y_max) int y; for(y=0;y<y_max;y++)
#define for_y_(y_max) for(y=0;y<y_max;y++)
	
// #define for_i_xy(w,h)	int i_x,i_y,i_xy,i_xy_max; i_xy_max = w*h; for(i_xy=0;i_xy < i_xy_max;i_xy++

// use }} 
// #define for_xy_next }}
#define next_xy }}
#define for_xy(w,h)	int i_x,i_y;\
for(i_y = 0; i_y < h ; i_y++){\
for(i_x = 0; i_x < w ; i_x++){

#define for_xy_(w,h) for(i_y = 0; i_y < h ; i_y++){\
for(i_x = 0; i_x < w ; i_x++){

#define for_fi(fi) for_xy(fi->w,fi->h)
#define for_fi_(fi) for_xy_(fi->w,fi->h)
#define next_fi }


#define for_i_cc(s) int cc = strlen(s); int i; for(i=0;i<cc;i++)
#define for_i_cc_(s) cc = strlen(s); for(i=0;i<cc;i++)	
//#define for_cc(s) for_i_cc(s)


#define return_if_null(expr) if(!(expr)) return 0
#define return_if_zero(expr) if(!(expr)) return 0
#define return_if_fail(expr) if((expr) != 0) return 0
#define return_if_not_find(expr) if((expr) < 0) return 0

// ptr
//#define if
#define if_null(expr) if(!(expr))
//status	
#define if_success(expr) if((expr) == 0)
#define if_fail(expr) if((expr) != 0)
//index
#define if_not_find(expr) if((expr) < 0)
#define if_find(expr) if((expr) >=0)
	



#define puts_i(i) printf("%i\n",i);
#define puts_s_i(s,i) printf("%s %i\n",s,i);


#define IFF(expr,on_true,on_false) ((expr)? on_true : on_false)


#define bitwise_and(a,b) ((a)&(b))
#define bitwise_or(a,b) ((a)|(b))
#define bitwise_xor(a,b) ((a)^(b))
#define bitwise_not(a) (~(a))
#define bitwise_shift_left(a,n) ((a) << (n))
#define bitwise_shift_right(a,n) ((a) >> (n))

#define logical_and(a,b) ((a)&&(b))
#define logical_or(a,b) ((a)||(b))
#define logical_not(a) (!(a))

//#define A_DEBUG_PUTS
#ifdef A_DEBUG_PUTS
#define dputs_i(i) printf("%i\n",i);
#define dputs_s_i(s,i) printf("%s %i\n",s,i);
#endif


#define argc_msg(num,msg) if(argc < num){puts(msg); return 0;}
 
 
 //todo expr = 0;
 #define free_safe(expr) {if(expr) free(expr);}
 

 
/* verbose_printf */
//#define A_VERBOSE_PRINTF

#ifdef A_VERBOSE_PRINTF
#define verbose_printf printf
#else
#define verbose_printf(expr,...)
#endif

// A Project By Arshad Latti
#endif


#ifndef STA_GENERAL_TYPES_H
#define STA_GENERAL_TYPES_H

/* #include "sta_def.h"
#include <stddef.h>


 */
 
 #ifdef __cplusplus
extern "C" {
#endif
 
/*
gt_init_with_func
gt_term_free_func
gt_set_func // line a= b
gt_init_func//CCls c();

gt_is_equual_func
gt_is_greater_then_func
gt_is_less_then_func
gt_is_less_or_equle_func
gt_is_greater_or_equle_func
gt_append_func //like strcat
gt_math_add_func // math_add(out,a,b);
sub/div/mul etc..
*/


typedef void * (*gt_init_with_func)(void * dst,void * src);//return dst ptr on success
typedef void  (*gt_term_free_func)(void * ptr);
typedef void (*gt_set_func)(void * dst,void * src);
typedef void (*gt_init_func)(void * ptr);


#ifdef __cplusplus
}
#endif






#endif


#ifndef STA_GT_LIST_H
#define STA_GT_LIST_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


//#define STA_GT_LIST_IMPLEMENTATION
//#include "sta-main/sta_gt_list.h"


struct s_gt_list__node
{
	struct s_gt_list__node * next;
	struct s_gt_list__node * pre;
	gt_term_free_func data_term_func;
	char data[sizeof(struct s_gt_list__node *)];
};
typedef struct s_gt_list__node  gt_list__node_t;

struct s_gt_list
{
	gt_list__node_t * node_first;
	gt_list__node_t * node_last;
	
	size_t item_size;
	size_t items_count;
	
	gt_init_with_func item_init_with_func;
	gt_term_free_func item_term_free_func;
};
typedef struct s_gt_list gt_list_t;



gt_list_t * gt_list_new_ex(size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);
gt_list_t * gt_list_new(size_t item_size);
void gt_list_delete(gt_list_t * gtl);

a_bool gt_list_add_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);
a_bool gt_list_add(gt_list_t * gtl,void * item);

a_bool gt_list_add_first(gt_list_t * gtl,void * item);
a_bool gt_list_add_first_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func);


gt_list__node_t * gt_list_add_at(gt_list_t * gtl,void * item,int index);
gt_list__node_t * gt_list_add_at_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func,int index);


void * gt_list_get_at(gt_list_t * gtl,int index);
//a_bool gt_list_add_first_ex



gt_list__node_t * gt_list__node_add(gt_list_t * gtl,size_t item_size,a_bool is_at_first);
gt_list__node_t * gt_list__node_remove(gt_list_t * gtl,a_bool is_at_first);

gt_list__node_t * gt_list__node_add_at(gt_list_t * gtl,size_t item_size,int index);
gt_list__node_t * gt_list__node_get_at(gt_list_t * gtl,int index);


// A Project By Arshad Latti


void * sta_malloc_ex(void ** out_ptr_ptr,size_t size);
#define sta_malloc(var,size) sta_malloc_ex((void**)var,size)

#ifdef __cplusplus
}
#endif

#endif




#ifdef STA_GT_LIST_IMPLEMENTATION

#ifndef STA_GT_LIST_IMPLEMENTATION_INCLUDED
#define STA_GT_LIST_IMPLEMENTATION_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif



a_bool gt_list_add(gt_list_t * gtl,void * item)
{
	return gt_list_add_ex(gtl,item,gtl->item_size,gtl->item_init_with_func,gtl->item_term_free_func);
}

gt_list__node_t * gt_list_add_at(gt_list_t * gtl,void * item,int index)
{
	
		return gt_list_add_at_ex(gtl,item,gtl->item_size,gtl->item_init_with_func,gtl->item_term_free_func,index);

	
}


gt_list__node_t * gt_list_add_at_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func,int index)
{
	gt_list__node_t * node = gt_list__node_add_at(gtl,item_size,index);
	if(node)
	{
	
	if(item_init_with_func)
		(*item_init_with_func)(node->data,item);
	else
		memcpy(node->data,item,item_size);
	node->data_term_func = item_term_free_func;
	}
	return node;
}







a_bool gt_list_add_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func)
{
	gt_list__node_t * node = gt_list__node_add(gtl,item_size,a_false);
	if(!node)
		return a_false;
	
	if(item_init_with_func)
		(*item_init_with_func)(node->data,item);
	else
		memcpy(node->data,item,item_size);
	node->data_term_func = item_term_free_func;
	
	return a_true;
	
}





a_bool gt_list_add_first(gt_list_t * gtl,void * item)
{
	return gt_list_add_first_ex(gtl,item,gtl->item_size,gtl->item_init_with_func,gtl->item_term_free_func);
}



a_bool gt_list_add_first_ex(gt_list_t * gtl,void * item,size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func)
{
	gt_list__node_t * node = gt_list__node_add(gtl,item_size,a_true);
	if(!node)
		return a_false;
	
	if(item_init_with_func)
		(*item_init_with_func)(node->data,item);
	else
		memcpy(node->data,item,item_size);
	node->data_term_func = item_term_free_func;
	return a_true;
}



void gt_list_delete(gt_list_t * gtl)
{
	gt_list__node_t * node;
	while((node = gt_list__node_remove(gtl,a_false)))
	{
		if(node->data_term_func)
			(*node->data_term_func)(node->data);
		free(node);
	}
	free(gtl);
}


void * gt_list_get_at(gt_list_t * gtl,int index)
{
	
	gt_list__node_t * node= gt_list__node_get_at(gtl,index);
	if(node)
	{
		
		return node->data;
	}
	
	return a_null;
}


gt_list_t * gt_list_new(size_t item_size)
{
	return gt_list_new_ex(item_size,a_null,a_null);
}



gt_list_t * gt_list_new_ex(size_t item_size,gt_init_with_func item_init_with_func,gt_term_free_func item_term_free_func)
{
	gt_list_t * gtl;
	sta_malloc(&gtl,sizeof(gt_list_t));
	if(!gtl)
		return a_null;
	gtl->node_first = a_null;
	gtl->node_last  = a_null;
	gtl->item_size  = item_size;
	gtl->items_count=0;
	gtl->item_init_with_func = item_init_with_func;
	gtl->item_term_free_func = item_term_free_func;
	return gtl;//FIXED  not return gtl
}


gt_list__node_t * gt_list__node_add(gt_list_t * gtl,size_t item_size,a_bool is_at_first)
{
	gt_list__node_t * node;
	if(!sta_malloc(&node,sizeof(gt_list__node_t) + (item_size- sizeof(gt_list__node_t*))))
		return a_null;
	
	gtl->items_count++;
	if(gtl->items_count == 1)//if list is empty
	{
		gtl->node_first = node;
		gtl->node_last  = node;
		return node;
	}
	
	gt_list__node_t * node_pre;
	
	if(is_at_first)
	{
	node_pre = gtl->node_first;
	node_pre->pre = node;
	node->pre = a_null;
	node->next = node_pre;
	gtl->node_first = node;
	}
    else
	{
	node_pre = gtl->node_last;
	node_pre->next=node;
	node->pre = node_pre;
	node->next = a_null;
	gtl->node_last = node;
	}


	return node;
}




gt_list__node_t * gt_list__node_add_at(gt_list_t * gtl,size_t item_size,int index)
{
	if(index == 0)
		return gt_list__node_add(gtl,item_size,a_true);
	if(index >= gtl->items_count)
		return gt_list__node_add(gtl,item_size,a_false);
	
	
	gt_list__node_t * node;
	
	   
	gt_list__node_t * node_pre;
    node_pre = gt_list__node_get_at(gtl,index);
	
	if(!node_pre)
		return a_null;
	
	if(!sta_malloc(&node,sizeof(gt_list__node_t) + (item_size- sizeof(gt_list__node_t*))))
		return a_null;
	
    node->pre = node_pre->pre;
	node->next = node_pre;
	
	node_pre->pre = node;
   node->pre->next = node;
	gtl->items_count++;

	return node;
	
}




gt_list__node_t * gt_list__node_get_at(gt_list_t * gtl,int index)
{
		gt_list__node_t * node;
	if(!gtl->items_count)
		return a_null;
	if( index >= gtl->items_count)
		return a_null;
	int i = 0;
	node  = gtl->node_first;
	//if(index == 0)
		//return node;
	for(i=0;i< gtl->items_count;i++)
	{
			if(index == i)
			return node;
			node = node->next;
	if(!node)
		return a_null;
	}
	
	
	
	
	return a_null;
}



gt_list__node_t * gt_list__node_remove(gt_list_t * gtl,a_bool is_at_first)
{
	gt_list__node_t * node;
	
	if(!gtl->items_count)
		return a_null;
	
	if(gtl->items_count == 1)
	{
		node = gtl->node_first;
		gtl->items_count=0;
		return node;
	}
	
	gtl->items_count--;
	
	if(is_at_first)
	{
		node = gtl->node_first;
		gtl->node_first = node->next;
	}
	else
	{
		node = gtl->node_last;
		gtl->node_last = node->pre;
	}
	return node;
}



void * sta_malloc_ex(void ** out_ptr_ptr,size_t size)
{
	void * mem = malloc(size);
	if(out_ptr_ptr)
		*out_ptr_ptr = mem;
	
	return mem;
}


#ifdef __cplusplus
}
#endif

#endif

#endif




#ifndef STA_CONTEXT_DYNAMIC_HANDLER_H
#define STA_CONTEXT_DYNAMIC_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

struct s_context_dynamic_handler__item
{
	void * ptr;
	gt_term_free_func term_func;
	a_bool is_var;
};
typedef struct s_context_dynamic_handler__item context_dynamic_handler__item_t;
void context_dynamic_handler__item_term(void * p);

/*
#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta-main/sta_context_dynamic_handler.h"
*/


/* --- Exception Type Constants --- */
#define CDH_OOM -2 /**< Out of Memory */
#define CDH_ERROR     -1  /**< Unkown error */

#define CDH_NONE      0  /**< No error */
#define CDH_ERROR_UNKOWN   1  /**< Unkown error */
#define CDH_STR       2  /**< Error payload is a string */
#define CDH_CODE      3  /**< Error payload is an integer code */
#define CDH_DATA      4  /**< Error payload is a custom data pointer */
#define CDH_DATA_USER 1001 /**< Base value for user-defined error types */

typedef struct {
    int type;  // -1 = erorr, 0=none,1 = error, 2=str,3 = code, 4..N = data
 
        int code; // for CODE type
		       char * data; // for STRING and DATA type 
	gt_term_free_func free_fn; // for both str and ptr (optional)
} cdh_exception_t;


#define a_throw(type,code,data,free_fn) do{ context_dynamic_handler_set_exception(cdh,type,code,data,free_fn); a_return(0) }while(0);
#define a_throw_i(code) a_throw(CDH_CODE,code,a_null,a_null)
#define a_throw_s(str)  a_throw(CDH_STR,CDH_ERROR,(void*)(str),a_null)
#define a_throw_s_copy(str) a_throw(CDH_STR,CDH_ERROR,cdh_str_copy_malloc(str), (gt_term_free_func)free)


 // delete old if needed
#define a_try a_sure(context_dynamic_handler_start_try(cdh)) do {
#define a_catch }while(0); if(a_is_error){
#define a_final } context_dynamic_handler_clear_exception_and_error(cdh);
#define a_fail } if(a_is_error) a_return(0)
#define Try a_try 
#define Catch a_catch
#define Final a_final
#define Fail a_fail




typedef struct context_dynamic_handler_s context_dynamic_handler_t;

struct context_dynamic_handler_s
{
	context_dynamic_handler_t * sub;
	context_dynamic_handler_t * super;
	//int sub_is_error;
	
	int is_error;
	void * r;//return value
	gt_list_t * dh;
	
		// sub_exception is owns and exception is not owned 
	cdh_exception_t * exception;// set set by caller, throw use this, its free by caller 
	cdh_exception_t * sub_exception;// for called function , sub->exception = cdh->sub_exception;
};


//note cdh parameter can null 
context_dynamic_handler_t * context_dynamic_handler_new(void);
context_dynamic_handler_t * context_dynamic_handler_sub(context_dynamic_handler_t * cdh);
void context_dynamic_handler_delete(context_dynamic_handler_t * cdh);

void * context_dynamic_handler_add(context_dynamic_handler_t * cdh,void * ptr,void * func);


//return pointer of var 
void * context_dynamic_handler_add_var(context_dynamic_handler_t * cdh,void ** var_ptr,void * func);

void * context_dynamic_handler_set_error(context_dynamic_handler_t * cdh,int is_error);




#define a_is_error context_dynamic_handler_get_error(cdh)
int context_dynamic_handler_get_error(context_dynamic_handler_t * cdh);
a_bool context_dynamic_handler_set_exception(context_dynamic_handler_t * cdh,int type,int code,void * data,gt_term_free_func free_fn);

int context_dynamic_handler_clear_exception_and_error(context_dynamic_handler_t * cdh);

a_bool context_dynamic_handler_start_try(context_dynamic_handler_t * cdh);








//var cdh
#define a_begin(expr) context_dynamic_handler_t * cdh = context_dynamic_handler_new();\
if(!cdh) return 0;

#define a_handle(ptr,func) if(ptr)\
{if(!context_dynamic_handler_add(cdh,ptr,func)) {(*func)(ptr); a_return(0)}}\
else a_return(0);
	
#define a_handle_var(var,func) if(!context_dynamic_handler_add_var(cdh,(void **)&var,func)){if(var)(*func)(var);a_return(0)}

#define a_handle_r(ptr,func) if(cdh){cdh->r = ptr; a_handle_var(cdh->r,func)}else{if(ptr)(*func)(ptr); a_return(0)}

#define a_return(expr) {context_dynamic_handler_delete(cdh);return expr;}
#define a_sure(expr) {if(!(expr)) a_return(0)}
//#define a_ok_cdh(expr) {if(cdh){ if(cdh->is_error) a_return(0) }}
#define a_ok(ptr) {if(ptr){if(ptr->is_error) a_return(0) } else a_return(0)}

#define a_return_ok(expr) {if(cdh) cdh->r = 0;context_dynamic_handler_delete(cdh);return expr;}

#define a_sub context_dynamic_handler_sub(cdh)
#define a_cdh context_dynamic_handler_t * cdh
/* 
//example a_cdh and a_sub( both can null)
int cdh_foo(a_cdh)
{
	//use a_cdh
}
#define foo(expr) cdh_foo(a_sub) 

//example a_begin and a_return
int fun(void)
{
	a_begin()
	
	cdh_foo(a_sub);
	foo()
	
	a_return(0)
}


*/

#define a_create_pod(name_of_type,name_of_variable) name_of_type * name_of_variable = (name_of_type *) malloc(sizeof(name_of_type)); a_handle(name_of_variable,free) \
memset(name_of_variable,0,sizeof(name_of_type));

// example: a_new_context_dynamic_handler_t a_delete_context_dynamic_handler_t
// format : {a_new_}name_of_type {a_delete_}name_of_type
#define a_create(name_of_type,name_of_variable) a_create_(name_of_type,(),name_of_variable)
//a_create_(a,(),v)
#define a_create_(name_of_type,parameters,name_of_variable) name_of_type * name_of_variable = a_new_##name_of_type parameters;\
a_handle(name_of_variable,a_delete_##name_of_type)


#define a_new(name_of_type,name_of_variable) a_new_(name_of_type,(),name_of_variable)

#define a_new_(name_of_type,parameters,name_of_variable)name_of_variable = a_new_##name_of_type parameters;\
a_handle(name_of_variable,a_delete_##name_of_type)



// A Project By Arshad Latti


char * cdh_str_copy_malloc(const char * str);


void cdh_exception_clear(cdh_exception_t * e);
cdh_exception_t * cdh_exception_new(void);
void cdh_exception_delete(cdh_exception_t * cdhe);
a_bool cdh_exception_set(cdh_exception_t * cdhe,int type,int code,void * data,gt_term_free_func free_fn);


#ifdef __cplusplus
}
#endif

#endif





#ifdef STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION

#ifndef STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION_INCLUDED
#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif



//cdh_exception_clear coded by deepseek
void cdh_exception_clear(cdh_exception_t * e)
{
    if (!e) return;
    if (e->free_fn && e->data) {
        /* Only string-like and data payloads are freed.  Code-only
           exceptions have data == NULL so nothing happens. */
        (*e->free_fn)(e->data);
    }
    memset(e, 0, sizeof(cdh_exception_t));
}



//cdh_exception_delete is coded by deepseek

void cdh_exception_delete(cdh_exception_t * e)
{
    if (!e) return;
    cdh_exception_clear(e);
    free(e);
}


//cdh_exception_new coded by deepseek
cdh_exception_t * cdh_exception_new(void)
{
    return (cdh_exception_t *)calloc(1, sizeof(cdh_exception_t));
}



//cdh_exception_set is co coded by DeepSeek and Arshad Latti
a_bool cdh_exception_set(cdh_exception_t * e, int type, int code,
                         void * data, gt_term_free_func free_fn)
{
    if (!e) return a_false;
    cdh_exception_clear(e);
    e->type    = type;
    e->code    = code;
    e->data    = (char *)data;
    e->free_fn = free_fn;
	
	if(type == CDH_STR && !data)
	{
		 e->free_fn = a_null;
		 e->type = CDH_OOM;
	}
    return a_true;
}



//cdh_str_copy_malloc coded by DeepSeek
char * cdh_str_copy_malloc(const char * str)
{
    if (!str) return a_null;
    size_t n = strlen(str) + 1;
    char * p = (char *)malloc(n);
    if (p) memcpy(p, str, n);
    return p;
}

void * context_dynamic_handler_add(context_dynamic_handler_t * cdh,void * ptr,void * func)
{
	
		if(!cdh) return a_null;
	
	
	context_dynamic_handler__item_t  item;
	item.ptr = ptr;
	item.is_var = a_false;
	item.term_func = (gt_term_free_func)func;
	if(gt_list_add(cdh->dh,&item))
		return ptr;
 return a_null;
}

void * context_dynamic_handler_add_var(context_dynamic_handler_t * cdh,void ** var_ptr,void * func){
	
	if(!cdh) return a_null;
	
		context_dynamic_handler__item_t item;
	item.ptr = (void*)var_ptr;
	item.is_var = a_true;
	item.term_func = (gt_term_free_func)func;
	if(gt_list_add(cdh->dh,&item))
		return (void*)var_ptr;
 return a_null;
}

//context_dynamic_handler_clear_exception_and_error  is coded with help of DeepSeek
int context_dynamic_handler_clear_exception_and_error(context_dynamic_handler_t * cdh)
{
    if (!cdh) return 0;
    cdh_exception_clear(cdh->sub_exception);
   // cdh_exception_clear(cdh->exception);
    cdh->is_error = 0;
    return 0;
}

void context_dynamic_handler_delete(context_dynamic_handler_t * cdh)
{
	if(cdh)
	{
	 if(cdh->sub)
       context_dynamic_handler_delete(cdh->sub);
    if (cdh->super) cdh->super->sub = a_null;
	
	if (cdh->sub_exception) 
		cdh_exception_delete(cdh->sub_exception);
	
      gt_list_delete(cdh->dh);
	  free(cdh);
	}
}


int context_dynamic_handler_get_error(context_dynamic_handler_t * cdh)
{
	if(cdh)
	{
	if(cdh->is_error) return cdh->is_error;
	//for handling in this function, try catch block
	if(cdh->sub_exception)
	 {
	   if(cdh->sub_exception->type) return cdh->sub_exception->type;
     }
	 // for bubble up exception
	if(cdh->exception)
	 {
	   if(cdh->exception->type) return cdh->exception->type;
     }
	 return 0;
	}
	
	
	
	return 1;
}

context_dynamic_handler_t * context_dynamic_handler_new(void)
{
	context_dynamic_handler_t * r = (context_dynamic_handler_t *) malloc(sizeof(context_dynamic_handler_t));
	if(r)
	{
		
	
	r->dh = gt_list_new_ex(sizeof(context_dynamic_handler__item_t),a_null,context_dynamic_handler__item_term);
	if(!r->dh)
	{
		free(r);
		return a_null;
	}
	r->is_error=0;
	r->r=a_null;
	r->sub= a_null;
	r->super = a_null;
	r->exception = a_null;
	r->sub_exception =a_null;
	}
	
	return r;
}


void * context_dynamic_handler_set_error(context_dynamic_handler_t * cdh,int is_error)
{
	
	if(cdh)
	{
		cdh->is_error = is_error;
	}
	return cdh;
}

//context_dynamic_handler_set_exception is coded with help of DeepSeek


a_bool context_dynamic_handler_set_exception(context_dynamic_handler_t * cdh,
                                          int type, int code,
                                          void * data,
                                          gt_term_free_func free_fn)
{
	a_bool is_added = a_false;
	if(cdh) 
	{
	cdh->is_error = a_true;
	
    if (cdh->exception) {
		cdh_exception_set(cdh->exception, type, code, data, free_fn);
	 is_added = a_true;
	}

	}
	
    if(!is_added)
	{
		 if (free_fn && data) {
        (*free_fn)(data);
    }
	}
    return is_added;
}

//context_dynamic_handler_start_try
// context_dynamic_handler_start_try is coded by DeepSeek
a_bool context_dynamic_handler_start_try(context_dynamic_handler_t * cdh)
{
    if (!cdh) return a_false;
    if (cdh->sub_exception) cdh_exception_clear(cdh->sub_exception);
    else {
        cdh->sub_exception = cdh_exception_new();
        if (!cdh->sub_exception) return a_false;
    }
    return a_true;
}


context_dynamic_handler_t * context_dynamic_handler_sub(context_dynamic_handler_t * cdh)
{
	if(cdh)
	{
		if(cdh->sub)
			context_dynamic_handler_delete(cdh->sub);
		cdh->sub = context_dynamic_handler_new();
		 if (cdh->sub) {
        cdh->sub->exception = cdh->sub_exception;
		cdh->sub->super = cdh;
    }
		return cdh->sub;
	}
	return a_null;
}


void context_dynamic_handler__item_term(void * p)
{
	
	context_dynamic_handler__item_t * itm=(context_dynamic_handler__item_t*)p;
	void ** ptr;
	if(!itm)
		return;
	
	if(!itm->term_func)
		return;
	
	if(!itm->ptr)
		return;
	
	if(itm->is_var)
	{
		ptr = (void**)itm->ptr;
		if(*ptr)(*itm->term_func)(*ptr); //	(*itm->term_func)(*ptr);
	}
	else
	{
		(*itm->term_func)(itm->ptr);
	}
	
}



#ifdef __cplusplus
}
#endif

#endif

#endif



