
module libvnv
    
    use iso_c_binding
    implicit none
    private
    public :: vnv_initialize
    public :: vnv_finalize
    public :: vnv_loop_init
    public :: vnv_loop_begin
    public :: vnv_loop_iter
    public :: vnv_loop_end
    public :: vnv_point_init
    public :: vnv_point_run
    public :: vnv_plug_init
    public :: vnv_plug_run
    public :: vnv_iterator_init
    public :: vnv_iterator_run
    public :: vnv_declare_integer_parameter
    public :: vnv_declare_float_parameter
    public :: vnv_declare_double_parameter
    public :: vnv_declare_string_parameter
    public :: vnv_declare_integer_array
    public :: vnv_declare_float_array
    public :: vnv_declare_double_array
    public :: vnv_declare_integer_matrix
    public :: vnv_declare_float_matrix
    public :: vnv_declare_double_matrix
    public :: vnv_get_integer_parameter
    public :: vnv_get_float_parameter
    public :: vnv_get_double_parameter
    public :: vnv_get_string_parameter
    public :: vnv_debug
    public :: vnv_info
    public :: vnv_warning
    public :: vnv_error
    public :: vnv_input_file
    public :: vnv_output_file
    
    integer, parameter, public :: VNV_JSON_NULL = 0
    integer, parameter, public :: VNV_JSON_OBJECT = 1
    integer, parameter, public :: VNV_JSON_ARRAY = 2
    integer, parameter, public :: VNV_JSON_BOOLEAN = 3
    integer, parameter, public :: VNV_JSON_INTEGER = 4
    integer, parameter, public :: VNV_JSON_FLOAT = 5
    integer, parameter, public :: VNV_JSON_STRING = 6
 
    interface
        
  ! void functions maps to subroutines
    TYPE(C_PTR) function vnv_init_start_c(package, regis) bind(C, name="vnv_init_start_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        type(c_funptr), intent(in) :: regis
    end function

    ! void functions maps to subroutines
    subroutine vnv_add_arg_c(ctx, str) bind(C, name="vnv_add_arg_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: str(*)
    end subroutine


  ! void functions maps to subroutines
    subroutine vnv_init_end_c(ctx) bind(C, name="vnv_init_end_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end subroutine

 ! void functions maps to subroutines
    subroutine vnv_finalize_c(package) bind(C, name="vnv_finalize_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
    end subroutine

    
    TYPE(C_PTR) function vnv_loop_init_c(world,package,fname) bind(C, name="vnv_loop_init_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: fname(*)
    end function

    subroutine vnv_loop_begin_c(ctx) bind(C, name="vnv_loop_begin_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end subroutine

    subroutine vnv_loop_iter_c(ctx,iter) bind(C, name="vnv_loop_iter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: iter(*)
    end subroutine
    
    
    subroutine vnv_loop_end_c(ctx) bind(C, name="vnv_loop_end_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end subroutine
    
    TYPE(C_PTR) function vnv_point_init_c(world,package,fname) bind(C, name="vnv_point_init_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: fname(*)
    end function

    subroutine vnv_point_run_c(ctx) bind(C, name="vnv_point_run_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end subroutine
    
    TYPE(C_PTR) function vnv_plug_init_c(world, package,fname) bind(C, name="vnv_plug_init_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: fname(*)
    end function

    INTEGER(C_INT) function vnv_plug_run_c(ctx) bind(C, name="vnv_plug_run_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end function
    

    TYPE(C_PTR) function vnv_iterator_init_c(world,package,fname, once) bind(C, name="vnv_iterator_init_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: fname(*)
        INTEGER(C_INT), intent(in) :: once
    end function

    INTEGER(C_INT) function vnv_iterator_run_c(ctx) bind(C, name="vnv_iterator_run_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
    end function

    subroutine vnv_declare_integer_parameter_c(ctx,name,value) bind(C, name="vnv_declare_integer_parameter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        INTEGER(C_INT), intent(in) :: value
    end subroutine
    
    subroutine vnv_declare_float_parameter_c(ctx,name,value) bind(C, name="vnv_declare_float_parameter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(C_FLOAT), intent(in) :: value
    end subroutine
    
    subroutine vnv_declare_double_parameter_c(ctx,name,value) bind(C, name="vnv_declare_double_parameter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(C_DOUBLE), intent(in) :: value
    end subroutine

    subroutine vnv_declare_string_parameter_c(ctx,name,value) bind(C, name="vnv_declare_string_parameter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        character(len=1, kind=C_CHAR), intent(in) :: value(*)
    end subroutine

    subroutine vnv_declare_integer_array_c(ctx,name,value,n) bind(C, name="vnv_declare_integer_array_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        INTEGER(C_INT), dimension(*), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: n        
    end subroutine

    subroutine vnv_declare_float_array_c(ctx,name,value,n) bind(C, name="vnv_declare_float_array_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(c_float), dimension(*), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: n        
    end subroutine

    subroutine vnv_declare_double_array_c(ctx,name,value,n) bind(C, name="vnv_declare_double_array_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(C_DOUBLE), dimension(*), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: n        
    end subroutine


    subroutine vnv_declare_integer_matrix_c(ctx,name,value,d1,d2) bind(C, name="vnv_declare_integer_matrix_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        INTEGER(c_int), dimension(:,:), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: d1,d2        
    end subroutine


    subroutine vnv_declare_float_matrix_c(ctx,name,value,d1,d2) bind(C, name="vnv_declare_float_matrix_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(C_FLOAT), dimension(:,:), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: d1,d2        
    end subroutine

    subroutine vnv_declare_double_matrix_c(ctx,name,value,d1,d2) bind(C, name="vnv_declare_double_matrix_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        REAL(C_DOUBLE), dimension(:,:), intent(inout) :: value
        INTEGER(C_SIZE_T), intent(in) :: d1,d2        
    end subroutine

    subroutine vnv_declare_type_parameter_c(ctx,name,ty,value) bind(C,name="vnv_declare_type_parameter_x")
        use iso_c_binding
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        character(len=1, kind=C_CHAR), intent(in) :: ty(*)
        type(c_ptr), value :: value
      end subroutine

    INTEGER(C_INT) function  vnv_get_integer_parameter_c(package,parameter,def) bind(C, name="vnv_get_integer_parameter_x")
      use iso_c_binding
      implicit none
      character(len=1, kind=C_CHAR), intent(in) :: package(*)
      character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
      INTEGER(C_INT), intent(in) :: def
    end function
  
    REAL(C_FLOAT) function  vnv_get_float_parameter_c(package,parameter,def) bind(C, name="vnv_get_float_parameter_x")
      use iso_c_binding
      implicit none
      character(len=1, kind=C_CHAR), intent(in) :: package(*)
      character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
      REAL(C_FLOAT), intent(in) :: def
    end function
  
    REAL(C_DOUBLE) function  vnv_get_double_parameter_c(package,parameter,def) bind(C, name="vnv_get_double_parameter_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
        REAL(C_DOUBLE), intent(in) :: def
    end function

    TYPE(C_PTR) function  vnv_get_string_parameter_c(package,parameter) bind(C, name="vnv_get_str_parameter_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
    end function

    INTEGER(C_INT) function  vnv_get_parameter_type_c(package,parameter) bind(C, name="vnv_get_parameter_type_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
    end function


    INTEGER(C_INT) function  vnv_get_parameter_size_c(package,parameter) bind(C, name="vnv_get_parameter_size_x")
        use iso_c_binding
        implicit none
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: parameter(*)
    end function

    subroutine vnv_log_c(world,level,package,message) bind(C, name="vnv_log_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        character(len=1, kind=C_CHAR), intent(in) :: level(*)
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: message(*)
    end subroutine

   
    subroutine vnv_file_c(world, input, package, name, filename,reader, action) bind(C, name="vnv_file_x")
        use iso_c_binding
        implicit none
        INTEGER(C_INT), intent(in) :: world
        INTEGER(C_INT), intent(in) :: input        
        character(len=1, kind=C_CHAR), intent(in) :: package(*)
        character(len=1, kind=C_CHAR), intent(in) :: name(*)
        character(len=1, kind=C_CHAR), intent(in) :: filename(*)
        character(len=1, kind=C_CHAR), intent(in) :: reader(*)
        INTEGER(C_INT), intent(in) :: action        
    end subroutine
 

    end interface

contains ! Implementation of the functions. We just wrap the C function here.

    function convert(str) result(c_str)
        implicit none
        character(len=*), intent(in) :: str
        character(len=1, kind=C_CHAR) :: c_str(len_trim(str) + 1)
        integer :: N, i

        ! Converting Fortran string to C string
        N = len_trim(str)
        do i = 1, N
            c_str(i) = str(i:i)
        end do
        c_str(N + 1) = C_NULL_CHAR
        return 
    end function convert

  

    subroutine vnv_add_arg(ctx, str)
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=*), intent(in) :: str
        call vnv_add_arg_c(ctx, convert(str))
    end subroutine

    subroutine vnv_declare_type_parameter(ctx, name, ty, value)
        implicit none
        character(len=*), intent(in) :: name
        character(len=*), intent(in) :: ty
        TYPE(C_PTR), intent(in) :: ctx
        type(c_ptr), intent(in) :: value
        call vnv_declare_type_parameter_c(ctx, convert(name), convert(ty), value)
    end subroutine

    subroutine vnv_declare_integer_parameter(ctx, name,value)
        implicit none
        character(len=*), intent(in) :: name  
        TYPE(C_PTR), intent(in) :: ctx
        integer(kind=c_int), intent(in) :: value
        call vnv_declare_integer_parameter_c(ctx, convert(name),  value)
    end subroutine
    
    subroutine vnv_declare_float_parameter(ctx, name,value)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real, intent(in) :: value
        call vnv_declare_float_parameter_c(ctx,convert(name), real(value, kind=c_float))
    end subroutine
    
    subroutine vnv_declare_double_parameter(ctx,name,value)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real(8), intent(in) :: value
        call vnv_declare_double_parameter_c(ctx,convert(name), real(value, kind=c_double))
    end subroutine
    
    subroutine vnv_declare_string_parameter(ctx,name,value)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        character(len=*), intent(in) :: value
        call vnv_declare_string_parameter_c(ctx, convert(name),convert(value))
    end subroutine

    subroutine vnv_declare_integer_array(ctx,name,value,n)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        integer, dimension(*), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: n
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_integer_array_c(ctx,name_c_str, value, int(n, kind=c_size_t))
    end subroutine
    

    subroutine vnv_declare_float_array(ctx,name,value,n)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real(kind=c_float), dimension(*), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: n
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_float_array_c(ctx,name_c_str, value, int(n, kind=c_size_t))
    end subroutine


    subroutine vnv_declare_double_array(ctx,name,value,n)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real(kind=c_double), dimension(*), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: n
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_double_array_c(ctx,name_c_str, value, int(n, kind=c_size_t))
    end subroutine

    subroutine vnv_declare_integer_matrix(ctx,name,value,d0,d1)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        integer(kind=c_int), dimension(:,:), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: d0,d1
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_integer_matrix_c(ctx,name_c_str, value,int(d0, kind=c_size_t),int(d1, kind=c_size_t))
    end subroutine

    subroutine vnv_declare_float_matrix(ctx,name,value,d0,d1)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real(kind=c_float), dimension(:,:), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: d0,d1
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_float_matrix_c(ctx,name_c_str, value, int(d0, kind=c_size_t),int(d1, kind=c_size_t))
    end subroutine

    subroutine vnv_declare_double_matrix(ctx,name,value,d0,d1)
        implicit none
        character(len=*), intent(in) :: name
        TYPE(C_PTR), intent(in) :: ctx
        real(kind=c_double), dimension(:,:), intent(inout) :: value
        integer(kind=c_int), value, intent(in) :: d0,d1
        character(len=1, kind=C_CHAR) :: name_c_str(len_trim(name) + 1)
        name_c_str = convert(name)        
        call vnv_declare_double_matrix_c(ctx,name_c_str, value, int(d0, kind=c_size_t),int(d1, kind=c_size_t))
    end subroutine


    TYPE(C_PTR) function vnv_point_init(world,package,fname)
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: fname
        character(len=1, kind=C_CHAR) :: package_c_str(len_trim(package) + 1)
        character(len=1, kind=C_CHAR) :: fname_c_str(len_trim(fname) + 1)
        package_c_str = convert(package)        
        fname_c_str = convert(fname)        
        vnv_point_init = vnv_point_init_c(int(world,kind=c_int), package_c_str,fname_c_str)
    end function

    subroutine vnv_point_run(ctx)
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        call vnv_point_run_c(ctx)
    end subroutine
    
    function vnv_loop_init(world,package,fname) result(ptr)
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: fname
        TYPE(C_PTR) :: ptr
        ptr = vnv_loop_init_c(int(world,kind=c_int),convert(package),convert(fname))
    end function

    subroutine vnv_loop_begin(ctx)
        implicit none
        TYPE(C_PTR), intent(in) :: ctx        
        call vnv_loop_begin_c(ctx)
    end subroutine

    subroutine vnv_loop_iter(ctx,iter)
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        character(len=*), intent(in) :: iter        
        call vnv_loop_iter_c(ctx,convert(iter))
    end subroutine

    subroutine vnv_loop_end(ctx)
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        call vnv_loop_end_c(ctx)
    end subroutine

    subroutine vnv_initialize(package,fname,regis)
       CHARACTER(*), intent(in) :: package
       CHARACTER(*), intent(in) :: fname
       type(c_funptr), intent(in) :: regis

       INTEGER :: i
       CHARACTER(len=32) :: arg
       TYPE(C_PTR) :: ctx
       
       ctx = vnv_init_start_c(convert(package), regis)

       DO i = 0, iargc()
            CALL getarg(i,arg)
            CALL vnv_add_arg(ctx,arg)
       END DO

       call vnv_init_end_c(ctx)

    end subroutine vnv_initialize

    subroutine vnv_finalize(package)
       CHARACTER(*), intent(in) :: package
       character(len=1, kind=C_CHAR) :: package_c_str(len_trim(package) + 1)
       package_c_str = convert(package)        
       CALL vnv_finalize_c(package_c_str)        
     end subroutine vnv_finalize
 

     TYPE(C_PTR) function vnv_plug_init(world,package,fname)
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: fname
        vnv_plug_init = vnv_plug_init_c(int(world,kind=c_int),convert(package),convert(fname))
    end function

    INTEGER(C_INT) function vnv_plug_run(ctx) 
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        vnv_plug_run = vnv_plug_run_c(ctx)
        return;
    end function
     
    TYPE(C_PTR) function vnv_iterator_init(world,package,fname, once)
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: fname
        INTEGER, intent(in) :: once       
        vnv_iterator_init = vnv_iterator_init_c(int(world,kind=c_int),convert(package),convert(fname),INT(once,kind=C_INT))
    end function

    INTEGER(C_INT) function vnv_iterator_run(ctx) 
        implicit none
        TYPE(C_PTR), intent(in) :: ctx
        vnv_iterator_run = vnv_iterator_run_c(ctx)
        return;
    end function

    INTEGER(C_INT) function vnv_get_integer_parameter(package,parameter, def) 
        implicit none
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        INTEGER(C_INT), intent(in) :: def 
        vnv_get_integer_parameter = vnv_get_integer_parameter_c(convert(package),convert(parameter), def)
        return;
    end function

    REAL(C_FLOAT) function vnv_get_float_parameter(package,parameter, def) 
    implicit none
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        REAL(C_FLOAT), intent(in) :: def 
        vnv_get_float_parameter = vnv_get_float_parameter_c(convert(package),convert(parameter), def)
        return;
    end function


    REAL(c_double) function vnv_get_double_parameter(package,parameter, def) 
    implicit none
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        REAL(c_double), intent(in) :: def 
        vnv_get_double_parameter = vnv_get_double_parameter_c(convert(package),convert(parameter), def)
        return;
    end function



    function vnv_get_string_parameter(package,parameter, def) result(f_string)
        use, intrinsic :: iso_c_binding, only: c_ptr,c_f_pointer,c_char,c_null_char
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        character(len=*), intent(in) :: def
        character(len=:), allocatable :: f_string
        character(kind=c_char), dimension(:), pointer :: char_array_pointer => null()
        character(len=255) :: aux_string
        integer :: i,length
        type(c_ptr) :: c_string_pointer
            
        c_string_pointer = vnv_get_string_parameter_c(convert(package),convert(parameter))
            
        call c_f_pointer(c_string_pointer,char_array_pointer,[255])
            
        if (.not.associated(char_array_pointer)) then
           allocate(character(len=len(def))::f_string);
           f_string=def;
           return;
        end if

        aux_string=" "
        do i=1,255
          if (char_array_pointer(i)==c_null_char) then
             length=i-1; exit
          end if
          aux_string(i:i)=char_array_pointer(i)
        end do
        allocate(character(len=length)::f_string)
        f_string=aux_string(1:length)
    end function vnv_get_string_parameter

    INTEGER(C_INT) function vnv_get_parameter_type(package,parameter) 
        implicit none
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        vnv_get_parameter_type = vnv_get_parameter_type_c(convert(package),convert(parameter))
    end function

    INTEGER(C_INT) function vnv_get_parameter_size(package,parameter) 
        implicit none
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: parameter
        vnv_get_parameter_size = vnv_get_parameter_size_c(convert(package),convert(parameter))
    end function

    subroutine vnv_debug(world, package, message) 
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: message
        call vnv_log_c(int(world,kind=c_int), convert("debug"), convert(package),convert(message))
    end subroutine

    subroutine vnv_error(world, package, message) 
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: message
        call vnv_log_c(int(world,kind=c_int), convert("error"), convert(package),convert(message))
    end subroutine

    subroutine vnv_info(world, package, message) 
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: message
        call vnv_log_c(int(world,kind=c_int), convert("info"), convert(package),convert(message))
    end subroutine

    subroutine vnv_warning(world, package, message) 
        implicit none
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: message
        call vnv_log_c(int(world,kind=c_int), convert("warning"), convert(package),convert(message))
    end subroutine

    subroutine vnv_input_file(world,package,name, filename,reader,action)
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: name
        character(len=*), intent(in) :: filename
        character(len=*), intent(in) :: reader
        INTEGER, intent(in) :: action

        call vnv_file_c(int(world,kind=c_int),INT(1,kind=C_INT),convert(package),convert(name), &
            convert(filename),convert(reader),int(action,kind=c_int))        
    end subroutine

    subroutine vnv_output_file(world,package,name,filename,reader,action)
        INTEGER, intent(in) :: world
        character(len=*), intent(in) :: package
        character(len=*), intent(in) :: name
        character(len=*), intent(in) :: filename
        character(len=*), intent(in) :: reader
        INTEGER, intent(in) :: action
        call vnv_file_c(int(world,kind=c_int),INT(0,kind=C_INT),convert(package),convert(name), &
            convert(filename),convert(reader),int(action,kind=c_int))        
    end subroutine
    
end module